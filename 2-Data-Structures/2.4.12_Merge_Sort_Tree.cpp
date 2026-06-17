/*

A merge sort tree is a static segment tree in which every node stores the sorted multiset of the
values in its range. The nodes are exactly the ranges visited by a merge sort, and each node's
sorted list is the merge of its two children's lists. This supports queries that count, within a
range of indices, how many values fall below a threshold or inside a value interval: decompose the
index range into $O(\log n)$ canonical nodes and binary search each node's sorted list.

The structure is built once and not modified afterward. All index ranges are inclusive [`lo`, `hi`]
with 0-based indices, and values may be of any comparable type.

- `MergeSortTree<T>(a)` builds the tree over the array `a`.
- `count_leq(lo, hi, x)` returns the number of indices `i` in [`lo`, `hi`] with `a[i]` $\leq$ `x`.
- `count_in(lo, hi, x, y)` returns the number of indices `i` in [`lo`, `hi`] with
  `x` $\leq$ `a[i]` $\leq$ `y`.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the size of the array.
- O(log^2 n) per call to `count_leq()` and `count_in()`.

Space Complexity:
- O(n log n) for storage of the tree.
- O(log n) auxiliary stack space per query.

*/

#include <algorithm>
#include <iterator>
#include <vector>

template<class T>
class MergeSortTree {
  int len;
  std::vector<std::vector<T>> tree;

  void build(int node, int lo, int hi, const std::vector<T> &a) {
    if (lo == hi) {
      tree[node] = {a[lo]};
      return;
    }
    int mid = lo + (hi - lo) / 2;
    build(2 * node, lo, mid, a);
    build(2 * node + 1, mid + 1, hi, a);
    std::merge(
        tree[2 * node].begin(), tree[2 * node].end(), tree[2 * node + 1].begin(),
        tree[2 * node + 1].end(), std::back_inserter(tree[node])
    );
  }

  template<class Fn>
  int query(int node, int lo, int hi, int tgt_lo, int tgt_hi, Fn count_node) const {
    if (tgt_hi < lo || hi < tgt_lo) {
      return 0;
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      return count_node(tree[node]);
    }
    int mid = lo + (hi - lo) / 2;
    return query(2 * node, lo, mid, tgt_lo, tgt_hi, count_node) +
           query(2 * node + 1, mid + 1, hi, tgt_lo, tgt_hi, count_node);
  }

 public:
  MergeSortTree(const std::vector<T> &a) : len(a.size()), tree(4 * a.size()) {
    if (len > 0) {
      build(1, 0, len - 1, a);
    }
  }

  int count_leq(int lo, int hi, const T &x) const {
    return query(1, 0, len - 1, lo, hi, [&](const std::vector<T> &v) {
      return static_cast<int>(std::upper_bound(v.begin(), v.end(), x) - v.begin());
    });
  }

  int count_in(int lo, int hi, const T &x, const T &y) const {
    return query(1, 0, len - 1, lo, hi, [&](const std::vector<T> &v) {
      return static_cast<int>(
          std::upper_bound(v.begin(), v.end(), y) - std::lower_bound(v.begin(), v.end(), x)
      );
    });
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{5, 2, 8, 6, 1, 9, 3};
  MergeSortTree<int> t(a);

  assert(t.count_leq(0, 6, 5) == 4);    // 5, 2, 1, 3
  assert(t.count_leq(2, 4, 6) == 2);    // 6, 1
  assert(t.count_in(0, 6, 3, 8) == 4);  // 5, 8, 6, 3
  assert(t.count_in(1, 5, 2, 6) == 2);  // 2, 6 (8, 1, 9 excluded)
  return 0;
}
