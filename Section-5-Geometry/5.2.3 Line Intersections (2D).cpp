/*

Intersections between straight lines, as well as between line segments
in 2 dimensions. Also included are functions to determine the closest
point to a line, which is done by finding the intersection through the
perpendicular. Note that you should modify the TOUCH_IS_INTERSECT flag
used for line segment intersection, depending on whether you wish for
the algorithm to consider barely touching segments to intersect.

All operations are O(1) in time and space.

*/

#include <algorithm> /* std::min(), std::max() */
#include <cmath>     /* fabs(), sqrt() */
#include <utility>   /* std::pair */

typedef std::pair<double, double> point;
#define x first
#define y second

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps)  /* equal to */
#define LT(a, b) ((a) < (b) - eps)         /* less than */
#define LE(a, b) ((a) <= (b) + eps)        /* less than or equal to */

//intersection of line l1 and line l2, each in ax + by + c = 0 form
//returns: -1, if lines do not intersect,
//          0, if there is exactly one intersection point, or
//         +1, if there are infinite intersection
//in the 2nd case, the intersection point is optionally stored into p
int line_intersection(const double & a1, const double & b1, const double & c1,
                      const double & a2, const double & b2, const double & c2,
                      point * p = 0) {
  if (EQ(a1 * b2, a2 * b1))
    return (EQ(a1 * c2, a2 * c1) || EQ(b1 * c2, b2 * c1)) ? 1 : -1;
  if (p != 0) {
    p->x = (b1 * c1 - b1 * c2) / (a2 * b1 - a1 * b2);
    if (!EQ(b1, 0)) p->y = -(a1 * p->x + c1) / b1;
    else p->y = -(a2 * p->x + c2) / b2;
  }
  return 0;
}

//intersection of line through p1, p2, and line through p2, p3
//returns: -1, if lines do not intersect,
//          0, if there is exactly one intersection point, or
//         +1, if there are infinite intersections
//in the 2nd case, the intersection point is optionally stored into p
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

//Line Segment Intersection (http://stackoverflow.com/a/565282)

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double abs(const point & a) { return sqrt(norm(a)); }
double dot(const point & a, const point & b) { return a.x * b.x + a.y * b.y; }
double cross(const point & a, const point & b) { return a.x * b.y - a.y * b.x; }

//should we consider barely touching segments an intersection?
const bool TOUCH_IS_INTERSECT = true;

//does [l, h] contain m?
//precondition: l <= h
bool contain(const double & l, const double & m, const double & h) {
  if (TOUCH_IS_INTERSECT) return LE(l, m) && LE(m, h);
  return LT(l, m) && LT(m, h);
}

//does [l1, h1] overlap with [l2, h2]?
//precondition: l1 <= h1 and l2 <= h2
bool overlap(const double & l1, const double & h1,
             const double & l2, const double & h2) {
  if (TOUCH_IS_INTERSECT) return LE(l1, h2) && LE(l2, h1);
  return LT(l1, h2) && LT(l2, h1);
}

//intersection of line segment ab with line segment cd
//returns: -1, if segments do not intersect,
//          0, if there is exactly one intersection point
//         +1, if the intersection is another line segment
//In case 2, the intersection point is stored into p
//In case 3, the intersection segment is stored into p and q
int seg_intersection(const point & a, const point & b,
                     const point & c, const point & d,
                     point * p = 0, point * q = 0) {
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) { //collinear
    double t0 = dot(ac, ab) / norm(ab);
    double t1 = t0 + dot(cd, ab) / norm(ab);
    if (overlap(std::min(t0, t1), std::max(t0, t1), 0, 1)) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != 0) *p = res1;
        return 0; //collinear, meeting at an endpoint
      }
      if (p != 0 && q != 0) *p = res1, *q = res2;
      return 1; //collinear and overlapping
    } else {
      return -1; //collinear and disjoint
    }
  }
  if (EQ(c1, 0)) return -1; //parallel and disjoint
  double t = cross(ac, cd) / c1, u = c2 / c1;
  if (contain(0, t, 1) && contain(0, u, 1)) {
    if (p != 0) *p = point(a.x + t * ab.x, a.y + t * ab.y);
    return 0; //non-parallel with one intersection
  }
  return -1; //non-parallel with no intersections
}

