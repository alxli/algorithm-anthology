/*

Given a point p and a polygon in two dimensions, determine whether p lies inside
the polygon using a ray casting algorithm.

- point_in_polygon(p, lo, hi) returns whether p lies within the polygon defined
  by the range [lo, hi) of points specifying the vertices in either clockwise
  or counter-clockwise order, where lo and hi must be RandomAccessIterators. If
  p lies barely on an edge (within EPS), then the result will depend on the
  setting of EDGE_IS_INSIDE.

Time Complexity:
- O(n) per call to point_in_polygon(lo, hi), where n is the distance between lo
  and hi.

Space Complexity:
- O(1) auxiliary.

*/

#include <cmath>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GT(a, b) ((a) > (b) + EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x)*(b.y - o.y) - (a.y - o.y)*(b.x - o.x);
}

template<class It>
bool point_in_polygon(const point &p, It lo, It hi) {
  static const bool EDGE_IS_INSIDE = true;
  bool ans = 0;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    if (EQ(i->y, p.y) &&
        (EQ(i->x, p.x) ||
         (EQ(j->y, p.y) && (LE(i->x, p.x) || LE(j->x, p.x))))) {
      return EDGE_IS_INSIDE;
    }
    if (GT(i->y, p.y) != GT(j->y, p.y)) {
      double det = cross(*i, *j, p);
      if (EQ(det, 0)) {
        return EDGE_IS_INSIDE;
      }
      if (GT(det, 0) != GT(j->y, i->y)) {
        ans = !ans;
      }
    }
  }
  return ans;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Irregular trapezoid.
  point p[] = {point(-1, 3), point(1, 3), point(2, 1), point(0, 0)};
  assert(point_in_polygon(point(1, 2), p, p + 4));
  assert(point_in_polygon(point(0, 3), p, p + 4));
  assert(!point_in_polygon(point(0, 3.01), p, p + 4));
  assert(!point_in_polygon(point(2, 2), p, p + 4));
  return 0;
}
