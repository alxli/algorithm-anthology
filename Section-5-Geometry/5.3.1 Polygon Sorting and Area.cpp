/*

centroid() - Simply returns the geometric average point of all the
points given. This could be used to find the reference center point
for the following function. An empty range will result in (0, 0).
Complexity: O(n) on the number of points in the given range.

cw_comp() - Given a set of points, these points could possibly form
many different polygons. The following sorting comparators, when
used in conjunction with std::sort, will produce one such ordering
of points which is sorted in clockwise order relative to a custom-
defined center point that must be set beforehand. This could very
well be the result of mean_point(). ccw_comp() is the opposite
function, which produces the points in counterclockwise order.
Complexity: O(1) per call.

polygon_area() - A given range of points is interpreted as a polygon
based on the ordering they're given in. The shoelace formula is used
to determine its area. The polygon does not necessarily have to be
sorted using one of the functions above, but may be any ordering that
produces a valid polygon. You may optionally pass the last point in
the range equal to the first point and still expect the correct result.
Complexity: O(n) on the number of points in the range, assuming that
the points are already sorted in the order that specifies the polygon.

*/

#include <algorithm> /* std::sort() */
#include <cmath>     /* fabs() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define GE(a, b) ((a) >= (b) - eps)       /* greater than or equal to */

//magnitude of the 3D cross product with Z component implicitly equal to 0
//the answer assumes the origin (0, 0) is instead shifted to point o.
//this is equal to 2x the signed area of the triangle from these 3 points.
double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

point ctr;

template<class It> point centroid(It lo, It hi) {
  if (lo == hi) return point(0, 0);
  double xtot = 0, ytot = 0, points = hi - lo;
  for (; lo != hi; ++lo) {
    xtot += lo->x;
    ytot += lo->y;
  }
  return point(xtot / points, ytot / points);
}

//ctr must be defined beforehand
bool cw_comp(const point & a, const point & b) {
  if (GE(a.x - ctr.x, 0) && LT(b.x - ctr.x, 0)) return true;
  if (LT(a.x - ctr.x, 0) && GE(b.x - ctr.x, 0)) return false;
  if (EQ(a.x - ctr.x, 0) && EQ(b.x - ctr.x, 0)) {
    if (GE(a.y - ctr.y, 0) || GE(b.y - ctr.y, 0))
      return a.y > b.y;
    return b.y > a.y;
  }
  double det = cross(ctr, a, b);
  if (EQ(det, 0))
    return (a.x - ctr.x) * (a.x - ctr.x) + (a.y - ctr.y) * (a.y - ctr.y) >
           (b.x - ctr.x) * (b.x - ctr.x) + (b.y - ctr.y) * (b.y - ctr.y);
  return det < 0;
}

bool ccw_comp(const point & a, const point & b) {
  return cw_comp(b, a);
}

//area of a polygon specified by range [lo, hi) - shoelace formula in O(n)
//[lo, hi) must point to the polygon vertices, sorted in CW or CCW order
template<class It> double polygon_area(It lo, It hi) {
  if (lo == hi) return 0;
  double area = 0;
  if (*lo != *--hi)
    area += (lo->x - hi->x) * (lo->y + hi->y);
  for (It i = hi, j = hi - 1; i != lo; --i, --j)
    area += (i->x - j->x) * (i->y + j->y);
  return fabs(area / 2.0);
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;
#define pt point

int main() {
  //irregular pentagon with only (1, 2) not on the convex hull
  //the ordering here is already sorted in ccw order around their centroid
  //we will scramble them and see if our comparator works
  pt pts[] = {pt(1, 3), pt(1, 2), pt(2, 1), pt(0, 0), pt(-1, 3)};
  vector<pt> v(pts, pts + 5);
  std::random_shuffle(v.begin(), v.end());
  ctr = centroid(v.begin(), v.end()); //note: ctr is a global variable
  assert(EQ(ctr.x, 0.6) && EQ(ctr.y, 1.8));
  sort(v.begin(), v.end(), cw_comp);
  for (int i = 0; i < (int)v.size(); i++) assert(v[i] == pts[i]);
  assert(EQ(polygon_area(v.begin(), v.end()), 5));
  return 0;
}
