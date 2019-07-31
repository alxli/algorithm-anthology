/*

Intersection and closest point calculations in two dimensions for straight lines
and line segments.

- line_intersection(a1, b1, c1, a2, b2, c2, &p) determines whether the lines
  a1*x + b1*y + c1 = 0 and a2*x + b2*x + c2 = 0 intersects, returning -1 if
  there is no intersection because the lines are parallel, 0 if there is exactly
  one intersection (in which case the intersection point is stored into pointer
  p if it's not NULL), or 1 if there are infinite intersections because the
  lines are identical.
- line_intersection(p1, p2, p3, p4, &p) determines whether the infinite lines
  (not segments) through points p1, p2 and through points p3 and p4 intersect,
  returning -1 if there is no intersection because the lines are parallel, 0 if
  there is exactly one intersection (in which case the intersection point is
  stored into pointer p if it's not NULL), or 1 if there are infinite
  intersections because the lines are identical.
- seg_intersection(a, b, c, d, &p, &q) determines whether the line segment ab
  intersects the line segment cd, returning -1 if the segments do not intersect,
  0 if there is exactly one intersection point (in which case it is stored into
  pointer p if it's not NULL), or 1 if the intersection is another line segment
  (in which case the two endpoints are stored into pointers p and q if they are
  not NULL). If the segments are barely touching (close within EPS), then the
  result will depend on the setting of TOUCH_IS_INTERSECT.
- closest_point(a, b, c, p) returns the point on line a*x + b*y + c = 0 that is
  closest to point p. Note that the result always lies on the line through p
  which is perpendicular to the line a*x + b*y + c = 0.
- closest_point(a, b, p) returns the point on segment ab closest to point p.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }
double dot(const point &a, const point &b) { return a.x*b.x + a.y*b.y; }
double cross(const point &a, const point &b) { return a.x*b.y - a.y*b.x; }

int line_intersection(double a1, double b1, double c1, double a2, double b2,
                      double c2, point *p = NULL) {
  if (EQ(a1, a2) && EQ(b1, b2)) {
    return EQ(c1, c2) ? 1 : -1;
  }
  if (p != NULL) {
    p->x = (b1*c1 - b1*c2) / (a2*b1 - a1*b2);
    if (!EQ(b1, 0)) {
      p->y = -(a1*p->x + c1) / b1;
    } else {
      p->y = -(a2*p->x + c2) / b2;
    }
  }
  return 0;
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

int seg_intersection(const point &a, const point &b, const point &c,
                     const point &d, point *p = NULL, point *q = NULL) {
  static const bool TOUCH_IS_INTERSECT = true;
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    double t0 = dot(ac, ab) / sqnorm(ab);
    double t1 = t0 + dot(cd, ab) / sqnorm(ab);
    double mint = std::min(t0, t1), maxt = std::max(t0, t1);
    bool overlap = TOUCH_IS_INTERSECT ? (LE(mint, 1) && LE(0, maxt))
                                      : (LT(mint, 1) && LT(0, maxt));
    if (overlap) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != NULL) {
          *p = res1;
        }
        return 0;  // Collinear and meeting at an endpoint.
      }
      if (p != NULL && q != NULL) {
        *p = res1;
        *q = res2;
      }
      return 1;  // Collinear and overlapping.
    } else {
      return -1;  // Collinear and disjoint.
    }
  }
  if (EQ(c1, 0)) {
    return -1;  // Parallel and disjoint.
  }
  double t = cross(ac, cd)/c1, u = c2/c1;
  bool t_between_01 = TOUCH_IS_INTERSECT ? (LE(0, t) && LE(t, 1))
                                         : (LT(0, t) && LT(t, 1));
  bool u_between_01 = TOUCH_IS_INTERSECT ? (LE(0, u) && LE(u, 1))
                                         : (LT(0, u) && LT(u, 1));
  if (t_between_01 && u_between_01) {
    if (p != NULL) {
      *p = point(a.x + t*ab.x, a.y + t*ab.y);
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

point closest_point(double a, double b, double c, const point &p) {
  if (EQ(a, 0)) {
    return point(p.x, -c);  // Horizontal line.
  }
  if (EQ(b, 0)) {
    return point(-c, p.y);  // Vertical line.
  }
  point res;
  line_intersection(a, b, c, -b, a, b*p.x - a*p.y, &res);
  return res;
}

point closest_point(const point &a, const point &b, const point &p) {
  if (a == b) return a;
  point ap(p.x - a.x, p.y - a.y), ab(b.x - a.x, b.y - a.y);
  double t = dot(ap, ab) / norm(ab);
  if (t <= 0) return a;
  if (t >= 1) return b;
  return point(a.x + t * ab.x, a.y + t * ab.y);
}

/*** Example Usage ***/

#include <cassert>
#define point point

bool EQP(const point &a, const point &b) {
  return EQ(a.x, b.x) && EQ(a.y, b.y);
}

int main() {
  point p, q;

  assert(line_intersection(-1, 1, 0, 1, 1, -3, &p) == 0);
  assert(EQP(p, point(1.5, 1.5)));
  assert(line_intersection(point(0, 0), point(1, 1), point(0, 4), point(4, 0),
                           &p) == 0);
  assert(EQP(p, point(2, 2)));

  {
    #define test(a, b, c, d, e, f, g, h) seg_intersection( \
        point(a, b), point(c, d), point(e, f), point(g, h), &p, &q)

    // Intersection is a point.
    assert(0 == test(-4, 0, 4, 0, 0, -4, 0, 4) && EQP(p, point(0, 0)));
    assert(0 == test(0, 0, 10, 10, 2, 2, 16, 4) && EQP(p, point(2, 2)));
    assert(0 == test(-2, 2, -2, -2, -2, 0, 0, 0) && EQP(p, point(-2, 0)));
    assert(0 == test(0, 4, 4, 4, 4, 0, 4, 8) && EQP(p, point(4, 4)));

    // Intersection is a segment.
    assert(1 == test(10, 10, 0, 0, 2, 2, 6, 6));
    assert(EQP(p, point(2, 2)) && EQP(q, point(6, 6)));
    assert(1 == test(6, 8, 14, -2, 14, -2, 6, 8));
    assert(EQP(p, point(6, 8)) && EQP(q, point(14, -2)));

    // No intersection.
    assert(-1 == test(6, 8, 8, 10, 12, 12, 4, 4));
    assert(-1 == test(-4, 2, -8, 8, 0, 0, -4, 6));
    assert(-1 == test(4, 4, 4, 6, 0, 2, 0, 0));
    assert(-1 == test(4, 4, 6, 4, 0, 2, 0, 0));
    assert(-1 == test(-2, -2, 4, 4, 10, 10, 6, 6));
    assert(-1 == test(0, 0, 2, 2, 4, 0, 1, 4));
    assert(-1 == test(2, 2, 2, 8, 4, 4, 6, 4));
    assert(-1 == test(4, 2, 4, 4, 0, 8, 10, 0));
  }

  assert(EQP(point(2.5, 2.5), closest_point(-1, -1, 5, point(0, 0))));
  assert(EQP(point(3, 0), closest_point(1, 0, -3, point(0, 0))));
  assert(EQP(point(0, 3), closest_point(0, 1, -3, point(0, 0))));

  assert(EQP(point(3, 0),
             closest_point(point(3, 0), point(3, 3), point(0, 0))));
  assert(EQP(point(2, -1),
             closest_point(point(2, -1), point(4, -1), point(0, 0))));
  assert(EQP(point(4, -1),
             closest_point(point(2, -1), point(4, -1), point(5, 0))));
  return 0;
}
