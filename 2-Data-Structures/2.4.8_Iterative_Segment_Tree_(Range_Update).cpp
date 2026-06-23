/*

Maintain a fixed-size array while supporting range updates and range aggregate queries using a
bottom-up lazy segment tree. Like the point-update iterative tree, leaves are stored starting at a
power-of-two offset in a flat array. Each internal node stores an aggregate value, plus an optional
pending lazy delta that is pushed down only along paths that need to inspect children. Compared with
the recursive lazy tree, this version keeps ordinary range updates and queries in flat loops, which
can reduce constant factors. The recursive version is usually easier to adapt when the lazy
operation or search logic becomes unusual.

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

- `IterativeLazySegTree<T>(n, v)` constructs an array of size `n` with indices [$0$, `n`), and all
  values initialized to `v`.
- `IterativeLazySegTree<T>(lo, hi)` constructs an array from two random-access iterators as a range
  [`lo`, `hi`), initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value at index `i` by applying the delta `d`.
- `update(lo, hi, d)` modifies the value at each array index from `lo` to `hi`, inclusive, by
  applying the delta `d` to each value.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range [`lo`, `hi`) satisfies `pred`. It returns `size()` if `pred` remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range [`lo`, `hi`) satisfies `pred`. It returns $0$ if `pred` remains true to the beginning.

For the boundary-search functions, `pred` takes aggregate `T` values of candidate ranges. As a range
grows, `pred` may change from true to false but never back to true; The empty range is considered
valid. E.g. for `combine = min`, use `pred(mn) = (mn > x)` to find the first value `<= x`, or for
`combine = sum`, use `pred(sum) = (sum <= x)` with nonnegative values to find the longest range
within the limit `x`.

Both boundary searches are iterative to demonstrate how the flat-array layout can avoid recursion.
They push lazy deltas along the endpoints, decompose the search range into O(log n) canonical nodes,
scan those nodes in order while accumulating their aggregates, then push and descend inside the
first node that makes `pred` false. A recursive descent like the one in 2.4.6 is equally valid and
may be easier to adapt.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()`, `update()`, `query()`, `max_right()`, and `min_left()`.

Space Complexity:
- O(n) for storage of the array elements and lazy deltas.
- O(1) auxiliary space for all operations.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <vector>

template<typename T>
class IterativeLazySegTree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d, int64_t len) { return d; }
  static T compose_deltas(const T &d1, const T &d2) { return d2; }

  int len, base, height;
  std::vector<T> value, delta;
  std::vector<int64_t> seg_len;
  std::vector<char> pending;

  void init_storage() {
    base = 1;
    height = 0;
    while (base < len) {
      base <<= 1;
      height++;
    }
    value.assign(2 * base, T());
    delta.assign(2 * base, T());
    pending.assign(2 * base, false);
    seg_len.assign(2 * base, 1);
    for (int i = base - 1; i > 0; i--) {
      seg_len[i] = seg_len[i << 1] + seg_len[i << 1 | 1];
    }
  }

  void pull(int i) { value[i] = combine(value[i << 1], value[i << 1 | 1]); }

  void apply(int i, const T &d) {
    value[i] = apply_delta(value[i], d, seg_len[i]);
    if (i < base) {
      delta[i] = pending[i] ? compose_deltas(delta[i], d) : d;
      pending[i] = true;
    }
  }

  void push(int i) {
    if (pending[i]) {
      apply(i << 1, delta[i]);
      apply(i << 1 | 1, delta[i]);
      pending[i] = false;
    }
  }

  void push_path(int i) {
    for (int h = height; h > 0; h--) {
      push(i >> h);
    }
  }

  void pull_after_update(int l, int r) {
    for (int h = 1; h <= height; h++) {
      if (((l >> h) << h) != l) {
        pull(l >> h);
      }
      if (((r >> h) << h) != r) {
        pull((r - 1) >> h);
      }
    }
  }

 public:
  explicit IterativeLazySegTree(int n, const T &v = T()) : len(n) {
    assert(len > 0);
    init_storage();
    for (int i = 0; i < len; i++) {
      value[base + i] = v;
    }
    for (int i = base - 1; i > 0; i--) {
      pull(i);
    }
  }

  template<typename It>
  IterativeLazySegTree(It lo, It hi) : len(hi - lo) {
    assert(len > 0);
    init_storage();
    for (int i = 0; i < len; i++) {
      value[base + i] = *(lo + i);
    }
    for (int i = base - 1; i > 0; i--) {
      pull(i);
    }
  }

  int size() const { return len; }

  T at(int i) {
    assert(0 <= i && i < len);
    i += base;
    push_path(i);
    return value[i];
  }

  T query(int lo, int hi) {
    assert(0 <= lo && lo <= hi && hi < len);
    int l = lo + base, r = hi + 1 + base;
    push_path(l);
    push_path(r - 1);
    std::optional<T> left, right;
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) {
        left = left ? combine(*left, value[l++]) : value[l++];
      }
      if (r & 1) {
        right = right ? combine(value[--r], *right) : value[--r];
      }
    }
    return !left ? *right : (!right ? *left : combine(*left, *right));
  }

  void update(int lo, int hi, const T &d) {
    assert(0 <= lo && lo <= hi && hi < len);
    int l = lo + base, r = hi + 1 + base;
    push_path(l);
    push_path(r - 1);
    int l0 = l, r0 = r;
    for (; l < r; l >>= 1, r >>= 1) {
      if (l & 1) {
        apply(l++, d);
      }
      if (r & 1) {
        apply(--r, d);
      }
    }
    pull_after_update(l0, r0);
  }

  void update(int i, const T &d) {
    assert(0 <= i && i < len);
    update(i, i, d);
  }

  template<typename Pred>
  int max_right(int lo, const Pred &pred) {
    assert(0 <= lo && lo <= len);
    if (lo == len) {
      return len;
    }
    push_path(lo + base);
    push_path(len - 1 + base);
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
        push(i);
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
  int min_left(int hi, const Pred &pred) {
    assert(0 <= hi && hi <= len);
    if (hi == 0) {
      return 0;
    }
    push_path(base);
    push_path(hi - 1 + base);
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
        push(i);
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

/*** Example Usage and Output:

Values: 6 -2 4 8 10
Values: 5 5 5 1 5

***/

#include <iostream>
using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  IterativeLazySegTree<int> t(a.begin(), a.end());
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

  // Boundary search works through lazy updates too; values are now {5, 5, 5, 1, 5}.
  assert(t.max_right(0, [](int mn) { return mn > 1; }) == 3);
  assert(t.min_left(5, [](int mn) { return mn > 1; }) == 4);
  return 0;
}
