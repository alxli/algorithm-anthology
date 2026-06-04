/*

Maintain a fixed-size array while supporting dynamic queries of contiguous sub-arrays and dynamic
updates of individual indices.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return $a + b$.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at a
single updated index. The default definition below supports updates that "set" the chosen array
index to a new value. Another possible update operation is "increment", in which case
`apply_delta(v, d)` should return $v + d$.

- `SegTree(n, v)` constructs an array of size `n` with indices from 0 to `n - 1`, inclusive,
  and all values initialized to `v`.
- `SegTree(lo, hi)` constructs an array from two random-access iterators as a range `[lo, hi)`,
  initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()`, `update()`, and `query()`.

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
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
The minimum value in the range [0, 3] is -2.

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  int arr[5] = {6, -2, 1, 8, 10};
  SegTree<int> t(arr, arr + 5);
  t.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == -2);
  return 0;
}
