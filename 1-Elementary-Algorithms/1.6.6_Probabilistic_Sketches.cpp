/*

A sketch answers questions about a stream using far less memory than the stream itself, in exchange
for a bounded, one-sided error. This section implements a Bloom filter, which tests set membership,
and a count-min sketch, which estimates how often each value occurred. Both spread every value over
several independent positions of a fixed-size table, and both derive those positions from one pair
of base hashes: the Kirsch-Mitzenmacher technique uses $h_i(x) = h_1(x) + i \cdot h_2(x)$ instead of
computing an unrelated hash for every position, which matches the false-positive behavior of truly
independent hashes while paying for only two. Both base hashes come from the SplitMix64 mixer of
section 3.2.1, and the second is forced odd so that repeated steps never stand still.

A Bloom filter represents a set as a bit array. Inserting a value sets the $k$ bits it hashes to,
and a membership test reports that a value is present only if all $k$ of its bits are set. Bits are
never cleared, so a value that was inserted always tests positive: the error is one-sided, and only
a false positive is possible, produced when unrelated insertions happen to have set all $k$ bits.
For $n$ values in $m$ bits, the false-positive probability is minimized by $k = (m/n) \ln 2$, which
yields $m = -n \ln p / (\ln 2)^2$ bits for a target rate $p$.

- `BloomFilter<T>(capacity, false_positive_rate)` constructs an empty filter sized for `capacity`
  insertions at the given target rate, where `capacity` is positive and the rate lies in $(0, 1)$.
  `BloomFilter<T, Hash>(capacity, false_positive_rate, hash)` instead stores the supplied hasher.
- `insert(x)` adds `x` to the filter.
- `contains(x)` returns whether `x` may be in the filter. It never returns `false` for a value that
  was inserted, but may return `true` for one that was not.
- `count()` returns the number of insertions performed, counting repeated values separately.
- `bit_count()` and `hash_count()` return the derived table size $m$ and number of positions $k$.
- `false_positive_rate()` returns the current estimated rate $(1 - e^{-kn/m})^k$, which rises as the
  filter fills past its planned capacity.

Deletion is not supported, since clearing a bit could hide an unrelated value that also set it. Use
a counting Bloom filter, which replaces each bit with a small counter, when values must leave.

Time Complexity:
- O(1) per call to `count()`, `bit_count()`, `hash_count()`, and `false_positive_rate()`.
- O(k) per call to `insert()` and `contains()`, where $k$ is `hash_count()`, plus the cost of one
  call to the hasher.
- O(m) per call to the constructor, where $m$ is `bit_count()`.

Space Complexity:
- O(m) for storage of the filter, where $m$ is `bit_count()`.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

uint64_t splitmix64(uint64_t x) {
  x += 0x9e3779b97f4a7c15ULL;
  x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
  x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
  return x ^ (x >> 31);
}

template<typename T, typename Hash>
std::pair<uint64_t, uint64_t> hash_pair(const Hash &hash, const T &x) {
  uint64_t h1 = splitmix64(static_cast<uint64_t>(hash(x)));
  return {h1, splitmix64(h1 + 1) | 1};
}

template<typename T, typename Hash = std::hash<T>>
class BloomFilter {
  std::vector<uint64_t> words;
  uint64_t nbits;
  int hashes, inserted;
  Hash hash;

 public:
  BloomFilter(int capacity, double false_positive_rate, Hash hash = Hash{})
      : nbits(0), hashes(0), inserted(0), hash(hash) {
    assert(capacity > 0 && false_positive_rate > 0 && false_positive_rate < 1);
    double ln2 = std::log(2.0);
    nbits =
        static_cast<uint64_t>(std::ceil(-capacity * std::log(false_positive_rate) / (ln2 * ln2)));
    hashes = std::max(1, static_cast<int>(std::lround(nbits * ln2 / capacity)));
    words.resize((nbits + 63) / 64);
  }

  void insert(const T &x) {
    auto [h1, h2] = hash_pair(hash, x);
    for (int i = 0; i < hashes; i++) {
      uint64_t bit = (h1 + static_cast<uint64_t>(i) * h2) % nbits;
      words[bit >> 6] |= 1ULL << (bit & 63);
    }
    inserted++;
  }

  bool contains(const T &x) const {
    auto [h1, h2] = hash_pair(hash, x);
    for (int i = 0; i < hashes; i++) {
      uint64_t bit = (h1 + static_cast<uint64_t>(i) * h2) % nbits;
      if (((words[bit >> 6] >> (bit & 63)) & 1) == 0) {
        return false;
      }
    }
    return true;
  }

  int count() const { return inserted; }
  uint64_t bit_count() const { return nbits; }
  int hash_count() const { return hashes; }

  double false_positive_rate() const {
    double filled = 1 - std::exp(-1.0 * hashes * inserted / nbits);
    return std::pow(filled, hashes);
  }
};

/*

A count-min sketch estimates the total weight added under each value. It stores an $R$ by $C$ table
of counters and, for each row, adds the weight to the one counter that the value hashes to in that
row. Every counter that a value touches therefore holds that value's true total plus the totals of
whatever else collided there, so with nonnegative weights each row is an overestimate and the
minimum over the rows is the sharpest one available. Choosing $C = \lceil e/\epsilon \rceil$ and
$R = \lceil \ln(1/\delta) \rceil$ bounds the overestimate by $\epsilon N$ with probability at least
$1 - \delta$, where $N$ is the total weight added.

- `CountMinSketch<T>(eps, delta)` constructs an empty sketch whose estimates exceed the true count
  by at most `eps` times the total weight, with probability at least $1 - `delta`$. Both parameters
  must lie in $(0, 1)$. `CountMinSketch<T, Hash>(eps, delta, hash)` instead stores the supplied
  hasher.
- `add(x, c = 1)` adds weight `c` to the count of `x`.
- `count(x)` returns the estimated total weight of `x`, which is never an underestimate.
- `total()` returns the total weight added over all values.
- `num_rows()` and `num_cols()` return the derived table dimensions.

The error bound is relative to the total weight of the whole stream, so a sketch estimates heavy
values well and light ones poorly. That makes it a natural companion to the deterministic
heavy-hitter candidates of section 1.6.3: run both, then use the sketch to rank the candidates.
Negative weights break the guarantee, since a row is then no longer an overestimate; deletions
require the count-mean-min variant or a conservative sketch.

Time Complexity:
- O(R) per call to `add()` and `count()`, plus the cost of one call to the hasher.
- O(R*C) per call to the constructor.
- O(1) per call to `total()`, `num_rows()`, and `num_cols()`.

Space Complexity:
- O(w*d) for storage of the sketch.
- O(1) auxiliary for all operations.

*/

