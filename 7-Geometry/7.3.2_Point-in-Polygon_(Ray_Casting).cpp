/*

Given a point `p` and a polygon, determines whether `p` lies inside using ray casting. A ray cast
from `p` must cross the polygon's boundary an odd number of times exactly when `p` is inside, so
the function counts crossings between a horizontal ray and each polygon edge. The function is
templated on the point type `Pt`. The local `Point` struct (`double` coordinates) is the default;
replace it with `Point`/`PointD`/ `PointI` from 7.1.1 or any struct with numeric `.x` and `.y`
fields. All comparisons are exact (no epsilon): the ray-crossing parity needs a consistent
comparison to be correct, and the result is exact for integer-coordinate points.

- `point_in_polygon(p, lo, hi)` returns whether `p` lies within the polygon with vertices specified
  by the range `[lo, hi)` of points in either clockwise or counter-clockwise order. If `p` lies
  exactly on an edge or vertex, the result will depend on the value of `EDGE_IS_INSIDE`.

Overflow warning: the edge orientation test forms a cross product that grows like the squared
coordinate magnitude. For integer point types use a 64-bit coordinate type (e.g. `PointL` from
7.1.1) once coordinates exceed ~46000.

Time Complexity:
- O(n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cstdint>

template<class Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o) {
  // Overflow risk for integer Pt: these products are ~O(max_coord^2); use int64_t if necessary.
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Detection is exact for integer-coordinate Pt.
template<class Pt, class It>
bool point_in_polygon(const Pt &p, It lo, It hi) {
  static const bool EDGE_IS_INSIDE = true;
  if (lo == hi) {
    return false;
  }
  bool res = false;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    if (i->y == p.y && (i->x == p.x || (j->y == p.y && (i->x <= p.x || j->x <= p.x)))) {
      return EDGE_IS_INSIDE;
    }
    if ((p.y < i->y) != (p.y < j->y)) {
      auto det = cross(*i, *j, p);
      if (det == 0) {
        return EDGE_IS_INSIDE;
      }
      if ((0 < det) != (i->y < j->y)) {
        res = !res;
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

int main() {
  vector<Point> poly{Point(-1, 3), Point(1, 3), Point(2, 1), Point(0, 0)};
  assert(point_in_polygon(Point(1, 2), poly.begin(), poly.end()));
  assert(point_in_polygon(Point(0, 3), poly.begin(), poly.end()));
  assert(!point_in_polygon(Point(0, 3.01), poly.begin(), poly.end()));
  assert(!point_in_polygon(Point(2, 2), poly.begin(), poly.end()));

  // Integer-coordinate points: exact detection.
  vector<PointI> ipoly{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
  assert(point_in_polygon(PointI(2, 2), ipoly.begin(), ipoly.end()));
  assert(!point_in_polygon(PointI(5, 2), ipoly.begin(), ipoly.end()));
  return 0;
}
