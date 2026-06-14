/*

Maintain a numerical array while supporting point increments and range-sum queries. A Fenwick tree
(also called a binary indexed tree) stores partial prefix sums: each internal entry `tree[i]` holds
the sum of a block of indices whose length is the lowest set bit of `i`. A point update touches
O(log n) covering blocks, and a prefix-sum query combines O(log n) disjoint blocks.

When the stored values are nonnegative frequencies, `kth(k)` answers order-statistic queries by
binary lifting: it walks the implicit power-of-two block structure to find the first index whose
prefix count reaches `k`. This lets the same structure double as a dynamic multiset over a bounded
integer domain, useful for coordinate-compressed kth-element lookups and online rank queries.

- `FenwickTree(n)` constructs an array with 0-based indices from 0 to `n - 1`, inclusive, with all
  values initialized to 0.
- `size()` returns the size of the array.
- `add(i, x)` adds `x` to the value at index `i`.
- `set(i, x)` assigns the value at index `i` to `x`.
- `at(i)` returns the value at index `i`.
- `sum(hi)` returns the sum of all values at indices from 0 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.
- `kth(k)` returns the smallest index `i` such that `sum(i)` $\geq$ `k`, assuming all values are
  nonnegative. The rank `k` must satisfy `1 <= k <= sum(size() - 1)`.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `add()`, both `sum()` functions, `set()`, `at()`, and `kth()`.

Space Complexity:
- O(n) for storage of the Fenwick tree.
- O(1) auxiliary for all operations.

*/

#include <vector>

template<class T>
class FenwickTree {
  int len;
  std::vector<T> tree;

 public:
  explicit FenwickTree(int n) : len(n), tree(n + 1) {}

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

  int kth(T k) const {
    int pos = 0, pw = 1;
    while (pw * 2 <= len) {
      pw *= 2;
    }
    for (; pw > 0; pw >>= 1) {
      if (pos + pw <= len && tree[pos + pw] < k) {
        pos += pw;
        k -= tree[pos];
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
  FenwickTree<int> t(5);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    t.set(i, a[i]);
  }
  t.add(0, -5);  // a is now {5, 1, 2, 3, 4}.
  assert(t.at(0) == 5);
  assert(t.sum(1, 3) == 6);  // 1 + 2 + 3
  assert(t.sum(4) == 15);    // 5 + 1 + 2 + 3 + 4

  // With nonnegative values, the tree doubles as a frequency table for order-statistic queries.
  FenwickTree<int> freq(8);
  freq.add(1, 1);  // One element of value 1.
  freq.add(3, 3);  // Three elements of value 3.
  freq.add(6, 1);  // One element of value 6.
  assert(freq.kth(1) == 1);
  assert(freq.kth(2) == 3);
  assert(freq.kth(4) == 3);
  assert(freq.kth(5) == 6);
  return 0;
}
