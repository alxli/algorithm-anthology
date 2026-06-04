/*

Given a point $p$ and a polygon in two dimensions, determine whether $p$ lies inside the polygon
using a ray casting algorithm.

- `point_in_polygon(p, lo, hi)` returns whether $p$ lies within the polygon defined by the range
  `[lo, hi)` of points specifying the vertices in either clockwise or counter-clockwise order, where
  `lo` and `hi` must be random-access iterators. If $p$ lies barely on an edge (within `EPS`), then
  the result will depend on the setting of `EDGE_IS_INSIDE`.

Time Complexity:
- O(n) per call to `point_in_polygon(p, lo, hi)`, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cmath>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LE(a, b) ((a) <= (b) + EPS)
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

template<class It>
bool point_in_polygon(const Point &p, It lo, It hi) {
  static const bool EDGE_IS_INSIDE = true;
  bool res = false;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    if (EQ(i->y, p.y) && (EQ(i->x, p.x) || (EQ(j->y, p.y) && (LE(i->x, p.x) || LE(j->x, p.x))))) {
      return EDGE_IS_INSIDE;
    }
    if (GT(i->y, p.y) != GT(j->y, p.y)) {
      double det = cross(*i, *j, p);
      if (EQ(det, 0)) {
        return EDGE_IS_INSIDE;
      }
      if (GT(det, 0) != GT(j->y, i->y)) {
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

int main() {
  // Irregular trapezoid.
  vector<Point> p{Point(-1, 3), Point(1, 3), Point(2, 1), Point(0, 0)};
  assert(point_in_polygon(Point(1, 2), p.begin(), p.end()));
  assert(point_in_polygon(Point(0, 3), p.begin(), p.end()));
  assert(!point_in_polygon(Point(0, 3.01), p.begin(), p.end()));
  assert(!point_in_polygon(Point(2, 2), p.begin(), p.end()));
  return 0;
}
