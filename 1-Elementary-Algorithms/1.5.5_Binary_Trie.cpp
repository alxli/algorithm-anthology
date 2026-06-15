/*

Maintain a multiset of non-negative $b$-bit integers as a binary trie, storing each value as a
root-to-leaf path of its bits from the most significant (bit $b - 1$) down to the least significant.
Every node keeps a count of the stored values passing through it, which supports deletion and
counting queries, and lets the trie answer XOR-extremal queries by a greedy walk down the bits.

The classic application is the maximum-XOR query: to maximize `x XOR y` over all stored `y`, walk
from the most significant bit. At each step, descend toward the child whose bit differs from that of
`x` whenever such a branch exists, since setting a higher bit of the result always dominates any
combination of lower bits. The minimum-XOR query is the mirror image, preferring the matching bit.

This is a multiset (a value may be inserted more than once), and is distinct from the XOR basis: the
basis spans subset XORs of a fixed set, whereas this answers XOR queries against the stored elements
themselves with support for insertion and deletion.

Values must lie in `[0, 2^BITS)`; the template parameters select the unsigned word type `U` and the
bit width `BITS`, which must be less than the width of `U` (for example, 30 with `uint32_t` or
62 with `uint64_t`). Nodes are kept in a pool indexed by integer, where index 0 is the root and also
serves as the null child.

- `BinaryTrie()` constructs an empty multiset.
- `size()` returns the number of stored values, and `empty()` returns whether there are none.
- `insert(x)` adds one occurrence of `x`.
- `erase(x)` removes one occurrence of `x`, returning whether one was present.
- `count(x)` returns the number of occurrences of `x`.
- `max_xor(x)` and `min_xor(x)` return the maximum and minimum of `x XOR y` over all stored `y`. The
  trie must be non-empty.
- `count_xor_less(x, k)` returns the number of stored `y` (with multiplicity) such that `x XOR y` is
  strictly less than `k`.

Time Complexity:
- O(b) per call to `insert()`, `erase()`, `count()`, `max_xor()`, `min_xor()`, and
  `count_xor_less()`, where $b$ is the bit width `BITS`.
- O(1) per call to `size()` and `empty()`.

Space Complexity:
- O(`BITS` * n) for storage, where $n$ is the number of distinct values inserted so far.

*/

#include <array>
#include <cstdint>
#include <vector>

template<class U = uint32_t, int BITS = 30>
class BinaryTrie {
  std::vector<std::array<int, 2>> child;  // child[node][bit], where 0 means no child.
  std::vector<int> cnt;                   // Number of stored values passing through each node.

  int new_node() {
    child.push_back({0, 0});
    cnt.push_back(0);
    return static_cast<int>(cnt.size()) - 1;
  }

 public:
  BinaryTrie() { new_node(); }  // Node 0 is the root (and doubles as the null child).

  int size() const { return cnt[0]; }
  bool empty() const { return cnt[0] == 0; }

  void insert(U x) {
    int node = 0;
    cnt[node]++;
    for (int b = BITS - 1; b >= 0; b--) {
      int bit = (x >> b) & 1;
      if (child[node][bit] == 0) {
        int next = new_node();
        child[node][bit] = next;
      }
      node = child[node][bit];
      cnt[node]++;
    }
  }

  int count(U x) const {
    int node = 0;
    for (int b = BITS - 1; b >= 0; b--) {
      int next = child[node][(x >> b) & 1];
      if (next == 0) {
        return 0;
      }
      node = next;
    }
    return cnt[node];
  }

  bool erase(U x) {
    if (count(x) == 0) {
      return false;
    }
    int node = 0;
    cnt[node]--;
    for (int b = BITS - 1; b >= 0; b--) {
      node = child[node][(x >> b) & 1];
      cnt[node]--;
    }
    return true;
  }

  U max_xor(U x) const {
    int node = 0;
    U res = 0;
    for (int b = BITS - 1; b >= 0; b--) {
      int bit = (x >> b) & 1;
      int opp = child[node][bit ^ 1];
      if (opp != 0 && cnt[opp] > 0) {
        res |= U(1) << b;
        node = opp;
      } else {
        node = child[node][bit];
      }
    }
    return res;
  }

  U min_xor(U x) const {
    int node = 0;
    U res = 0;
    for (int b = BITS - 1; b >= 0; b--) {
      int bit = (x >> b) & 1;
      int same = child[node][bit];
      if (same != 0 && cnt[same] > 0) {
        node = same;
      } else {
        res |= U(1) << b;
        node = child[node][bit ^ 1];
      }
    }
    return res;
  }

  int count_xor_less(U x, U k) const {
    if ((k >> BITS) != 0) {
      return cnt[0];  // Every XOR result is below 2^BITS, which is at most k.
    }
    int node = 0, res = 0;
    for (int b = BITS - 1; b >= 0; b--) {
      int xb = (x >> b) & 1, kb = (k >> b) & 1;
      if (kb == 1) {
        // Stored values matching x at bit b give XOR bit 0 here, hence are already below k.
        int below = child[node][xb];
        if (below != 0) {
          res += cnt[below];
        }
        node = child[node][xb ^ 1];  // Stay tied with k by taking XOR bit 1.
      } else {
        node = child[node][xb];  // XOR bit must be 0 to remain below k.
      }
      if (node == 0) {
        break;
      }
    }
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  BinaryTrie<> trie;
  for (int x : {3, 10, 5, 25, 2}) {
    trie.insert(x);
  }
  assert(trie.size() == 5);

  // x XOR y over the stored y: {26, 19, 28, 0, 27} for x = 25.
  assert(trie.max_xor(25) == 28);                           // 25 XOR 5.
  assert(trie.min_xor(25) == 0);                            // 25 XOR 25.
  assert(trie.count_xor_less(25, 20) == 2);                 // XOR values 0 and 19 are below 20.
  assert(trie.count_xor_less(25, uint32_t(1) << 30) == 5);  // Bound at/above 2^BITS counts all.

  assert(trie.count(10) == 1);
  trie.insert(10);
  assert(trie.count(10) == 2);  // Multiset: two copies of 10.
  assert(trie.erase(10) && trie.count(10) == 1);
  assert(trie.erase(25) && trie.count(25) == 0);
  assert(!trie.erase(25));        // Already gone.
  assert(trie.max_xor(0) == 10);  // Largest remaining value is 10.
  return 0;
}
