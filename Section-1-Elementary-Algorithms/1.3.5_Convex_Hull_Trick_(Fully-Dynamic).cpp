/*

Given a set of pairs (m, b) specifying lines of the form y = mx + b, process a
set of x-coordinate queries each asking to find the minimum y-value when any of
the given lines are evaluated at the specified x. To instead have the queries
optimize for maximum y-value, call the constructor with query_max=true.

The following implementation is a fully dynamic variant of the convex hull
optimization technique, using a self-balancing binary search tree (std::set) to
support the ability to call add_line() and query() in any desired order.

Time Complexity:
- O(n) for any interlaced sequence of add_line() and query() calls, where n
  is the number of lines added. This is because the overall number of steps
  taken by add_line() and query() are respectively bounded by the number of
  lines. Thus a single call to either add_line() or query() will have an O(1)
  amortized running time.

Space Complexity:
- O(n) for storage of the lines.
- O(1) auxiliary for add_line() and query().

*/

#include <limits>
#include <set>

class hull_optimizer {
  struct line {
    long long m, b, value;
    double xlo;
    bool is_query, query_max;

    line(long long m, long long b, long long v, bool is_query, bool query_max)
        : m(m), b(b), value(v), xlo(-std::numeric_limits<double>::max()),
          is_query(is_query), query_max(query_max) {}

    double intersect(const line &l) const {
      if (m == l.m) {
        return std::numeric_limits<double>::max();
      }
      return (double)(l.b - b)/(m - l.m);
    }

    bool operator<(const line &l) const {
      if (l.is_query) {
        return query_max ? (xlo < l.value) : (l.value < xlo);
      }
      return m < l.m;
    }
  };

  std::set<line> hull;
  bool query_max;

  typedef std::set<line>::iterator hulliter;

  bool has_prev(hulliter it) const {
    return it != hull.begin();
  }

  bool has_next(hulliter it) const {
    return (it != hull.end()) && (++it != hull.end());
  }

  bool irrelevant(hulliter it) const {
    if (!has_prev(it) || !has_next(it)) {
      return false;
    }
    hulliter prev = it, next = it;
    --prev;
    ++next;
    return query_max ? (prev->intersect(*next) <= prev->intersect(*it))
                     : (next->intersect(*prev) <= next->intersect(*it));
  }

  hulliter update_left_border(hulliter it) {
    if ((query_max && !has_prev(it)) || (!query_max && !has_next(it))) {
      return it;
    }
    hulliter it2 = it;
    double value = it->intersect(query_max ? *--it2 : *++it2);
    line l(*it);
    l.xlo = value;
    hull.erase(it++);
    return hull.insert(it, l);
  }

 public:
  hull_optimizer(bool query_max = false) : query_max(query_max) {}

  void add_line(long long m, long long b) {
    line l(m, b, 0, false, query_max);
    hulliter it = hull.lower_bound(l);
    if (it != hull.end() && it->m == l.m) {
      if ((query_max && it->b < b) || (!query_max && b < it->b)) {
        hull.erase(it++);
      } else {
        return;
      }
    }
    it = hull.insert(it, l);
    if (irrelevant(it)) {
      hull.erase(it);
      return;
    }
    while (has_prev(it) && irrelevant(--it)) {
      hull.erase(it++);
    }
    while (has_next(it) && irrelevant(++it)) {
      hull.erase(it--);
    }
    it = update_left_border(it);
    if (has_prev(it)) {
      update_left_border(--it);
    }
    if (has_next(++it)) {
      update_left_border(++it);
    }
  }

  long long query(long long x) const {
    line q(0, 0, x, true, query_max);
    hulliter it = hull.lower_bound(q);
    if (query_max) {
      --it;
    }
    return it->m*x + it->b;
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
