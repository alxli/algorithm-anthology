/*

Given a set of pairs $(m, b)$ specifying lines of the form $y = mx + b$, process a set of
$x$-coordinate queries each asking to find the minimum $y$-value when any of the given lines are
evaluated at the specified $x$. To instead have the queries optimize for maximum $y$-value, call the
constructor with `query_max = true`. This is useful for dynamic programming recurrences of the form
`dp[i] = min(m[j] * x[i] + b[j])` when line slopes and query coordinates are not monotone.

The following implementation is a fully dynamic variant of the convex hull optimization technique,
using a self-balancing binary search tree (`std::set`) to support the ability to call `add_line()`
and `query()` in any desired order. The tree stores the lower envelope of the lines sorted by slope:
inserting a line removes any neighbors it dominates, and each line records the interval of queries
for which it is the best, so a query is a single tree lookup.

- `HullOptimizer(query_max)` constructs an empty hull. By default, `query(x)` minimizes; if
  `query_max` is true, `query(x)` maximizes.
- `add_line(m, b)` inserts line $y = mx + b$ (can be called in any order).
- `query(x)` returns the best $y$-value among all inserted lines at coordinate `x`. At least one
  line must have been inserted.

Time Complexity:
- O(log n) amortized per call to `add_line()` and O(log n) per call to `query()`, where $n$ is the
  number of lines added.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for `add_line()` and `query()`.

*/

#include <cassert>
#include <cstdint>
#include <set>

class HullOptimizer {
  struct Line {
    int64_t m, b;
    mutable int64_t xhi;
    bool is_query;

    Line(int64_t m, int64_t b, int64_t xhi = 0, bool is_query = false)
        : m(m), b(b), xhi(xhi), is_query(is_query) {}

    bool operator<(const Line &l) const { return l.is_query ? xhi < l.xhi : m < l.m; }
  };

  std::multiset<Line> hull;
  bool query_max;

  using hulliter = std::multiset<Line>::iterator;

  static int64_t div_floor(int64_t a, int64_t b) { return a / b - ((a ^ b) < 0 && a % b); }

  bool update_border(hulliter x, hulliter y) {
    if (y == hull.end()) {
      x->xhi = INT64_MAX;
      return false;
    }
    if (x->m == y->m) {
      x->xhi = (x->b > y->b) ? INT64_MAX : INT64_MIN;
    } else {
      x->xhi = div_floor(y->b - x->b, x->m - y->m);
    }
    return x->xhi >= y->xhi;
  }

 public:
  explicit HullOptimizer(bool query_max = false) : query_max(query_max) {}

  void add_line(int64_t m, int64_t b) {
    if (!query_max) {
      m = -m;
      b = -b;
    }
    auto z = hull.insert(Line(m, b));
    auto y = z++;
    auto x = y;
    while (update_border(y, z)) {
      z = hull.erase(z);
    }
    if (x != hull.begin() && update_border(--x, y)) {
      update_border(x, y = hull.erase(y));
    }
    while ((y = x) != hull.begin() && (--x)->xhi >= y->xhi) {
      update_border(x, hull.erase(y));
    }
  }

  int64_t query(int64_t x) const {
    assert(!hull.empty());
    Line q(0, 0, x, true);
    auto it = hull.lower_bound(q);
    int64_t res = it->m * x + it->b;
    return query_max ? res : -res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  HullOptimizer h;
  h.add_line(3, 0);
  h.add_line(0, 6);
  h.add_line(1, 2);
  h.add_line(2, 1);
  assert(h.query(0) == 0);
  assert(h.query(2) == 4);
  assert(h.query(1) == 3);
  assert(h.query(3) == 5);

  HullOptimizer mx(true);
  mx.add_line(3, 0);
  mx.add_line(0, 6);
  mx.add_line(1, 2);
  assert(mx.query(0) == 6);
  assert(mx.query(3) == 9);
  return 0;
}
