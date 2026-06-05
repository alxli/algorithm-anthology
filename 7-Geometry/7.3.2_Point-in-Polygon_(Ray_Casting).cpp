/*

Given a point $p$ and a polygon, determines whether $p$ lies inside using ray casting.

The function is templated on the point type `Pt`. The local `Point` struct (double coordinates) is
the default; replace it with `PointD`/ `PointI` from 7.1.1 or any struct with numeric `.x` and `.y`
fields. With integer-coordinate points the cross-product comparisons are exact.

- `point_in_polygon(p, lo, hi)` returns whether $p$ lies within the polygon given by range
  `[lo, hi)` in clockwise or counter-clockwise order. If $p$ lies on an edge (within `EPS`), the
  result depends on `EDGE_IS_INSIDE`.

Time Complexity:
- O(n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cmath>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

template<class Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Detection is exact for integer-coordinate Pt.
template<class Pt, class It>
bool point_in_polygon(const Pt &p, It lo, It hi) {
  static const bool EDGE_IS_INSIDE = true;
  bool res = false;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    if (EQ(i->y, p.y) && (EQ(i->x, p.x) || (EQ(j->y, p.y) && (LE(i->x, p.x) || LE(j->x, p.x))))) {
      return EDGE_IS_INSIDE;
    }
    if (LT(p.y, i->y) != LT(p.y, j->y)) {
      auto det = cross(*i, *j, p);
      if (EQ(det, 0)) {
        return EDGE_IS_INSIDE;
      }
      if (LT(0, det) != LT(i->y, j->y)) {
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
};

struct PointI {
  int x, y;
};

int main() {
  vector<Point> poly{Point{-1, 3}, Point{1, 3}, Point{2, 1}, Point{0, 0}};
  assert(point_in_polygon(Point{1, 2}, poly.begin(), poly.end()));
  assert(point_in_polygon(Point{0, 3}, poly.begin(), poly.end()));
  assert(!point_in_polygon(Point{0, 3.01}, poly.begin(), poly.end()));
  assert(!point_in_polygon(Point{2, 2}, poly.begin(), poly.end()));

  // Integer-coordinate points: exact detection.
  vector<PointI> ipoly{{0, 0}, {4, 0}, {4, 4}, {0, 4}};
  assert(point_in_polygon(PointI{2, 2}, ipoly.begin(), ipoly.end()));
  assert(!point_in_polygon(PointI{5, 2}, ipoly.begin(), ipoly.end()));

  return 0;
}
