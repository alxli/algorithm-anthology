/*

Computes randomized XOR fingerprints for keys and sets of keys using Zobrist hashing. Each distinct
key is assigned a pseudorandom 64-bit token, and a set is represented by the XOR of its members'
tokens. This makes insertion and deletion the same operation: XOR the key's token once.

This technique is useful for probabilistic equality checks on sets, distinct-prefix queries,
board-game states, and other unordered collections. It is not cryptographic, and collisions are
possible with probability roughly $q^2 / 2^{64}$ over $q$ compared fingerprints. For multisets,
plain XOR only records element parity; use a summed hash or pair it with counts if multiplicity
matters.

- `ZobristHash<T>(seed = 0)` constructs a token generator with initial value `seed`.
- `token(x)` returns the stable token assigned to key `x`, creating it if needed.
- `toggle(h, x)` returns the hash obtained by inserting `x` into set hash `h` if absent, or removing
  `x` from `h` if present.
- `distinct_prefix_hashes(lo, hi)` returns `n + 1` hashes. Element `i` hashes the first `i` values
  in $[`lo`, `hi`)$ with each distinct key contributing only on its first occurrence.

Time Complexity:
- O(1) expected per call to `token()` and `toggle()`.
- O(n) expected per call to `distinct_prefix_hashes(lo, hi)`, where $n$ is the distance between `lo`
  and `hi`.

Space Complexity:
- O(n) for stored key tokens, where $n$ is the number of distinct keys seen so far.
- O(n) auxiliary for `distinct_prefix_hashes()`.

*/

#include <cstdint>
#include <unordered_map>
#include <unordered_set>
#include <vector>

template<typename T>
class ZobristHash {
  std::unordered_map<T, uint64_t> tokens;
  uint64_t state;

  static uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
  }

 public:
  explicit ZobristHash(uint64_t seed = 0) : state(seed) {}

  uint64_t token(const T &x) {
    if (auto it = tokens.find(x); it != tokens.end()) {
      return it->second;
    }
    state = splitmix64(state);
    tokens[x] = state;
    return state;
  }

  uint64_t toggle(uint64_t h, const T &x) { return h ^ token(x); }

  template<typename It>
  std::vector<uint64_t> distinct_prefix_hashes(It lo, It hi) {
    std::unordered_set<T> seen;
    std::vector<uint64_t> res(1);
    uint64_t h = 0;
    for (It it = lo; it != hi; ++it) {
      if (seen.insert(*it).second) {
        h ^= token(*it);
      }
      res.push_back(h);
    }
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{1, 2, 1, 3};
  vector<int> b{2, 1, 3};
  ZobristHash<int> zh(1234567);  // Fixed seed for reproducibility.
  vector<uint64_t> pa = zh.distinct_prefix_hashes(a.begin(), a.end());
  vector<uint64_t> pb = zh.distinct_prefix_hashes(b.begin(), b.end());

  assert(pa[2] == pb[2]);  // Distinct sets {1, 2} and {2, 1}.
  assert(pa[4] == pb[3]);  // Distinct sets {1, 2, 3} and {2, 1, 3}.

  uint64_t h = 0;
  h = zh.toggle(h, 5);
  assert(h != 0);
  h = zh.toggle(h, 5);
  assert(h == 0);
  return 0;
}