template<typename T, typename Hash = std::hash<T>>
class CountMinSketch {
  std::vector<std::vector<int64_t>> table;
  int rows, cols;
  int64_t weight;
  Hash hash;

 public:
  CountMinSketch(double eps, double delta, Hash hash = Hash{}) : weight(0), hash(hash) {
    assert(eps > 0 && eps < 1 && delta > 0 && delta < 1);
    rows = std::max(1, static_cast<int>(std::ceil(std::log(1 / delta))));
    cols = static_cast<int>(std::ceil(std::exp(1.0) / eps));
    table.assign(rows, std::vector<int64_t>(cols));
  }

  void add(const T &x, int64_t c = 1) {
    auto [h1, h2] = hash_pair(hash, x);
    for (int r = 0; r < rows; r++) {
      table[r][(h1 + static_cast<uint64_t>(r) * h2) % cols] += c;
    }
    weight += c;  // Overflow warning.
  }

  int64_t count(const T &x) const {
    auto [h1, h2] = hash_pair(hash, x);
    int64_t res = INT64_MAX;
    for (int r = 0; r < rows; r++) {
      res = std::min(res, table[r][(h1 + static_cast<uint64_t>(r) * h2) % cols]);
    }
    return res;
  }

  int64_t total() const { return weight; }
  int num_rows() const { return rows; }
  int num_cols() const { return cols; }
};

/*** Example Usage ***/

#include <string>
using namespace std;

int main() {
  BloomFilter<string> filter(1000, 0.01);
  assert(filter.bit_count() == 9586 && filter.hash_count() == 7);
  for (int i = 0; i < 1000; i++) {
    filter.insert("key" + to_string(i));
  }
  assert(filter.count() == 1000);
  for (int i = 0; i < 1000; i++) {
    assert(filter.contains("key" + to_string(i)));  // Inserted values never test negative.
  }
  int false_positives = 0;
  for (int i = 1000; i < 2000; i++) {
    false_positives += filter.contains("key" + to_string(i));
  }
  assert(false_positives < 40);  // Absent values may test positive, about 10 of these 1000.
  assert(filter.false_positive_rate() < 0.02);

  CountMinSketch<string> sketch(0.01, 0.01);
  sketch.add("apple", 3);
  sketch.add("banana");
  assert(sketch.total() == 4);
  assert(sketch.count("apple") == 3 && sketch.count("banana") == 1);
  // A value that was never added reads 0 unless it collides in every row, and an estimate is never
  // lower than the truth.
  assert(sketch.count("cherry") <= sketch.total());
  return 0;
}
