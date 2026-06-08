/*

Given a static array, precompute a table that answers range queries for any associative operation in
constant time. Unlike the ordinary sparse table for range minimum queries, the disjoint sparse table
does not require the operation to be idempotent, so it also handles sums, products, and matrix
products where overlapping the two query halves would double-count.

The array is divided into blocks whose size doubles at each of the $O(\log n)$ levels. At the level
whose block size is $2^{k+1}$, every block is split at its center, and the table stores, for each
position, the fold of the contiguous run from that position up to (or down to) the center. To answer
a query `[lo, hi]`, find the level at which `lo` and `hi` first fall on opposite sides of a center:
this is the position of the highest set bit of `lo xor hi`. At that level, `lo` lies in the block's
left half and `hi` in its right half, so the answer is the stored suffix fold ending at the center
combined with the stored prefix fold beginning at the center.

The query operation is defined by an associative function `combine(a, b)`. The default code below
returns the "min" of the range; for "sum", `combine(a, b)` should return `a + b`.

- `DisjointSparseTable(lo, hi)` builds the table from the range `[lo, hi)` of a random-access
  iterator.
- `size()` returns the size of the array.
- `query(lo, hi)` returns `combine()` applied to all indices from `lo` to `hi`, inclusive.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()` and `query()`.

Space Complexity:
- O(n log n) for storage of the table.
- O(1) auxiliary for `query()`.

*/

#include <algorithm>
#include <vector>

template<class T>
class DisjointSparseTable {
  static T combine(const T &a, const T &b) { return std::min(a, b); }

  int len;
  std::vector<T> a;
  std::vector<std::vector<T>> fold;

 public:
  template<class It>
  DisjointSparseTable(It lo, It hi) : a(lo, hi) {
    len = a.size();
    int levels = 1;
    while ((1 << levels) < len) {
      levels++;
    }
    fold.assign(levels, std::vector<T>(len));
    for (int level = 0; level < levels; level++) {
      int range = 1 << (level + 1), half = 1 << level;
      for (int center = half; center < len + half; center += range) {
        int left = center - half, right = std::min(len, center + half);
        int top = std::min(center, len) - 1;  // Suffix folds over [i, center).
        fold[level][top] = a[top];
        for (int i = top - 1; i >= left; i--) {
          fold[level][i] = combine(a[i], fold[level][i + 1]);
        }
        if (center < len) {  // Prefix folds over [center, i].
          fold[level][center] = a[center];
          for (int i = center + 1; i < right; i++) {
            fold[level][i] = combine(fold[level][i - 1], a[i]);
          }
        }
      }
    }
  }

  int size() const { return len; }

  T query(int lo, int hi) const {
    if (lo == hi) {
      return a[lo];
    }
    int level = 31 - __builtin_clz((unsigned)(lo ^ hi));
    return combine(fold[level][lo], fold[level][hi]);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10, -5, 3};
  DisjointSparseTable<int> t(a.begin(), a.end());
  assert(t.size() == 7);
  assert(t.query(0, 6) == -5);
  assert(t.query(1, 3) == -2);
  assert(t.query(4, 4) == 10);
  assert(t.query(2, 4) == 1);
  return 0;
}