//determines the point on line ax + by + c = 0 that is closest to point p
//this always lies on the line through p perpendicular to l.
point closest_point(const double & a, const double & b, const double & c,
                    const point & p) {
  if (EQ(a, 0)) return point(p.x, -c); //horizontal line
  if (EQ(b, 0)) return point(-c, p.y); //vertical line
  point res;
  line_intersection(a, b, c, -b, a, b * p.x - a * p.y, &res);
  return res;
}

//determines the point on segment ab closest to point p
point closest_point(const point & a, const point & b, const point & p) {
  if (a == b) return a;
  point ap(p.x - a.x, p.y - a.y), ab(b.x - a.x, b.y - a.y);
  double t = dot(ap, ab) / norm(ab);
  if (t <= 0) return a;
  if (t >= 1) return b;
  return point(a.x + t * ab.x, a.y + t * ab.y);
}

/*** Example Usage ***/

#include <cassert>
#define pt point

int main() {
  point p;
  assert(line_intersection(-1, 1, 0, 1, 1, -3, &p) == 0);
  assert(p == pt(1.5, 1.5));
  assert(line_intersection(pt(0, 0), pt(1, 1), pt(0, 4), pt(4, 0), &p) == 0);
  assert(p == pt(2, 2));

  //tests for segment intersection (examples in order from link below)
  //http://martin-thoma.com/how-to-check-if-two-line-segments-intersect/
  {
#define test(a,b,c,d,e,f,g,h) seg_intersection(pt(a,b),pt(c,d),pt(e,f),pt(g,h),&p,&q)
    pt p, q;
    //intersection is a point
    assert(0 == test(-4, 0, 4, 0, 0, -4, 0, 4));   assert(p == pt(0, 0));
    assert(0 == test(0, 0, 10, 10, 2, 2, 16, 4));  assert(p == pt(2, 2));
    assert(0 == test(-2, 2, -2, -2, -2, 0, 0, 0)); assert(p == pt(-2, 0));
    assert(0 == test(0, 4, 4, 4, 4, 0, 4, 8));     assert(p == pt(4, 4));

    //intersection is a segment
    assert(1 == test(10, 10, 0, 0, 2, 2, 6, 6));
    assert(p == pt(2, 2) && q == pt(6, 6));
    assert(1 == test(6, 8, 14, -2, 14, -2, 6, 8));
    assert(p == pt(6, 8) && q == pt(14, -2));

    //no intersection
    assert(-1 == test(6, 8, 8, 10, 12, 12, 4, 4));
    assert(-1 == test(-4, 2, -8, 8, 0, 0, -4, 6));
    assert(-1 == test(4, 4, 4, 6, 0, 2, 0, 0));
    assert(-1 == test(4, 4, 6, 4, 0, 2, 0, 0));
    assert(-1 == test(-2, -2, 4, 4, 10, 10, 6, 6));
    assert(-1 == test(0, 0, 2, 2, 4, 0, 1, 4));
    assert(-1 == test(2, 2, 2, 8, 4, 4, 6, 4));
    assert(-1 == test(4, 2, 4, 4, 0, 8, 10, 0));
  }
  assert(pt(2.5, 2.5) == closest_point(-1, -1, 5, pt(0, 0)));
  assert(pt(3, 0)     == closest_point(1, 0, -3, pt(0, 0)));
  assert(pt(0, 3)     == closest_point(0, 1, -3, pt(0, 0)));

  assert(pt(3, 0)  == closest_point(pt(3, 0), pt(3, 3), pt(0, 0)));
  assert(pt(2, -1) == closest_point(pt(2, -1), pt(4, -1), pt(0, 0)));
  assert(pt(4, -1) == closest_point(pt(2, -1), pt(4, -1), pt(5, 0)));
  return 0;
}
