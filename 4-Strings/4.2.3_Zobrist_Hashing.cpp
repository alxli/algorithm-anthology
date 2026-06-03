/*

Computes randomized XOR fingerprints for keys and sets of keys using Zobrist hashing. Each distinct
key is assigned a pseudorandom 64-bit token, and a set is represented by the XOR of its members'
tokens. This makes insertion and deletion the same operation: XOR the key's token once.

This technique is useful for probabilistic equality checks on sets, distinct-prefix queries,
board-game states, and other unordered collections. It is not cryptographic, and collisions are
possible with probability roughly $q^2 / 2^{64}$ over $q$ compared fingerprints. For multisets,
plain XOR only records element parity; use a summed hash or pair it with counts if multiplicity
matters.

- `zobrist_hash(seed)` constructs a token generator with initial value `seed`.
- `get(x)` returns the stable token assigned to key `x`, creating it if needed.
- `toggle(h, x)` returns the hash obtained by inserting `x` into set hash `h` if absent, or removing
  `x` from `h` if present.
- `distinct_prefix_hashes(lo, hi)` returns prefix hashes where each distinct key in the range
  `[lo, hi)` contributes only on its first occurrence.

Time Complexity:
- O(log n) per call to `get(x)` and `toggle(h, x)`, where $n$ is the number of distinct keys seen so
  far.
- O(n log n) per call to `distinct_prefix_hashes(lo, hi)`, where $n$ is the distance between `lo`
  and `hi`.

Space Complexity:
- O(n) for stored key tokens, where $n$ is the number of distinct keys seen so far.
- O(n) auxiliary for `distinct_prefix_hashes(lo, hi)`.

*/

#include <map>
#include <set>
#include <vector>

typedef unsigned long long uint64;

template<class T>
class zobrist_hash {
  std::map<T, uint64> token;
  uint64 state;

  static uint64 splitmix64(uint64 x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
  }

 public:
  explicit zobrist_hash(uint64 seed = 0) : state(seed) {}

  uint64 get(const T &x) {
    typename std::map<T, uint64>::iterator it = token.find(x);
    if (it != token.end()) {
      return it->second;
    }
    state = splitmix64(state);
    token[x] = state;
    return state;
  }

  uint64 toggle(uint64 h, const T &x) { return h ^ get(x); }

  template<class It>
  std::vector<uint64> distinct_prefix_hashes(It lo, It hi) {
    std::set<T> seen;
    std::vector<uint64> res(1, 0);
    uint64 h = 0;
    for (It it = lo; it != hi; ++it) {
      if (seen.insert(*it).second) {
        h ^= get(*it);
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
  int a[] = {1, 2, 1, 3};
  int b[] = {2, 1, 3};
  zobrist_hash<int> zh(123);
  vector<uint64> pa = zh.distinct_prefix_hashes(a, a + 4);
  vector<uint64> pb = zh.distinct_prefix_hashes(b, b + 3);

  assert(pa[2] == pb[2]);  // Distinct sets {1, 2} and {2, 1}.
  assert(pa[4] == pb[3]);  // Distinct sets {1, 2, 3} and {2, 1, 3}.

  uint64 h = 0;
  h = zh.toggle(h, 5);
  assert(h != 0);
  h = zh.toggle(h, 5);
  assert(h == 0);
  return 0;
}
