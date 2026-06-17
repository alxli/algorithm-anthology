/*

Maintain a fixed-size array while supporting range updates and range aggregate queries. A lazy
segment tree stores one aggregate value for each recursively split interval, plus pending updates
that are pushed to children only when needed.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return `a + b`.

Range updates are defined by `apply_delta(v, d, len)`, which applies an update delta `d` to an
aggregate summary `v` representing `len` array values, and by `compose_deltas(old, d)`, which
combines a pending older delta with a newer delta in that order. These functions do not support
arbitrary combinations: applying a delta to a combined segment must be equivalent to applying it to
each child segment and then combining the results, and composed deltas must be equivalent to
performing their updates sequentially. The default code below defines range assignment. For range
increment, `compose_deltas(old, d)` should return `old + d`; `apply_delta(v, d, len)` should return
`v + d` for range-min/range-max queries, and `v + d * len` for range-sum queries.

- `LazySegTree<T>(n, v)` constructs an array of size `n` with indices [0, `n`), and all values
  initialized to `v`.
- `LazySegTree<T>(lo, hi)` constructs an array from two random-access iterators as a range
  [`lo`, `hi`), initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`, where `i` is between 0 and `size() - 1`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `update(lo, hi, d)` modifies the value at each array index from `lo` to `hi`, inclusive, by
  applying the delta `d` to each value.
- `find_first(lo, hi, pred)` returns the smallest index in [`lo`, `hi`] matching the search, or $-1$
  if none, in O(log n). `pred(v)` takes a node aggregate and must be monotone: if it is false, no
  element under that node qualifies (e.g. for the default min tree, `pred(v) = (v <= x)` finds the
  leftmost element `<= x`).
- `find_last(lo, hi, pred)` is the analogous query returning the largest such index.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range [`lo`, `hi`) satisfies `pred`, or `size()` if the predicate remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range [`lo`, `hi`) satisfies `pred`, or 0 if the predicate remains true to the beginning.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()`, `update()`, `query()`, `find_first()`, `find_last()`,
  `max_right()`, and `min_left()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(log n) auxiliary stack space for `update()` and `query()`.
- O(1) auxiliary for `size()`.

*/

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

template<class T>
class LazySegTree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d, int64_t len) { return d; }
  static T compose_deltas(const T &d1, const T &d2) { return d2; }

  int len;
  std::vector<T> value, delta;
  std::vector<bool> pending;

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

  void push_delta(int i, int lo, int hi) {
    if (pending[i]) {
      value[i] = apply_delta(value[i], delta[i], hi - lo + 1);
      if (lo != hi) {
        int l = 2 * i + 1, r = 2 * i + 2;
        delta[l] = pending[l] ? compose_deltas(delta[l], delta[i]) : delta[i];
        delta[r] = pending[r] ? compose_deltas(delta[r], delta[i]) : delta[i];
        pending[l] = pending[r] = true;
      }
      pending[i] = false;
    }
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) {
    push_delta(i, lo, hi);
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

  void update(int i, int lo, int hi, int tgt_lo, int tgt_hi, const T &d) {
    push_delta(i, lo, hi);
    if (hi < tgt_lo || lo > tgt_hi) {
      return;
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      delta[i] = d;
      pending[i] = true;
      push_delta(i, lo, hi);
      return;
    }
    update(2 * i + 1, lo, lo + (hi - lo) / 2, tgt_lo, tgt_hi, d);
    update(2 * i + 2, lo + (hi - lo) / 2 + 1, hi, tgt_lo, tgt_hi, d);
    value[i] = combine(value[2 * i + 1], value[2 * i + 2]);
  }

  template<class Pred>
  int find_first(int i, int lo, int hi, int tgt_lo, int tgt_hi, const Pred &pred) {
    push_delta(i, lo, hi);
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
  int find_last(int i, int lo, int hi, int tgt_lo, int tgt_hi, const Pred &pred) {
    push_delta(i, lo, hi);
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

  template<class Pred>
  int max_right(int i, int lo, int hi, int tgt_lo, const Pred &pred, std::optional<T> &acc) {
    push_delta(i, lo, hi);
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

  template<class Pred>
  int min_left(int i, int lo, int hi, int tgt_hi, const Pred &pred, std::optional<T> &acc) {
    push_delta(i, lo, hi);
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
  explicit LazySegTree(int n, const T &v = T())
      : len(n), value(4 * len), delta(4 * len), pending(4 * len, false) {
    if (len > 0) {
      build(0, 0, len - 1, v);
    }
  }

  template<class It>
  LazySegTree(It lo, It hi)
      : len(hi - lo), value(4 * len), delta(4 * len), pending(4 * len, false) {
    if (len > 0) {
      build(0, 0, len - 1, lo);
    }
  }

  int size() const { return len; }
  T at(int i) { return query(i, i); }
  T query(int lo, int hi) { return query(0, 0, len - 1, lo, hi); }
  void update(int i, const T &d) { update(0, 0, len - 1, i, i, d); }
  void update(int lo, int hi, const T &d) { update(0, 0, len - 1, lo, hi, d); }

  template<class Pred>
  int find_first(int lo, int hi, const Pred &pred) {
    return find_first(0, 0, len - 1, lo, hi, pred);
  }

  template<class Pred>
  int find_last(int lo, int hi, const Pred &pred) {
    return find_last(0, 0, len - 1, lo, hi, pred);
  }

  template<class Pred>
  int max_right(int lo, const Pred &pred) {
    std::optional<T> acc;
    int res = max_right(0, 0, len - 1, lo, pred, acc);
    return res == -1 ? len : res;
  }

  template<class Pred>
  int min_left(int hi, const Pred &pred) {
    std::optional<T> acc;
    int res = min_left(0, 0, len - 1, hi, pred, acc);
    return res == -1 ? 0 : res;
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
Values: 5 5 5 1 5

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  LazySegTree<int> t(a.begin(), a.end());
  t.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == -2);
  t.update(0, 4, 5);
  t.update(3, 2);
  t.update(3, 1);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == 1);

  // Segment-tree descent works through lazy updates too; values are now {5, 5, 5, 1, 5}.
  auto at_most_1 = [](int v) { return v <= 1; };
  assert(t.find_first(0, 4, at_most_1) == 3);  // only index 3 (value 1) qualifies
  assert(t.find_last(0, 4, at_most_1) == 3);
  assert(t.find_first(0, 2, at_most_1) == -1);  // no value <= 1 in [0, 2]
  assert(t.max_right(0, [](int mn) { return mn > 1; }) == 3);
  assert(t.min_left(5, [](int mn) { return mn > 1; }) == 4);
  return 0;
}
