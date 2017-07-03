/*

Given a single point p and another range of points specifying a
polygon, determine whether p lies within the polygon. Note that
you should modify the EDGE_IS_INSIDE flag, depending on whether
you wish for the algorithm to consider points lying on an edge of
the polygon to be inside it.

Complexity: O(n) on the number of vertices in the polygon.

*/

#include <algorithm> /* std::sort() */
#include <cmath>     /* fabs() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */

//should we consider points lying on an edge to be inside the polygon?
const bool EDGE_IS_INSIDE = true;

//magnitude of the 3D cross product with Z component implicitly equal to 0
//the answer assumes the origin (0, 0) is instead shifted to point o.
//this is equal to 2x the signed area of the triangle from these 3 points.
double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

//return whether point p is in polygon specified by range [lo, hi) in O(n)
//[lo, hi) must point to the polygon vertices, sorted in CW or CCW order
template<class It> bool point_in_polygon(const point & p, It lo, It hi) {
  int cnt = 0;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    if (EQ(i->y, p.y) && (EQ(i->x, p.x) ||
                         (EQ(j->y, p.y) && (LE(i->x, p.x) || LE(j->x, p.x)))))
      return EDGE_IS_INSIDE; //on an edge
    if (GT(i->y, p.y) != GT(j->y, p.y)) {
      double det = cross(p, *i, *j);
      if (EQ(det, 0)) return EDGE_IS_INSIDE; //on an edge
      if (GT(det, 0) != GT(j->y, i->y)) cnt++;
    }
  }
  return cnt % 2 == 1;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;
#define pt point

int main() {
  //irregular trapezoid
  pt p[] = {pt(-1, 3), pt(1, 3), pt(2, 1), pt(0, 0)};
  assert(point_in_polygon(pt(1, 2), p, p + 4));
  assert(point_in_polygon(pt(0, 3), p, p + 4));
  assert(!point_in_polygon(pt(0, 3.01), p, p + 4));
  assert(!point_in_polygon(pt(2, 2), p, p + 4));
  return 0;
}
