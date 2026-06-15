/*

Maintain a fixed-size array while supporting point updates and range aggregate queries using an
iterative segment tree. The leaves are stored starting at a power-of-two offset in a flat array and
every internal node stores the aggregate of its two children, so point updates rebuild ancestors by
walking upward instead of recursing. Unlike the recursive segment tree, this layout avoids recursive
stack usage for ordinary queries and updates. It is especially convenient for fast point-update
segment trees; lazy propagation is usually clearer in the recursive version.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return `a + b`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at a
single updated index. The default definition below supports updates that "set" the chosen array
index to a new value. Another possible update operation is "increment", in which case
`apply_delta(v, d)` should return `v + d`.

- `IterativeSegTree(n, v)` constructs an array of size `n` with indices from 0 to `n - 1`,
  inclusive, and all values initialized to `v`.
- `IterativeSegTree(lo, hi)` constructs an array from two random-access iterators as a range
  `[lo, hi)`, initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `find_first(lo, hi, pred)` returns the smallest index in `[lo, hi]` matching the search, or $-1$
  if none. `pred(v)` takes a node aggregate and must be monotone: if it is false, no element under
  that node qualifies.
- `find_last(lo, hi, pred)` is the analogous query returning the largest such index.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range `[lo, hi)` satisfies `pred`, or `size()` if the predicate remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range `[lo, hi)` satisfies `pred`, or 0 if the predicate remains true to the beginning.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()` and `at()`.
- O(log n) per call to `update()`, `query()`, `find_first()`, `find_last()`, `max_right()`, and
  `min_left()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary space for `query()` and `update()`, and O(log n) for boundary searches.

*/

#include <algorithm>
#include <optional>
#include <vector>

template<class T>
class IterativeSegTree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d) { return d; }

  int len, base;
  std::vector<T> value;

  template<class Pred>
  int find_first(int i, int lo, int hi, int qlo, int qhi, const Pred &pred) const {
    if (hi <= qlo || qhi <= lo || len <= lo) {
      return -1;
    }
    if (qlo <= lo && hi <= qhi && hi <= len && !pred(value[i])) {
      return -1;
    }
    if (hi - lo == 1) {
      return lo;
    }
    int mid = (lo + hi) / 2;
    int res = find_first(i << 1, lo, mid, qlo, qhi, pred);
    return res != -1 ? res : find_first(i << 1 | 1, mid, hi, qlo, qhi, pred);
  }

  template<class Pred>
  int find_last(int i, int lo, int hi, int qlo, int qhi, const Pred &pred) const {
    if (hi <= qlo || qhi <= lo || len <= lo) {
      return -1;
    }
    if (qlo <= lo && hi <= qhi && hi <= len && !pred(value[i])) {
      return -1;
    }
    if (hi - lo == 1) {
      return lo;
    }
    int mid = (lo + hi) / 2;
    int res = find_last(i << 1 | 1, mid, hi, qlo, qhi, pred);
    return res != -1 ? res : find_last(i << 1, lo, mid, qlo, qhi, pred);
  }

  template<class Pred>
  int max_right(int i, int lo, int hi, int qlo, const Pred &pred, std::optional<T> &acc) const {
    if (hi <= qlo || len <= lo) {
      return -1;
    }
    if (qlo <= lo && hi <= len) {
      T next = acc ? combine(*acc, value[i]) : value[i];
      if (pred(next)) {
        acc = next;
        return -1;
      }
      if (hi - lo == 1) {
        return lo;
      }
    }
    int mid = (lo + hi) / 2;
    int res = max_right(i << 1, lo, mid, qlo, pred, acc);
    return res != -1 ? res : max_right(i << 1 | 1, mid, hi, qlo, pred, acc);
  }

  template<class Pred>
  int min_left(int i, int lo, int hi, int qhi, const Pred &pred, std::optional<T> &acc) const {
    if (qhi <= lo || len <= lo) {
      return -1;
    }
    if (hi <= qhi && hi <= len) {
      T next = acc ? combine(value[i], *acc) : value[i];
      if (pred(next)) {
        acc = next;
        return -1;
      }
      if (hi - lo == 1) {
        return hi;
      }
    }
    int mid = (lo + hi) / 2;
    int res = min_left(i << 1 | 1, mid, hi, qhi, pred, acc);
    return res != -1 ? res : min_left(i << 1, lo, mid, qhi, pred, acc);
  }

 public:
  explicit IterativeSegTree(int n, const T &v = T()) : len(n), base(1) {
    while (base < len) {
      base <<= 1;
    }
    value.assign(2 * base, T());
    for (int i = 0; i < len; i++) {
      value[base + i] = v;
    }
    for (int i = base - 1; i > 0; i--) {
      value[i] = combine(value[i << 1], value[i << 1 | 1]);
    }
  }

  template<class It>
  IterativeSegTree(It lo, It hi) : len(hi - lo), base(1) {
    while (base < len) {
      base <<= 1;
    }
    value.assign(2 * base, T());
    for (int i = 0; i < len; i++) {
      value[base + i] = *(lo + i);
    }
    for (int i = base - 1; i > 0; i--) {
      value[i] = combine(value[i << 1], value[i << 1 | 1]);
    }
  }

  int size() const { return len; }
  T at(int i) const { return value[base + i]; }

  T query(int lo, int hi) const {
    std::optional<T> left, right;
    for (lo += base, hi += base + 1; lo < hi; lo >>= 1, hi >>= 1) {
      if (lo & 1) {
        left = left ? combine(*left, value[lo++]) : value[lo++];
      }
      if (hi & 1) {
        right = right ? combine(value[--hi], *right) : value[--hi];
      }
    }
    if (!left) {
      return *right;
    }
    if (!right) {
      return *left;
    }
    return combine(*left, *right);
  }

  void update(int i, const T &d) {
    i += base;
    value[i] = apply_delta(value[i], d);
    for (i >>= 1; i > 0; i >>= 1) {
      value[i] = combine(value[i << 1], value[i << 1 | 1]);
    }
  }

  template<class Pred>
  int find_first(int lo, int hi, const Pred &pred) const {
    return find_first(1, 0, base, lo, hi + 1, pred);
  }

  template<class Pred>
  int find_last(int lo, int hi, const Pred &pred) const {
    return find_last(1, 0, base, lo, hi + 1, pred);
  }

  template<class Pred>
  int max_right(int lo, const Pred &pred) const {
    std::optional<T> acc;
    int res = max_right(1, 0, base, lo, pred, acc);
    return res == -1 ? len : res;
  }

  template<class Pred>
  int min_left(int hi, const Pred &pred) const {
    std::optional<T> acc;
    int res = min_left(1, 0, base, hi, pred, acc);
    return res == -1 ? 0 : res;
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
  IterativeSegTree<int> t(a.begin(), a.end());
  t.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  cout << "The minimum value in the range [0, 3] is " << t.query(0, 3) << "." << endl;
  assert(t.query(0, 3) == -2);

  auto at_most_4 = [](int v) { return v <= 4; };
  assert(t.find_first(0, 4, at_most_4) == 1);
  assert(t.find_last(0, 4, at_most_4) == 2);
  assert(t.find_first(0, 4, [](int v) { return v <= -5; }) == -1);

  assert(t.max_right(0, [](int mn) { return mn > -2; }) == 1);
  assert(t.min_left(5, [](int mn) { return mn > -2; }) == 2);
  return 0;
}
