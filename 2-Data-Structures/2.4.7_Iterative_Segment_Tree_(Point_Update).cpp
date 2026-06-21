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

- `IterativeSegTree<T>(n, v)` constructs an array of size `n` with indices [0, `n`), and all values
  initialized to `v`.
- `IterativeSegTree<T>(lo, hi)` constructs an array from two random-access iterators as a range
  [`lo`, `hi`), initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range [`lo`, `hi`) satisfies `pred`. It returns `size()` if `pred` remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range [`lo`, `hi`) satisfies `pred`. It returns 0 if `pred` remains true to the beginning.

For the boundary-search functions, `pred` takes aggregate `T` values of candidate ranges. As a range
grows, `pred` may change from true to false but never back to true; The empty range is considered
valid. E.g. for `combine = min`, use `pred(mn) = (mn > x)` to find the first value `<= x`, or for
`combine = sum`, use `pred(sum) = (sum <= x)` with nonnegative values to find the longest range
within the limit `x`.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()` and `at()`.
- O(log n) per call to `update()`, `query()`, `max_right()`, and `min_left()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary space for `query()` and `update()`, and O(log n) for boundary searches.

*/

#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

template<typename T>
class IterativeSegTree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d) { return d; }

  int len, base;
  std::vector<T> value;

  template<typename Pred>
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

  template<typename Pred>
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
    assert(len > 0);
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

  template<typename It>
  IterativeSegTree(It lo, It hi) : len(hi - lo), base(1) {
    assert(len > 0);
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
  
  T at(int i) const {
    assert(0 <= i && i < len);
    return value[base + i];
  }

  T query(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi < len);
    std::optional<T> left, right;
    for (lo += base, hi += base + 1; lo < hi; lo >>= 1, hi >>= 1) {
      if (lo & 1) {
        left = left ? combine(*left, value[lo++]) : value[lo++];
      }
      if (hi & 1) {
        right = right ? combine(value[--hi], *right) : value[--hi];
      }
    }
    return !left ? *right : (!right ? *left : combine(*left, *right));
  }

  void update(int i, const T &d) {
    assert(0 <= i && i < len);
    i += base;
    value[i] = apply_delta(value[i], d);
    for (i >>= 1; i > 0; i >>= 1) {
      value[i] = combine(value[i << 1], value[i << 1 | 1]);
    }
  }

  template<typename Pred>
  int max_right(int lo, const Pred &pred) const {
    assert(0 <= lo && lo <= len);
    std::optional<T> acc;
    int res = max_right(1, 0, base, lo, pred, acc);
    return res == -1 ? len : res;
  }

  template<typename Pred>
  int min_left(int hi, const Pred &pred) const {
    assert(0 <= hi && hi <= len);
    std::optional<T> acc;
    int res = min_left(1, 0, base, hi, pred, acc);
    return res == -1 ? 0 : res;
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
The minimum value in the range [0, 3] is -2.

***/

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

  assert(t.max_right(0, [](int mn) { return mn > -2; }) == 1);
  assert(t.min_left(5, [](int mn) { return mn > -2; }) == 2);
  return 0;
}
