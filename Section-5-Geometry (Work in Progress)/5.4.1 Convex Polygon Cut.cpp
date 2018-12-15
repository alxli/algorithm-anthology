/*

Given a convex polygon (a polygon such that every line crossing through it will
only do so once) in two dimensions, and two points specifying an infinite line,
cut off the right part of the polygon, and return the resulting left part.

- convex_cut(lo, hi, p, q) returns the points of the left side of a polygon, in
  clockwise order, after it has been cut by the line containing points p and q.
  The original convex polygon is given by the range [lo, hi) of points in
  clockwise order, where lo and hi must be RandomAccessIterators.

Time Complexity:
- O(n) per call to convex_cut(lo, hi, p, q), where n is the distance between lo
  and hi.

Space Complexity:
- O(n) auxiliary for storage of the resulting convex cut.

*/

#include <cmath>
#include <cstdlib>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x)*(b.y - o.y) - (a.y - o.y)*(b.x - o.x);
}

int turn(const point &a, const point &o, const point &b) {
  double c = cross(a, b, o);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

int line_intersection(const point &p1, const point &p2,
                      const point &p3, const point &p4, point *p = NULL) {
  double a1 = p2.y - p1.y, b1 = p1.x - p2.x;
  double c1 = -(p1.x*p2.y - p2.x*p1.y);
  double a2 = p4.y - p3.y, b2 = p3.x - p4.x;
  double c2 = -(p3.x*p4.y - p4.x*p3.y);
  double x = -(c1*b2 - c2*b1), y = -(a1*c2 - a2*c1);
  double det = a1*b2 - a2*b1;
  if (EQ(det, 0)) {
    return (EQ(x, 0) && EQ(y, 0)) ? 1 : -1;
  }
  if (p != NULL) {
    *p = point(x / det, y / det);
  }
  return 0;
}

template<class It>
std::vector<point> convex_cut(It lo, It hi, const point &p, const point &q) {
  if (EQ(p.x, q.x) && EQ(p.y, q.y)) {
    throw std::runtime_error("Cannot cut using line from identical points.");
  }
  std::vector<point> res;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    int d1 = turn(q, p, *j), d2 = turn(q, p, *i);
    if (d1 >= 0) {
      res.push_back(*j);
    }
    if (d1*d2 < 0) {
      point r;
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
    vector<point> v;
    v.push_back(point(1, 3));
    v.push_back(point(2, 2));
    v.push_back(point(2, 1));
    v.push_back(point(0, 0));
    v.push_back(point(-1, 3));
    // Cut using the vertical line through (0, 0).
    vector<point> c;
    c.push_back(point(-1, 3));
    c.push_back(point(0, 3));
    c.push_back(point(0, 0));
    assert(convex_cut(v.begin(), v.end(), point(0, 0), point(0, 1)) == c);
  }
  { // On a non-convex input, the result may be multiple disjoint polygons!
    vector<point> v;
    v.push_back(point(0, 0));
    v.push_back(point(2, 2));
    v.push_back(point(0, 4));
    v.push_back(point(3, 4));
    v.push_back(point(3, 0));
    vector<point> c;
    c.push_back(point(1, 0));
    c.push_back(point(0, 0));
    c.push_back(point(1, 1));
    c.push_back(point(1, 3));
    c.push_back(point(0, 4));
    c.push_back(point(1, 4));
    assert(convex_cut(v.begin(), v.end(), point(1, 0), point(1, 4)) == c);
  }
  return 0;
}
