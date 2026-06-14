/*

Maintain a fixed-size array while supporting point updates and range aggregate queries. A segment
tree stores one aggregate value for each recursively split interval, so an update only rebuilds the
O(log n) intervals on the path from one leaf to the root.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return `a + b`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at a
single updated index. The default definition below supports updates that "set" the chosen array
index to a new value. Another possible update operation is "increment", in which case
`apply_delta(v, d)` should return `v + d`.

- `SegTree(n, v)` constructs an array of size `n` with indices from 0 to `n - 1`, inclusive,
  and all values initialized to `v`.
- `SegTree(lo, hi)` constructs an array from two random-access iterators as a range `[lo, hi)`,
  initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `find_first(lo, hi, pred)` returns the smallest index in `[lo, hi]` matching the search, or $-1$
  if none, in O(log n). `pred(v)` takes a node aggregate and must be monotone: if it is false, no
  element under that node qualifies (e.g. for the default min tree, `pred(v) = (v <= x)` finds the
  leftmost element `<= x`).
- `find_last(lo, hi, pred)` is the analogous query returning the largest such index.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()`, `update()`, `query()`, `find_first()`, and `find_last()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(log n) auxiliary stack space for `update()` and `query()`.
- O(1) auxiliary for `size()`.

*/

#include <algorithm>
#include <vector>

template<class T>
class SegTree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d) { return d; }

  int len;
  std::vector<T> value;

  void build(int i, int lo, int hi, const T &v) {
    if (lo == hi) {
      value[i] = v;
      return;
    }
    int mid = lo + (hi - lo) / 2;
    build(i * 2 + 1, lo, mid, v);
    build(i * 2 + 2, mid + 1, hi, v);
    value[i] = combine(value[i * 2 + 1], value[i * 2 + 2]);
  }

  template<class It>
  void build(int i, int lo, int hi, It arr) {
    if (lo == hi) {
      value[i] = *(arr + lo);
      return;
    }
    int mid = lo + (hi - lo) / 2;
    build(i * 2 + 1, lo, mid, arr);
    build(i * 2 + 2, mid + 1, hi, arr);
    value[i] = combine(value[i * 2 + 1], value[i * 2 + 2]);
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) const {
    if (lo == tgt_lo && hi == tgt_hi) {
      return value[i];
    }
    int mid = lo + (hi - lo) / 2;
    if (tgt_lo <= mid && mid < tgt_hi) {
      return combine(
          query(i * 2 + 1, lo, mid, tgt_lo, std::min(tgt_hi, mid)),
          query(i * 2 + 2, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi)
      );
    }
    if (tgt_lo <= mid) {
      return query(i * 2 + 1, lo, mid, tgt_lo, std::min(tgt_hi, mid));
    }
    return query(i * 2 + 2, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi);
  }

  void update(int i, int lo, int hi, int target, const T &d) {
    if (target < lo || target > hi) {
      return;
    }
    if (lo == hi) {
      value[i] = apply_delta(value[i], d);
      return;
    }
    int mid = lo + (hi - lo) / 2;
    update(i * 2 + 1, lo, mid, target, d);
    update(i * 2 + 2, mid + 1, hi, target, d);
    value[i] = combine(value[i * 2 + 1], value[i * 2 + 2]);
  }

  template<class Pred>
  int find_first(int i, int lo, int hi, int tgt_lo, int tgt_hi, const Pred &pred) const {
    if (tgt_hi < lo || hi < tgt_lo || !pred(value[i])) {
      return -1;
    }
    if (lo == hi) {
      return lo;
    }
    int mid = lo + (hi - lo) / 2;
    int res = find_first(i * 2 + 1, lo, mid, tgt_lo, tgt_hi, pred);
    return res != -1 ? res : find_first(i * 2 + 2, mid + 1, hi, tgt_lo, tgt_hi, pred);
  }

  template<class Pred>
  int find_last(int i, int lo, int hi, int tgt_lo, int tgt_hi, const Pred &pred) const {
    if (tgt_hi < lo || hi < tgt_lo || !pred(value[i])) {
      return -1;
    }
    if (lo == hi) {
      return lo;
    }
    int mid = lo + (hi - lo) / 2;
    int res = find_last(i * 2 + 2, mid + 1, hi, tgt_lo, tgt_hi, pred);
    return res != -1 ? res : find_last(i * 2 + 1, lo, mid, tgt_lo, tgt_hi, pred);
  }

 public:
  explicit SegTree(int n, const T &v = T()) : len(n), value(4 * len) {
    if (len > 0) {
      build(0, 0, len - 1, v);
    }
  }

  template<class It>
  SegTree(It lo, It hi) : len(hi - lo), value(4 * len) {
    if (len > 0) {
      build(0, 0, len - 1, lo);
    }
  }

  int size() const { return len; }
  T at(int i) const { return query(i, i); }
  T query(int lo, int hi) const { return query(0, 0, len - 1, lo, hi); }
  void update(int i, const T &d) { update(0, 0, len - 1, i, d); }

  template<class Pred>
  int find_first(int lo, int hi, const Pred &pred) const {
    return find_first(0, 0, len - 1, lo, hi, pred);
  }

  template<class Pred>
  int find_last(int lo, int hi, const Pred &pred) const {
    return find_last(0, 0, len - 1, lo, hi, pred);
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
The minimum value in the range [0, 3] is -2.

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  SegTree<int> t(a.begin(), a.end());
  t.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  cout << "The minimum value in the range [0, 3] is " << t.query(0, 3) << "." << endl;
  assert(t.query(0, 3) == -2);

  // Segment-tree descent: leftmost/rightmost index in a range with value <= a threshold.
  auto at_most_4 = [](int v) { return v <= 4; };
  assert(t.find_first(0, 4, at_most_4) == 1);                       // value -2 at index 1
  assert(t.find_last(0, 4, at_most_4) == 2);                        // value 4 at index 2
  assert(t.find_first(0, 4, [](int v) { return v <= -5; }) == -1);  // no value <= -5
  return 0;
}
