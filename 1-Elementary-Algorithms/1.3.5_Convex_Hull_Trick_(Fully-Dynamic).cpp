/*

Given a set of pairs (m, b) specifying lines of the form y = mx + b, process a
set of x-coordinate queries each asking to find the minimum y-value when any of
the given lines are evaluated at the specified x. To instead have the queries
optimize for maximum y-value, call the constructor with query_max=true.

The following implementation is a fully dynamic variant of the convex hull
optimization technique, using a self-balancing binary search tree (std::set) to
support the ability to call add_line() and query() in any desired order.

Time Complexity:
- O(log n) amortized per call to add_line() and O(log n) per call to query(),
  where n is the number of lines added.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for add_line() and query().

*/

#include <cassert>
#include <climits>
#include <set>

class hull_optimizer {
  struct line {
    long long m, b;
    mutable long long xhi;
    bool is_query;

    line(long long m, long long b, long long xhi = 0, bool is_query = false)
        : m(m), b(b), xhi(xhi), is_query(is_query) {}

    bool operator<(const line &l) const {
      return l.is_query ? xhi < l.xhi : m < l.m;
    }
  };

  std::multiset<line> hull;
  bool query_max;

  typedef std::multiset<line>::iterator hulliter;

  static long long div_floor(long long a, long long b) {
    return a/b - ((a ^ b) < 0 && a % b);
  }

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
  hull_optimizer(bool query_max = false) : query_max(query_max) {}

  void add_line(long long m, long long b) {
    if (!query_max) {
      m = -m;
      b = -b;
    }
    hulliter z = hull.insert(line(m, b));
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
    line q(0, 0, x, true);
    hulliter it = hull.lower_bound(q);
    long long res = it->m*x + it->b;
    return query_max ? res : -res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  hull_optimizer h;
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
