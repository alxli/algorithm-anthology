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

- `SegTree<T>(n, v)` constructs an array of size `n` with indices $[0, `n`)$, and all values
  initialized to `v`.
- `SegTree<T>(lo, hi)` constructs an array from two random-access iterators as a range
  $[`lo`, `hi`)$, initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range $[`lo`, `hi`)$ satisfies `pred`. It returns `size()` if `pred` remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range $[`lo`, `hi`)$ satisfies `pred`. It returns $0$ if `pred` remains true to the beginning.

For the boundary-search functions, `pred` takes aggregate `T` values of candidate ranges. As a range
grows, `pred` may change from true to false but never back to true; The empty range is considered
valid. E.g. for `combine = min`, use `pred(mn) = (mn > x)` to find the first value `<= x`, or for
`combine = sum`, use `pred(sum) = (sum <= x)` with nonnegative values to find the longest range
within the limit `x`.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()`, `update()`, `query()`, `max_right()`, and `min_left()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(log n) auxiliary stack space for `update()`, `query()`, `max_right()`, and `min_left()`.
- O(1) auxiliary for `size()`.

*/

#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

template<typename T>
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

  template<typename It>
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

  template<typename Pred>
  int max_right(int i, int lo, int hi, int tgt_lo, const Pred &pred, std::optional<T> &acc) const {
    if (hi < tgt_lo) {
      return -1;
    }
    if (tgt_lo <= lo) {
      T next = acc ? combine(*acc, value[i]) : value[i];
      if (pred(next)) {
        acc = next;
        return -1;
      }
      if (lo == hi) {
        return lo;
      }
    }
    int mid = lo + (hi - lo) / 2;
    int res = max_right(i * 2 + 1, lo, mid, tgt_lo, pred, acc);
    return res != -1 ? res : max_right(i * 2 + 2, mid + 1, hi, tgt_lo, pred, acc);
  }

  template<typename Pred>
  int min_left(int i, int lo, int hi, int tgt_hi, const Pred &pred, std::optional<T> &acc) const {
    if (tgt_hi <= lo) {
      return -1;
    }
    if (hi < tgt_hi) {
      T next = acc ? combine(value[i], *acc) : value[i];
      if (pred(next)) {
        acc = next;
        return -1;
      }
      if (lo == hi) {
        return lo + 1;
      }
    }
    int mid = lo + (hi - lo) / 2;
    int res = min_left(i * 2 + 2, mid + 1, hi, tgt_hi, pred, acc);
    return res != -1 ? res : min_left(i * 2 + 1, lo, mid, tgt_hi, pred, acc);
  }

 public:
  explicit SegTree(int n, const T &v = T()) : len(n), value(4 * len) {
    assert(len > 0);
    build(0, 0, len - 1, v);
  }

  template<typename It>
  SegTree(It lo, It hi) : len(hi - lo), value(4 * len) {
    assert(len > 0);
    build(0, 0, len - 1, lo);
  }

  int size() const { return len; }

  T at(int i) const {
    assert(0 <= i && i < len);
    return query(i, i);
  }

  T query(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi < len);
    return query(0, 0, len - 1, lo, hi);
  }

  void update(int i, const T &d) {
    assert(0 <= i && i < len);
    update(0, 0, len - 1, i, d);
  }

  template<typename Pred>
  int max_right(int lo, const Pred &pred) const {
    assert(0 <= lo && lo <= len);
    std::optional<T> acc;
    int res = max_right(0, 0, len - 1, lo, pred, acc);
    return res == -1 ? len : res;
  }

  template<typename Pred>
  int min_left(int hi, const Pred &pred) const {
    assert(0 <= hi && hi <= len);
    std::optional<T> acc;
    int res = min_left(0, 0, len - 1, hi, pred, acc);
    return res == -1 ? 0 : res;
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
The minimum value in the range [$0$, 3] is -2.

***/

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
  cout << "The minimum value in the range [$0$, 3] is " << t.query(0, 3) << "." << endl;
  assert(t.query(0, 3) == -2);

  // Boundary search by accumulated aggregate: stop before including the -2 at index 1.
  assert(t.max_right(0, [](int mn) { return mn > -2; }) == 1);
  assert(t.min_left(5, [](int mn) { return mn > -2; }) == 2);
  assert(t.max_right(2, [](int mn) { return mn >= 4; }) == t.size());
  assert(t.min_left(2, [](int mn) { return mn >= 0; }) == 2);
  return 0;
}
