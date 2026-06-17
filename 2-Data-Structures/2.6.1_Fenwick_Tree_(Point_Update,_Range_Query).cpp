/*

Maintain a numerical array while supporting point increments and range-sum queries. A Fenwick tree
(also called a binary indexed tree) stores partial prefix sums: each internal entry `tree[i]` holds
the sum of a block of indices whose length is the lowest set bit of `i`. A point update touches
O(log n) covering blocks, and a prefix-sum query combines O(log n) disjoint blocks.

When prefix sums are nondecreasing, `max_prefix(c)` answers prefix-boundary queries by binary
lifting: it walks the implicit power-of-two block structure to find the longest prefix with sum at
most `c`. This lets the same structure double as a dynamic multiset over a bounded integer domain,
useful for coordinate-compressed $k$-th element lookups and online rank queries.

- `Fenwick<T>(n)` constructs an array with 0-based indices [0, `n`), with all values initialized to
  0.
- `size()` returns the size of the array.
- `add(i, x)` adds `x` to the value at index `i`.
- `set(i, x)` assigns the value at index `i` to `x`.
- `at(i)` returns the value at index `i`.
- `sum(hi)` returns the sum of all values at indices from 0 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.
- `max_prefix(c)` returns the largest boundary `hi` such that `sum(0, hi - 1) <= c`, assuming
  prefix sums are nondecreasing. It may return any value from 0 to `size()`.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `add()`, both `sum()` functions, `set()`, `at()`, and `max_prefix()`.

Space Complexity:
- O(n) for storage of the Fenwick tree.
- O(1) auxiliary for all operations.

*/

#include <vector>

template<class T>
class Fenwick {
  int len;
  std::vector<T> tree;

 public:
  explicit Fenwick(int n) : len(n), tree(n + 1) {}

  int size() const { return len; }

  void add(int i, const T &x) {
    for (++i; i <= len; i += i & -i) {
      tree[i] += x;
    }
  }

  T sum(int hi) const {
    T res = 0;
    for (++hi; hi > 0; hi -= hi & -hi) {
      res += tree[hi];
    }
    return res;
  }

  void set(int i, const T &x) { add(i, x - at(i)); }
  T at(int i) const { return sum(i, i); }
  T sum(int lo, int hi) const { return sum(hi) - sum(lo - 1); }

  int max_prefix(T c) const {
    int pos = 0, pw = 1;
    while (pw * 2 <= len) {
      pw *= 2;
    }
    for (; pw > 0; pw >>= 1) {
      if (pos + pw <= len && tree[pos + pw] <= c) {
        c -= tree[pos + pw];
        pos += pw;
      }
    }
    return pos;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{10, 1, 2, 3, 4};
  Fenwick<int> t(5);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    t.set(i, a[i]);
  }
  t.add(0, -5);  // a is now {5, 1, 2, 3, 4}.
  assert(t.at(0) == 5);
  assert(t.sum(1, 3) == 6);  // 1 + 2 + 3
  assert(t.sum(4) == 15);    // 5 + 1 + 2 + 3 + 4

  // With nonnegative values, the tree doubles as a frequency table for order-statistic queries.
  Fenwick<int> freq(8);
  freq.add(1, 1);                   // One element of value 1.
  freq.add(3, 3);                   // Three elements of value 3.
  freq.add(6, 1);                   // One element of value 6.
  assert(freq.max_prefix(0) == 1);  // The longest prefix with count <= 0 is [0, 1).
  assert(freq.max_prefix(3) == 3);  // Prefix [0, 3) has count 1; [0, 4) has count 4.
  assert(freq.max_prefix(4) == 6);  // Prefix [0, 6) has count 4; [0, 7) has count 5.
  return 0;
}
