/*

1.3.5 - Convex Hull Trick (Fully Dynamic)

Given a set of pairs (m, b) describing lines of the form y = mx + b,
process a set of x-coordinate queries each asking to find the minimum
y-value of any of the given lines when evaluated at the specified x.
The convex hull optimization technique first ignores all lines which
never take on the maximum at any x value, then sorts the rest in order
of descending slope. The intersection points of adjacent lines in this
sorted list form the upper envelope of a convex hull, and line segments
connecting these points always take on the minimum y-value. The result
can be split up into x-intervals each mapped to the line which takes on
the minimum in that interval. The intervals can be binary search to
solve each query in O(log n) time on the number of lines.

Explanation: http://wcipeg.com/wiki/Convex_hull_trick

The following implementation is a fully dynamic version, using a
self-balancing binary search tree (std::set) to support calling line
addition and query operations in any desired order. In addition, one
may instead optimize for maximum y by setting QUERY_MAX to true.

Time Complexity: O(n log n) for n calls to add_line(), where each call
is O(log n) amortized on the number of lines added so far. Each call to
get_best() runs in O(log n) on the number of lines added so far.

Space Complexity: O(n) auxiliary on the number of calls to add_line().

*/

#include <set>

const bool QUERY_MAX = false;
const double INF = 1e30;

struct line {
  long long m, b, val;
  double xlo;
  bool is_query;

  line(long long m, long long b) {
    this->m = m;
    this->b = b;
    val = 0;
    xlo = -INF;
    is_query = false;
  }

  long long evaluate(long long x) const {
    return m * x + b;
  }

  bool parallel(const line & l) const {
    return m == l.m;
  }

  double intersect(const line & l) const {
    if (parallel(l))
      return INF;
    return (double)(l.b - b)/(m - l.m);
  }

  bool operator < (const line & l) const {
    if (l.is_query)
      return QUERY_MAX ? xlo < l.val : l.val < xlo;
    return m < l.m;
  }
};

std::set<line> hull;

typedef std::set<line>::iterator hulliter;

bool has_prev(hulliter it) {
  return it != hull.begin();
}

bool has_next(hulliter it) {
  return it != hull.end() && ++it != hull.end();
}

bool irrelevant(hulliter it) {
  if (!has_prev(it) || !has_next(it))
    return false;
  hulliter prev = it; --prev;
  hulliter next = it; ++next;
  return QUERY_MAX ?
          prev->intersect(*next) <= prev->intersect(*it) :
          next->intersect(*prev) <= next->intersect(*it);
}

hulliter update_left_border(hulliter it) {
  if ((QUERY_MAX && !has_prev(it)) || (!QUERY_MAX && !has_next(it)))
    return it;
  hulliter it2 = it;
  double val = it->intersect(QUERY_MAX ? *--it2 : *++it2);
  line buf(*it);
  buf.xlo = val;
  hull.erase(it++);
  return hull.insert(it, buf);
}

void add_line(long long m, long long b) {
  line l(m, b);
  hulliter it = hull.lower_bound(l);
  if (it != hull.end() && it->parallel(l)) {
    if ((QUERY_MAX && it->b < b) || (!QUERY_MAX && b < it->b))
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

long long get_best(long long x) {
  line q(0, 0);
  q.val = x;
  q.is_query = true;
  hulliter it = hull.lower_bound(q);
  if (QUERY_MAX)
    --it;
  return it->evaluate(x);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  add_line(3, 0);
  add_line(0, 6);
  add_line(1, 2);
  add_line(2, 1);
  assert(get_best(0) == 0);
  assert(get_best(1) == 3);
  assert(get_best(2) == 4);
  assert(get_best(3) == 5);
  return 0;
}
