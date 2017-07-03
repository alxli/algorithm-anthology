/*

Given a range of points specifying a polygon on the Cartesian
plane, as well as two points specifying an infinite line, "cut"
off the right part of the polygon with the line and return the
resulting polygon that is the left part.

Time Complexity: O(n) on the number of points in the poylgon.

*/

#include <cmath>   /* fabs() */
#include <utility> /* std::pair */
#include <vector>

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */

double cross(const point & o, const point & a, const point & b) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

int orientation(const point & o, const point & a, const point & b) {
  double c = cross(o, a, b);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

int line_intersection(const point & p1, const point & p2,
                      const point & p3, const point & p4, point * p = 0) {
  double a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  double c1 = -(p1.x * p2.y - p2.x * p1.y);
  double a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  double c2 = -(p3.x * p4.y - p4.x * p3.y);
  double x = -(c1 * b2 - c2 * b1), y = -(a1 * c2 - a2 * c1);
  double det = a1 * b2 - a2 * b1;
  if (EQ(det, 0))
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  if (p != 0) *p = point(x / det, y / det);
  return 0;
}

template<class It>
std::vector<point> convex_cut(It lo, It hi, const point & p, const point & q) {
  std::vector<point> res;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    int d1 = orientation(p, q, *j), d2 = orientation(p, q, *i);
    if (d1 >= 0) res.push_back(*j);
    if (d1 * d2 < 0) {
      point r;
      line_intersection(p, q, *j, *i, &r);
      res.push_back(r);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  //irregular pentagon with only (1, 2) not on the convex hull
  vector<point> v;
  v.push_back(point(1, 3));
  v.push_back(point(1, 2));
  v.push_back(point(2, 1));
  v.push_back(point(0, 0));
  v.push_back(point(-1, 3));
  //cut using the vertical line through (0, 0)
  vector<point> res = convex_cut(v.begin(), v.end(), point(0, 0), point(0, 1));
  cout << "left cut:\n";
  for (int i = 0; i < (int)res.size(); i++)
    cout << "(" << res[i].x << "," << res[i].y << ")\n";
  return 0;
}
