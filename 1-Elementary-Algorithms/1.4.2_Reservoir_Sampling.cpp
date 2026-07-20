/*

Selects uniformly random samples from a stream whose length may be unknown in advance. This is
useful when values arrive online or when storing the full dataset is unnecessary.

Reservoir sampling keeps only the chosen sample or samples in memory while processing each element
once. The first arrivals fill the reservoir, and the $i$-th element then replaces a uniformly chosen
occupant with probability $1/i$ (or $k/i$ when keeping $k$ samples), which by induction leaves every
element seen so far equally likely to be in the sample.

Each class maintains its reservoir incrementally; call `add(x)` once per stream element in any
order, then call `get()` to retrieve the result.

- `ReservoirSampleOne<T>()` constructs a single-element sampler.
- `ReservoirSampleK<T>(k)` constructs a `k`-element sampler.
- `add(x)` incorporates one more stream element.
- `get()` returns the current sample. For `ReservoirSampleOne`, `get()` requires at least one
  `add()` call; for `ReservoirSampleK`, it returns the full reservoir, which may contain fewer than
  `k` elements if the stream was shorter.
- `count()` returns the number of stream elements seen so far.

Time Complexity:
- O(1) per call to `ReservoirSampleOne::add()` and O(1) amortized per call to
  `ReservoirSampleK::add()`.
- O(n) to process a range of $n$ elements.

Space Complexity:
- O(1) storage for `ReservoirSampleOne`.
- O(k) storage for `ReservoirSampleK`.

*/

#include <cassert>
#include <optional>
#include <random>
#include <vector>

int rand_int(int lo, int hi) {
  static std::mt19937 rng(std::random_device{}());
  return std::uniform_int_distribution<int>(lo, hi)(rng);
}

template<typename T>
class ReservoirSampleOne {
  std::optional<T> sample;
  int seen = 0;

 public:
  ReservoirSampleOne() = default;

  void add(const T &x) {
    seen++;
    if (rand_int(1, seen) == 1) {
      sample = x;
    }
  }

  const T &get() const {
    assert(sample.has_value());
    return *sample;
  }

  int count() const { return seen; }
};

template<typename T>
class ReservoirSampleK {
  int k, seen = 0;
  std::vector<T> reservoir;

 public:
  explicit ReservoirSampleK(int k) : k(k) { assert(k >= 0); }

  void add(const T &x) {
    ++seen;
    if (k == 0) {
      return;
    }
    if (static_cast<int>(reservoir.size()) < k) {
      reservoir.push_back(x);
    } else {
      int j = rand_int(0, seen - 1);
      if (j < k) {
        reservoir[j] = x;
      }
    }
  }

  const std::vector<T> &get() const { return reservoir; }
  int count() const { return seen; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{10, 20, 30, 40, 50};

  // Streaming interface: feed elements one at a time.
  ReservoirSampleOne<int> s1;
  for (int x : a) {
    s1.add(x);
  }
  assert(s1.count() == 5);
  int one = s1.get();
  assert(one == 10 || one == 20 || one == 30 || one == 40 || one == 50);

  ReservoirSampleK<int> sk(3);
  for (int x : a) {
    sk.add(x);
  }
  assert(sk.count() == 5);
  assert(sk.get().size() == 3);

  ReservoirSampleK<int> large(10);
  for (int x : a) {
    large.add(x);
  }
  assert(large.count() == 5);
  assert(large.get().size() == 5);
  return 0;
}
