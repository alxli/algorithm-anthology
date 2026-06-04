/*

Given a convex polygon (a polygon such that every line crossing through it will only do so once) in
two dimensions, and two points specifying an infinite line, cut off the right part of the polygon,
and return the resulting left part.

- `convex_cut(lo, hi, p, q)` returns the points of the left side of a polygon, in clockwise order,
  after it has been cut by the line containing points $p$ and $q$. The original convex polygon is
  given by the range `[lo, hi)` of points in clockwise order, where `lo` and `hi` must be
  random-access iterators.

Time Complexity:
- O(n) per call to `convex_cut(lo, hi, p, q)`, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary for storage of the resulting convex cut.

*/

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double cross(const Point &a, const Point &b, const Point &o = Point(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

int turn(const Point &a, const Point &o, const Point &b) {
  double c = cross(a, b, o);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

int line_intersection(
    const Point &p1, const Point &p2, const Point &p3, const Point &p4, Point *p = nullptr
) {
  double a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  double c1 = -(p1.x * p2.y - p2.x * p1.y);
  double a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  double c2 = -(p3.x * p4.y - p4.x * p3.y);
  double x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  double det = a1 * b2 - a2 * b1;
  if (EQ(det, 0)) {
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  }
  if (p != nullptr) {
    *p = Point(x / det, y / det);
  }
  return 0;
}

template<class It>
std::vector<Point> convex_cut(It lo, It hi, const Point &p, const Point &q) {
  if (EQ(p.x, q.x) && EQ(p.y, q.y)) {
    throw std::runtime_error("Cannot cut using line from identical points.");
  }
  std::vector<Point> res;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    int d1 = turn(q, p, *j), d2 = turn(q, p, *i);
    if (d1 >= 0) {
      res.push_back(*j);
    }
    if (d1 * d2 < 0) {
      Point r;
      line_intersection(p, q, *j, *i, &r);
      res.push_back(r);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  {
    vector<Point> v{{1, 3}, {2, 2}, {2, 1}, {0, 0}, {-1, 3}};
    // Cut using the vertical line through (0, 0).
    vector<Point> c{{-1, 3}, {0, 3}, {0, 0}};
    assert(convex_cut(v.begin(), v.end(), Point(0, 0), Point(0, 1)) == c);
  }
  {  // On a non-convex input, the result may be multiple disjoint polygons!
    vector<Point> v{{0, 0}, {2, 2}, {0, 4}, {3, 4}, {3, 0}};
    vector<Point> c{{1, 0}, {0, 0}, {1, 1}, {1, 3}, {0, 4}, {1, 4}};
    assert(convex_cut(v.begin(), v.end(), Point(1, 0), Point(1, 4)) == c);
  }
  return 0;
}
