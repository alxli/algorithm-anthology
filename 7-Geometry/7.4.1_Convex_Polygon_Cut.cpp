/*

Given a convex polygon and a directed line from `p` to `q`, clips the polygon to the closed left
half-plane of that line. The polygon is walked edge by edge: vertices on the left side of the line
are kept, and whenever an edge crosses the line, the intersection point is appended to the output.

- `convex_cut(lo, hi, p, q)` returns the portion of the polygon lying on or to the left of the
  directed line `p` $\to$ `q`. The input range [`lo`, `hi`) must contain the vertices of a convex
  polygon in boundary order, either clockwise or counterclockwise. The returned polygon preserves
  that boundary order. If `p` equals `q`, the cutting line is invalid and an error is thrown.

The function is templated on the input point type. Side classification is done with cross products.
For integer-coordinate inputs, classification is exact only if the intermediate products do not
overflow. Edge-line intersection points are computed in floating point, and the returned polygon
uses `Point` with `double` coordinates.

Time Complexity:
- O(n) per call to `convex_cut(lo, hi, p, q)`, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary space for the returned polygon.

*/

#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
};

template<class PtA, class PtB, class PtO>
double cross(const PtA &a, const PtB &b, const PtO &o) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

template<class PtA, class PtO, class PtB>
int turn(const PtA &a, const PtO &o, const PtB &b) {
  double c = cross(a, b, o);
  return LT(c, 0) ? -1 : (LT(0, c) ? 1 : 0);
}

template<class PtA, class PtB>
int line_intersection(
    const PtA &p1, const PtA &p2, const PtB &p3, const PtB &p4, Point *p = nullptr
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

template<class It, class Pt>
std::vector<Point> convex_cut(It lo, It hi, const Pt &p, const Pt &q) {
  if (EQ(p.x, q.x) && EQ(p.y, q.y)) {
    throw std::runtime_error("Cannot cut using line from identical points.");
  }
  if (lo == hi) {
    return {};
  }
  std::vector<Point> res;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    Point pj(static_cast<double>(j->x), static_cast<double>(j->y));
    Point pi(static_cast<double>(i->x), static_cast<double>(i->y));
    int d1 = turn(q, p, pj), d2 = turn(q, p, pi);
    if (d1 >= 0) {
      res.push_back(pj);
    }
    if (d1 * d2 < 0) {
      Point r;
      line_intersection(p, q, pj, pi, &r);
      res.push_back(r);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

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
  {
    vector<PointI> v{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
    vector<Point> c{{0, 4}, {0, 0}, {2, 0}, {2, 4}};
    assert(convex_cut(v.begin(), v.end(), PointI(2, 0), PointI(2, 4)) == c);
  }
  return 0;
}
