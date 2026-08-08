/*

Maintains a dynamic set of lines $y = mx + b$ and answers minimum value queries at integer points.
Lines and queries may arrive in arbitrary order, making this useful for dynamic programming
recurrences of the form `dp[i] = min(m[j] * x[i] + b[j])` without monotone slopes or query
coordinates. The same interface can answer maximum queries when constructed with `query_max = true`.

This fully dynamic convex hull stores the envelope in a self-balancing binary search tree
(`std::set`). Inserting a line removes any neighbors it dominates, and each remaining line records
the last coordinate where it is optimal, so a query is one tree lookup. Unlike a Li Chao tree, this
structure requires no fixed coordinate domain and its complexity depends on the number of lines
rather than the domain width. Prefer it when the domain is unknown or very large, or when maximum
queries are needed; a Li Chao tree is often simpler when a manageable domain is known.

- `FullyDynamicCHT(query_max = false)` constructs an empty hull. By default, `query(x)` minimizes;
  if `query_max` is true, `query(x)` maximizes.
- `add_line(m, b)` inserts line $y = mx + b$. Lines may be added in any order.
- `query(x)` returns the best $y$-value among all inserted lines at coordinate `x`. At least one
  line must have been inserted, and query coordinates may be supplied in any order.

Overflow warning: minimization negates the coefficients, border updates subtract slopes/intercepts,
and `query()` evaluates `m * x + b`, so those intermediate values must fit in `int64_t`.

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

class FullyDynamicCHT {
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

  using HullIter = std::multiset<Line>::iterator;

  bool update_border(HullIter x, HullIter y) {
    if (y == hull.end()) {
      x->xhi = INT64_MAX;
      return false;
    }
    if (x->m == y->m) {
      x->xhi = (x->b > y->b) ? INT64_MAX : INT64_MIN;
    } else {
      int64_t a = y->b - x->b, b = x->m - y->m;  // Overflow warning.
      x->xhi = a / b - ((a ^ b) < 0 && a % b);
    }
    return x->xhi >= y->xhi;
  }

 public:
  explicit FullyDynamicCHT(bool query_max = false) : query_max(query_max) {}

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
    int64_t res = it->m * x + it->b;  // Overflow warning.
    return query_max ? res : -res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  FullyDynamicCHT h;
  h.add_line(3, 0);
  h.add_line(0, 6);
  h.add_line(1, 2);
  h.add_line(2, 1);
  // Minimize among y = 3x, 6, x + 2, and 2x + 1.
  assert(h.query(0) == 0);
  assert(h.query(2) == 4);
  assert(h.query(1) == 3);
  assert(h.query(3) == 5);

  FullyDynamicCHT mx(true);
  mx.add_line(3, 0);
  mx.add_line(0, 6);
  mx.add_line(1, 2);
  // Same interface can maximize when constructed with query_max = true.
  assert(mx.query(0) == 6);
  assert(mx.query(3) == 9);
  return 0;
}
