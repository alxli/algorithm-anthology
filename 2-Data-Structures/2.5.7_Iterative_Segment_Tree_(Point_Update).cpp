/*

Maintain a fixed-size array while supporting point updates and range aggregate queries using an
iterative segment tree. The leaves begin at a power-of-two offset in a flat array, and each internal
node stores the aggregate of its two children. Point updates rebuild ancestors by walking upward,
while queries scan a decomposition of the requested range. Compared with the recursive segment tree,
these flat loops avoid recursive stack usage and can reduce constant factors.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return `a + b`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at a
single updated index. The default definition below supports updates that "set" the chosen array
index to a new value. Another possible update operation is "increment", in which case
`apply_delta(v, d)` should return `v + d`.

- `IterativeSegTree<T>(n, v = T{})` constructs an array of size `n` with indices $[0, `n`)$, and all
  values initialized to `v`.
- `IterativeSegTree<T>(lo, hi)` constructs an array from the half-open random-access iterator range
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

Both boundary searches are iterative to demonstrate how the flat-array layout can avoid recursion.
They decompose the search range into O(log n) canonical nodes, scan those nodes in order while
accumulating their aggregates, then descend inside the first node that makes `pred` false. A
recursive descent like the one in 2.5.5 is equally valid and may be easier to adapt.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()` and `at()`.
- O(log n) per call to `query()`, `update()`, `max_right()`, and `min_left()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

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

  template<typename Gen>
  void build(const Gen &gen) {
    base = 1;
    while (base < len) {
      base <<= 1;
    }
    value.assign(2 * base, gen(0));
    for (int i = 1; i < len; i++) {
      value[base + i] = gen(i);
    }
    for (int i = base - 1; i > 0; i--) {
      value[i] = combine(value[i << 1], value[i << 1 | 1]);
    }
  }

 public:
  explicit IterativeSegTree(int n, const T &v = T{}) : len(n) {
    assert(len > 0);
    build([&](int) { return v; });
  }

  template<typename It>
  IterativeSegTree(It lo, It hi) : len(hi - lo) {
    assert(len > 0);
    build([&](int i) { return *(lo + i); });
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
    for (int pos = lo; pos < len;) {
      int width = pos == 0 ? base : pos & -pos;
      while (width > len - pos) {
        width >>= 1;
      }
      int i = (base + pos) / width;
      T next = acc ? combine(*acc, value[i]) : value[i];
      if (pred(next)) {
        acc = next;
        pos += width;
        continue;
      }
      while (i < base) {
        i <<= 1;
        next = acc ? combine(*acc, value[i]) : value[i];
        if (pred(next)) {
          acc = next;
          i++;
        }
      }
      return i - base;
    }
    return len;
  }

  template<typename Pred>
  int min_left(int hi, const Pred &pred) const {
    assert(0 <= hi && hi <= len);
    std::optional<T> acc;
    for (int pos = hi; pos > 0;) {
      int width = pos & -pos;
      int i = (base + pos) / width - 1;
      T next = acc ? combine(value[i], *acc) : value[i];
      if (pred(next)) {
        acc = next;
        pos -= width;
        continue;
      }
      while (i < base) {
        i = i << 1 | 1;
        next = acc ? combine(value[i], *acc) : value[i];
        if (pred(next)) {
          acc = next;
          i--;
        }
      }
      return i - base + 1;
    }
    return 0;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  IterativeSegTree<int> t(a.begin(), a.end());
  t.update(2, 4);
  vector<int> expected{6, -2, 4, 8, 10};
  for (int i = 0; i < t.size(); i++) {
    assert(t.at(i) == expected[i]);
  }
  assert(t.query(0, 3) == -2);

  assert(t.max_right(0, [](int mn) { return mn > -2; }) == 1);
  assert(t.min_left(5, [](int mn) { return mn > -2; }) == 2);
  return 0;
}
