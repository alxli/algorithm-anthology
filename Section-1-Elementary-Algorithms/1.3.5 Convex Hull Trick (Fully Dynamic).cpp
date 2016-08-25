/*

Given a set of pairs (m, b) specifying lines of the form y = mx + b, process a
set of x-coordinate queries each asking to find the minimum y-value when any of
the given lines are evaluated at the specified x. To instead have the queries
optimize for maximum y-value, set the QUERY_MAX flag to true.

The following implementation is a fully dynamic variant of the convex hull
optimization technique, using a self-balancing binary search tree (std::set) to
support the ability to call add_line() and get_best() in any desired order.

Explanation: http://wcipeg.com/wiki/Convex_hull_trick#Fully_dynamic_variant

Time Complexity: O(n log n) on the total number of calls made to add_line(), for
any interlaced sequence of add_line() and get_min() calls. Each add_line() call
is O(log n) amortized time on the number of lines added so far. Each get_best()
call runs in O(log n) on the number of lines added so far.

Space Complexity: O(n) auxiliary on the number of calls made to add_line().

*/

#include <limits>  // std::numeric_limits
#include <set>

class hull_optimizer {
  struct line {
    long long m, b, val;
    double xlo;
    bool is_query;
    bool query_max;

    line(long long m, long long b, long long val,
         bool is_query, bool query_max) {
      this->m = m;
      this->b = b;
      this->val = val;
      this->xlo = -std::numeric_limits<double>::max();
      this->is_query = is_query;
      this->query_max = query_max;
    }

    bool parallel(const line &l) const {
      return m == l.m;
    }

    double intersect(const line &l) const {
      if (parallel(l))
        return std::numeric_limits<double>::max();
      return (double)(l.b - b)/(m - l.m);
    }

    bool operator<(const line &l) const {
      if (l.is_query)
        return query_max ? (xlo < l.val) : (l.val < xlo);
      return m < l.m;
    }
  };

  std::set<line> hull;
  bool _query_max;

  typedef std::set<line>::iterator hulliter;

  bool has_prev(hulliter it) const {
    return it != hull.begin();
  }

  bool has_next(hulliter it) const {
    return (it != hull.end()) && (++it != hull.end());
  }

  bool irrelevant(hulliter it) const {
    if (!has_prev(it) || !has_next(it))
      return false;
    hulliter prev = it, next = it;
    --prev;
    ++next;
    return _query_max ? prev->intersect(*next) <= prev->intersect(*it)
                     : next->intersect(*prev) <= next->intersect(*it);
  }

  hulliter update_left_border(hulliter it) {
    if ((_query_max && !has_prev(it)) || (!_query_max && !has_next(it)))
      return it;
    hulliter it2 = it;
    double val = it->intersect(_query_max ? *--it2 : *++it2);
    line l(*it);
    l.xlo = val;
    hull.erase(it++);
    return hull.insert(it, l);
  }

 public:
  hull_optimizer(bool query_max = false) {
    this->_query_max = query_max;
  }

  void add_line(long long m, long long b) {
    line l(m, b, 0, false, _query_max);
    hulliter it = hull.lower_bound(l);
    if (it != hull.end() && it->parallel(l)) {
      if ((_query_max && it->b < b) || (!_query_max && b < it->b))
        hull.erase(it++);
      else
        return;
    }
    it = hull.insert(it, l);
    if (irrelevant(it)) {
      hull.erase(it);
      return;
    }
    while (has_prev(it) && irrelevant(--it))
      hull.erase(it++);
    while (has_next(it) && irrelevant(++it))
      hull.erase(it--);
    it = update_left_border(it);
    if (has_prev(it))
      update_left_border(--it);
    if (has_next(++it))
      update_left_border(++it);
  }

  long long get_best(long long x) const {
    line q(0, 0, x, true, _query_max);
    hulliter it = hull.lower_bound(q);
    if (_query_max)
      --it;
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
  assert(h.get_best(0) == 0);
  assert(h.get_best(2) == 4);
  assert(h.get_best(1) == 3);
  assert(h.get_best(3) == 5);
  return 0;
}
