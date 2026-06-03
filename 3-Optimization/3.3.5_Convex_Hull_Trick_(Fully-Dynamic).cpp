/*

Given a set of pairs $(m, b)$ specifying lines of the form $y = mx + b$, process a set of
x-coordinate queries each asking to find the minimum y-value when any of the given lines are
evaluated at the specified x. To instead have the queries optimize for maximum y-value, call the
constructor with `query_max = true`. This is useful for dynamic programming recurrences of the form
`dp[i] = min(m[j] * x[i] + b[j])` when line slopes and query coordinates are not monotone.

The following implementation is a fully dynamic variant of the convex hull optimization technique,
using a self-balancing binary search tree (`std::set`) to support the ability to call `add_line()`
and `query()` in any desired order.

- `HullOptimizer(query_max)` constructs an empty hull. By default, `query(x)` minimizes; if
  `query_max` is true, `query(x)` maximizes.
- `add_line(m, b)` inserts line $y = mx + b$ in arbitrary order.
- `query(x)` returns the best y-value among all inserted lines at coordinate `x`. At least one line
  must have been inserted.

Time Complexity:
- O(log n) amortized per call to `add_line()` and O(log n) per call to `query()`, where $n$ is the
  number of lines added.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for `add_line()` and `query()`.

*/

#include <cassert>
#include <climits>
#include <set>

class HullOptimizer {
  struct Line {
    long long m, b;
    mutable long long xhi;
    bool is_query;

    Line(long long m, long long b, long long xhi = 0, bool is_query = false)
        : m(m), b(b), xhi(xhi), is_query(is_query) {}

    bool operator<(const Line &l) const { return l.is_query ? xhi < l.xhi : m < l.m; }
  };

  std::multiset<Line> hull;
  bool query_max;

  typedef std::multiset<Line>::iterator hulliter;

  static long long div_floor(long long a, long long b) { return a / b - ((a ^ b) < 0 && a % b); }

  bool update_border(hulliter x, hulliter y) {
    if (y == hull.end()) {
      x->xhi = LLONG_MAX;
      return false;
    }
    if (x->m == y->m) {
      x->xhi = (x->b > y->b) ? LLONG_MAX : LLONG_MIN;
    } else {
      x->xhi = div_floor(y->b - x->b, x->m - y->m);
    }
    return x->xhi >= y->xhi;
  }

 public:
  HullOptimizer(bool query_max = false) : query_max(query_max) {}

  void add_line(long long m, long long b) {
    if (!query_max) {
      m = -m;
      b = -b;
    }
    hulliter z = hull.insert(Line(m, b));
    hulliter y = z++;
    hulliter x = y;
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

  long long query(long long x) const {
    assert(!hull.empty());
    Line q(0, 0, x, true);
    hulliter it = hull.lower_bound(q);
    long long res = it->m * x + it->b;
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
  return 0;
}
