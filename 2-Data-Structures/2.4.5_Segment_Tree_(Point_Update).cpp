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

- `SegTree<T>(n, v = T{})` constructs an array of size `n` with indices $[0, `n`)$, and all values
  initialized to `v`.
- `SegTree<T>(lo, hi)` constructs an array from the half-open random-access iterator range
  $[`lo`, `hi`)$.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the aggregate of the values at indices in $[`lo`, `hi`]$. If `lo == hi`,
  then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range $[`lo`, `hi`)$ satisfies `pred()`. As `hi` increases, `pred()` applied to this aggregate may
  change only from true to false. The empty range is valid, and `size()` is returned if the
  predicate remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range $[`lo`, `hi`)$ satisfies `pred()`. As `lo` decreases, `pred()` applied to this aggregate may
  change only from true to false. The empty range is valid, and $0$ is returned if the predicate
  remains true to the beginning.

For the boundary-search functions, `pred()` takes aggregate `T` values. For `combine = min`,
`pred(mn) = (mn > x)` makes `max_right()` stop at the first value `<= x` when extending right, while
`min_left()` stops just after the first such value when extending left. With nonnegative values and
`combine = sum`, `pred(sum) = (sum <= x)` finds the longest extension with sum at most `x`.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()`, `query()`, `update()`, `max_right()`, and `min_left()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(log n) auxiliary stack space for `query()`, `update()`, `max_right()`, and `min_left()`.
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

  template<typename Gen>
  void build(int i, int lo, int hi, const Gen &gen) {
    if (lo == hi) {
      value[i] = gen(lo);
      return;
    }
    int mid = lo + (hi - lo) / 2;
    build(i * 2 + 1, lo, mid, gen);
    build(i * 2 + 2, mid + 1, hi, gen);
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
    if (lo == hi) {
      value[i] = apply_delta(value[i], d);
      return;
    }
    int mid = lo + (hi - lo) / 2;
    if (target <= mid) {
      update(i * 2 + 1, lo, mid, target, d);
    } else {
      update(i * 2 + 2, mid + 1, hi, target, d);
    }
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
  explicit SegTree(int n, const T &v = T{}) : len(n) {
    assert(len > 0);
    value.assign(4 * len, v);
    build(0, 0, len - 1, [&](int) { return v; });
  }

  template<typename It>
  SegTree(It lo, It hi) : len(hi - lo) {
    assert(len > 0);
    value.assign(4 * len, *lo);
    build(0, 0, len - 1, [&](int i) { return *(lo + i); });
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

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  SegTree<int> t(a.begin(), a.end());
  t.update(2, 4);
  vector<int> expected{6, -2, 4, 8, 10};
  for (int i = 0; i < t.size(); i++) {
    assert(t.at(i) == expected[i]);
  }
  assert(t.query(0, 3) == -2);

  // Boundary search by accumulated aggregate: stop before including the -2 at index 1.
  assert(t.max_right(0, [](int mn) { return mn > -2; }) == 1);
  assert(t.min_left(5, [](int mn) { return mn > -2; }) == 2);
  assert(t.max_right(2, [](int mn) { return mn >= 4; }) == t.size());
  assert(t.min_left(2, [](int mn) { return mn >= 0; }) == 2);
  return 0;
}
