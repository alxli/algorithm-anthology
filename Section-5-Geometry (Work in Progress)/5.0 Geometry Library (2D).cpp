/*

This file contains all the *constant time* 2D geometric calculations in
this entire section. In other words, all of the mathematical manipulations
here will run in O(1) time. Computationally heavy algorithms such as the
closest pair problem and convex hull are omitted. All of these topics,
constant time or not, may be found in their independent files. You may
refer to each independent section for more documentation on their usage.
This file is merely a convenient amalgamation of the geometric definitions
to simplify cross-dependencies.

Functions in this file will try to take advantage of previously defined
classes, unlike in the split-up sections, which are more portable in
their implementations. Elsewhere, cross() may be implemented with the
formula expressed in terms of x and y, instead of point arithmetic and
the more generic cross() method of the point class. All of the operations
below apply to a 2D Cartesian coordinate system where the positive x
direction points to the "right" and the positive y direction points "up".

*/

#include <algorithm> /* std::swap() */
#include <cmath>     /* acos, atan2, cos, fabs, sin, sqrt */
#include <limits>    /* std::numeric_limits */
#include <ostream>
#include <stdexcept> /* std::runtime_error() */
#include <utility>   /* std::pair */
#include <vector>

const double posinf = std::numeric_limits<double>::max();
const double neginf = -std::numeric_limits<double>::max();
const double NaN = std::numeric_limits<double>::quiet_NaN();

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define NE(a, b) (fabs((a) - (b)) > eps)  /* not equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)       /* greater than or equal to */

//2D Points Class - like std::complex, but with epsilon comparisons
struct point {

  double x, y;

  point() : x(0), y(0) {}
  point(const point & p) : x(p.x), y(p.y) {}
  point(const std::pair<double, double> & p) : x(p.first), y(p.second) {}
  point(const double & a, const double & b) : x(a), y(b) {}

  bool operator < (const point & p) const {
    return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
  }

  bool operator > (const point & p) const {
    return EQ(x, p.x) ? LT(p.y, y) : LT(p.x, x);
  }

  bool operator == (const point & p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator != (const point & p) const { return !(*this == p); }
  bool operator <= (const point & p) const { return !(*this > p); }
  bool operator >= (const point & p) const { return !(*this < p); }
  point operator + (const point & p) const { return point(x + p.x, y + p.y); }
  point operator - (const point & p) const { return point(x - p.x, y - p.y); }
  point operator + (const double & v) const { return point(x + v, y + v); }
  point operator - (const double & v) const { return point(x - v, y - v); }
  point operator * (const double & v) const { return point(x * v, y * v); }
  point operator / (const double & v) const { return point(x / v, y / v); }
  point & operator += (const point & p) { x += p.x; y += p.y; return *this; }
  point & operator -= (const point & p) { x -= p.x; y -= p.y; return *this; }
  point & operator += (const double & v) { x += v; y += v; return *this; }
  point & operator -= (const double & v) { x -= v; y -= v; return *this; }
  point & operator *= (const double & v) { x *= v; y *= v; return *this; }
  point & operator /= (const double & v) { x /= v; y /= v; return *this; }
  friend point operator + (const double & v, const point & p) { return p + v; }
  friend point operator * (const double & v, const point & p) { return p * v; }

  double norm() const { return x * x + y * y; }
  double abs() const { return sqrt(x * x + y * y); }
  double arg() const { return atan2(y, x); }
  double dot(const point & p) const { return x * p.x + y * p.y; }
  double cross(const point & p) const { return x * p.y - y * p.x; }
  double proj(const point & p) const { return dot(p) / p.abs(); } //onto p
  point rot90() const { return point(-y, x); }

  //proportional unit vector of (x, y) such that x^2 + y^2 = 1
  point normalize() const {
    return (EQ(x, 0) && EQ(y, 0)) ? point(0, 0) : (point(x, y) / abs());
  }

  //rotate t radians CW about origin
  point rotateCW(const double & t) const {
    return point(x * cos(t) + y * sin(t), y * cos(t) - x * sin(t));
  }

  //rotate t radians CCW about origin
  point rotateCCW(const double & t) const {
    return point(x * cos(t) - y * sin(t), x * sin(t) + y * cos(t));
  }

  //rotate t radians CW about point p
  point rotateCW(const point & p, const double & t) const {
    return (*this - p).rotateCW(t) + p;
  }

  //rotate t radians CCW about point p
  point rotateCCW(const point & p, const double & t) const {
    return (*this - p).rotateCCW(t) + p;
  }

  //reflect across point p
  point reflect(const point & p) const {
    return point(2 * p.x - x, 2 * p.y - y);
  }

  //reflect across the line containing points p and q
  point reflect(const point & p, const point & q) const {
    if (p == q) return reflect(p);
    point r(*this - p), s = q - p;
    r = point(r.x * s.x + r.y * s.y, r.x * s.y - r.y * s.x) / s.norm();
    r = point(r.x * s.x - r.y * s.y, r.x * s.y + r.y * s.x) + p;
    return r;
  }

  friend double norm(const point & p) { return p.norm(); }
  friend double abs(const point & p) { return p.abs(); }
  friend double arg(const point & p) { return p.arg(); }
  friend double dot(const point & p, const point & q) { return p.dot(q); }
  friend double cross(const point & p, const point & q) { return p.cross(q); }
  friend double proj(const point & p, const point & q) { return p.proj(q); }
  friend point rot90(const point & p) { return p.rot90(); }
  friend point normalize(const point & p) { return p.normalize(); }
  friend point rotateCW(const point & p, const double & t) { return p.rotateCW(t); }
  friend point rotateCCW(const point & p, const double & t) { return p.rotateCCW(t); }
  friend point rotateCW(const point & p, const point & q, const double & t) { return p.rotateCW(q, t); }
  friend point rotateCCW(const point & p, const point & q, const double & t) { return p.rotateCCW(q, t); }
  friend point reflect(const point & p, const point & q) { return p.reflect(q); }
  friend point reflect(const point & p, const point & a, const point & b) { return p.reflect(a, b); }

  friend std::ostream & operator << (std::ostream & out, const point & p) {
    out << "(";
    out << (fabs(p.x) < eps ? 0 : p.x) << ",";
    out << (fabs(p.y) < eps ? 0 : p.y) << ")";
    return out;
  }
};

//2D line class in the form ax + by + c = 0, where lines are normalized
//so that b is always either 1 (normal lines) or 0 (vertical lines).
struct line {

  double a, b, c;

  line(): a(0), b(0), c(0) {} //invalid or uninitialized line

  line(const double & A, const double & B, const double & C) {
    a = A;
    b = B;
    c = C;
    if (!EQ(b, 0)) {
      a /= b; c /= b; b = 1;
    } else {
      c /= a; a = 1; b = 0;
    }
  }

  line(const double & slope, const point & p) {
    a = -slope;
    b = 1;
    c = slope * p.x - p.y;
  }

  line(const point & p, const point & q): a(0), b(0), c(0) {
    if (EQ(p.x, q.x)) {
      if (EQ(p.y, q.y)) return; //invalid line
      //vertical line
      a = 1;
      b = 0;
      c = -p.x;
      return;
    }
    a = -(p.y - q.y) / (p.x - q.x);
    b = 1;
    c = -(a * p.x) - (b * p.y);
  }

  bool operator == (const line & l) const {
    return EQ(a, l.a) && EQ(b, l.b) && EQ(c, l.c);
  }

  bool operator != (const line & l) const {
    return !(*this == l);
  }

  //whether the line is initialized and normalized
  bool valid() const {
    if (EQ(a, 0)) return !EQ(b, 0);
    return EQ(b, 1) || (EQ(b, 0) && EQ(a, 1));
  }

  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }

  double slope() const {
    if (!valid() || EQ(b, 0)) return NaN; //vertical
    return -a;
  }

  //solve for x, given y
  //for horizontal lines, either +inf, -inf, or nan is returned
  double x(const double & y) const {
    if (!valid() || EQ(a, 0)) return NaN; //invalid or horizontal
    return (-c - b * y) / a;
  }

  //solve for y, given x
  //for vertical lines, either +inf, -inf, or nan is returned
  double y(const double & x) const {
    if (!valid() || EQ(b, 0)) return NaN; //invalid or vertical
    return (-c - a * x) / b;
  }

  //returns whether p exists on the line
  bool contains(const point & p) const {
    return EQ(a * p.x + b * p.y + c, 0);
  }

  //returns whether the line is parallel to l
  bool parallel(const line & l) const {
    return EQ(a, l.a) && EQ(b, l.b);
  }

  //returns whether the line is perpendicular to l
  bool perpendicular(const line & l) const {
    return EQ(-a * l.a, b * l.b);
  }

  //return the parallel line passing through point p
  line parallel(const point & p) const {
    return line(a, b, -a * p.x - b * p.y);
  }

  //return the perpendicular line passing through point p
  line perpendicular(const point & p) const {
    return line(-b, a, b * p.x - a * p.y);
  }

  friend std::ostream & operator << (std::ostream & out, const line & l) {
    out << (fabs(l.a) < eps ? 0 : l.a) << "x" << std::showpos;
    out << (fabs(l.b) < eps ? 0 : l.b) << "y";
    out << (fabs(l.c) < eps ? 0 : l.c) << "=0" << std::noshowpos;
    return out;
  }
};

const double PI = acos(-1.0), RAD = 180 / PI, DEG = PI / 180;

//reduce angles to the range [0, 360) degrees. e.g. reduce_deg(-630) = 90
double reduce_deg(const double & t) {
  if (t < -360) return reduce_deg(fmod(t, 360));
  if (t < 0) return t + 360;
  return t >= 360 ? fmod(t, 360) : t;
}

//reduce angles to the range [0, 2*pi) radians. e.g. reduce_rad(720.5) = 0.5
double reduce_rad(const double & t) {
  if (t < -2 * PI) return reduce_rad(fmod(t, 2 * PI));
  if (t < 0) return t + 2 * PI;
  return t >= 2 * PI ? fmod(t, 2 * PI) : t;
}

//like std::polar(), but returns a point instead of an std::complex
point polar_point(const double & r, const double & theta) {
  return point(r * cos(theta), r * sin(theta));
}

//angle of segment (0, 0) to p, relative (CCW) to the +'ve x-axis in radians
double polar_angle(const point & p) {
  double t = arg(p);
  return t < 0 ? t + 2 * PI : t;
}

//smallest angle formed by points aob (angle is at point o) in radians
double angle(const point & a, const point & o, const point & b) {
  point u(o - a), v(o - b);
  return acos(u.dot(v) / (abs(u) * abs(v)));
}

//angle of line segment ab relative (CCW) to the +'ve x-axis in radians
double angle_between(const point & a, const point & b) {
  double t = atan2(a.cross(b), a.dot(b)); //or, t = b.arg() - a.arg()
  return t < 0 ? t + 2 * PI : t;
}

//minimum angle in radians between two lines in the range [0, PI/2]
double angle_between(const line & l1, const line & l2) {
  double t = atan2(l1.a * l2.b - l2.a * l1.b, l1.a * l2.a + l1.b * l2.b);
  if (t < 0) t += PI; //force angle to be positive
  if (GT(t, PI / 2)) t = PI - t; //force angle to be <= 90 degrees
  return t;
}

//magnitude of the 3D cross product with Z component implicitly equal to 0
//the answer assumes the origin (0, 0) is instead shifted to point o.
//this is equal to 2x the signed area of the triangle from these 3 points.
double cross(const point & o, const point & a, const point & b) {
  return (a - o).cross(b - o);
}

//does the path a->o->b form:
// -1 ==> a left turn on the plane?
//  0 ==> a single straight line segment? (i.e. are a,o,b collinear?) or
// +1 ==> a right turn on the plane?
//warning: the order of parameters is a,o,b, and NOT o,a,b as in cross()
int turn(const point & a, const point & o, const point & b) {
  double c = cross(o, a, b);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

//distance and squared distance from point a to point b
double dist(const point & a, const point & b) { return abs(b - a); }
double dist2(const point & a, const point & b) { return norm(b - a); }

//minimum distance from point p to line l
//if a = b = 0, then -inf, nan, or +inf is returned depending on sgn(c)
double line_dist(const point & p, const line & l) {
  return fabs(l.a * p.x + l.b * p.y + l.c) / sqrt(l.a * l.a + l.b * l.b);
}

//minimum distance from point p to the infinite line containing a and b
//if a = b, then the point distance from p to the single point is returned
double line_dist(const point & p, const point & a, const point & b) {
  if (a == b) return dist(p, a);
  return abs(a + (p - a).dot(b - a) * (b - a) / dist2(a, b) - p);
}

//distance between two lines each denoted by the form ax + by + c = 0
//if the lines are nonparallel, then the distance is 0, otherwise
//it is the perpendicular distance from a point on one line to the other
double line_dist(const line & l1, const line & l2) {
  if (EQ(l1.a * l2.b, l2.a * l1.b)) {
    double factor = EQ(l1.b, 0) ? (l1.a / l2.a) : (l1.b / l2.b);
    if (EQ(l1.c, l2.c * factor)) return 0;
    return fabs(l2.c * factor - l1.c) / sqrt(l1.a * l1.a + l1.b * l1.b);
  }
  return 0;
}

//minimum distance from point p to line segment ab
double seg_dist(const point & p, const point & a, const point & b) {
  if (a == b) return dist(p, a);
  point ab(b - a), ap(p - a);
  double n = norm(ab), d = ab.dot(ap);
  if (LE(d, 0) || EQ(n, 0)) return abs(ap);
  if (GE(d, n)) return abs(ap - ab);
  return abs(ap - ab * (d / n));
}

//intersection of line l1 and line l2
//returns: -1, if lines do not intersect,
//          0, if there is exactly one intersection point, or
//         +1, if there are infinite intersections (lines are equal)
//in the 2nd case, the intersection point is optionally stored into p
int line_intersection(const line & l1, const line & l2, point * p = 0) {
  if (l1.parallel(l2)) return (l1 == l2) ? 1 : -1;
  if (p != 0) {
    p->x = (l1.b * l1.c - l1.b * l2.c) / (l2.a * l1.b - l1.a * l2.b);
    if (!EQ(l1.b, 0)) p->y = -(l1.a * p->x + l1.c) / l1.b;
    else p->y = -(l2.a * p->x + l2.c) / l2.b;
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
//in case 2, the intersection point is stored into p
//in case 3, the intersection segment is stored into p and q
int seg_intersection(const point & a, const point & b,
                     const point & c, const point & d,
                     point * p = 0, point * q = 0) {
  point ab(b - a), ac(c - a), cd(d - c);
  double c1 = ab.cross(cd), c2 = ac.cross(ab);
  if (EQ(c1, 0) && EQ(c2, 0)) { //collinear
    double t0 = ac.dot(ab) / norm(ab);
    double t1 = t0 + cd.dot(ab) / norm(ab);
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
  double t = ac.cross(cd) / c1, u = c2 / c1;
  if (contain(0, t, 1) && contain(0, u, 1)) {
    if (p != 0) *p = a + t * ab;
    return 0; //non-parallel with one intersection
  }
  return -1; //non-parallel with no intersections
}

//minimum distance from any point on segment ab to any point on segment cd
double seg_dist(const point & a, const point & b,
                 const point & c, const point & d) {
  //check if segments are touching or intersecting - if so, distance is 0
  if (seg_intersection(a, b, c, d) >= 0) return 0;
  //find min distances across each endpoint to opposing segment
  return std::min(std::min(seg_dist(a, c, d), seg_dist(b, c, d)),
                  std::min(seg_dist(c, a, b), seg_dist(d, a, b)));
}

//determines the point on line l that is closest to point p
//this always lies on the line through p perpendicular to l.
point closest_point(const line & l, const point & p) {
  if (EQ(l.a, 0)) return point(p.x, -l.c); //horizontal line
  if (EQ(l.b, 0)) return point(-l.c, p.y); //vertical line
  line perp = l.perpendicular(p);
  point res;
  line_intersection(l, perp, &res);
  return res;
}

//determines the point on segment ab closest to point p
point closest_point(const point & a, const point & b, const point & p) {
  if (a == b) return a;
  point ap(p - a), ab(b - a);
  double t = ap.dot(ab) / norm(ab);
  if (t <= 0) return a;
  if (t >= 1) return b;
  return point(a.x + t * ab.x, a.y + t * ab.y);
}

//if a point lies on the edge of a polygon, do we consider it inside?
const bool EDGE_IS_INSIDE = true;

//does p1 and p2 lie on the same side of the line containing a and b?
bool same_side(const point & p1, const point & p2,
               const point & a, const point & b) {
  point ab(b - a);
  double c1 = ab.cross(p1 - a);
  double c2 = ab.cross(p2 - a);
  if (EDGE_IS_INSIDE) return GE(c1 * c2, 0);
  return GT(c1 * c2, 0);
}

//does point p lie within in triangle abc?
bool point_in_triangle(const point & p, const point & a,
                       const point & b, const point & c) {
  return same_side(p, a, b, c) &&
         same_side(p, b, a, c) &&
         same_side(p, c, a, b);
}

//triangle area from its vertices
double triangle_area(const point & a, const point & b, const point & c) {
  return fabs(cross(a, b, c)) / 2.0;
}

//triangle area from its side lengths
//precondition: a,b,c >= 0 and forms a valid triangle
double triangle_area_sides(const double & a, const double & b,
                           const double & c) {
  double s = (a + b + c) / 2.0;
  return sqrt(s * (s - a) * (s - b) * (s - c));
}

//triangle area from its medians (a median is a line segment
//joining a vertex to the midpoint of the opposing side)
double triangle_area_medians(const double & m1, const double & m2,
                             const double & m3) {
  return 4.0 * triangle_area_sides(m1, m2, m3) / 3.0;
}

//triangle area from its altitudes (an altitude is the shortest line
//segment between a vertex and its possibly extended opposite side)
double triangle_area_altitudes(const double & h1, const double & h2,
                               const double & h3) {
  if (EQ(h1, 0) || EQ(h2, 0) || EQ(h3, 0)) return 0;
  double x = h1 * h1, y = h2 * h2, z = h3 * h3;
  double v = 2.0 / (x * y) + 2.0 / (x * z) + 2.0 / (y * z);
  v -= 1.0 / (x * x) + 1.0 / (y * y) + 1.0 / (z * z);
  return 1.0 / sqrt(v);
}

//does point p lie within the rectangle with bottom left at (x, y)
//and top right at (x + w, y + h)? Note that negative widths and heights
//are supported, representing rectangles that span left (instead of right)
//and down (instead of up), respectively, starting from (x, y).
bool point_in_rectangle(const point & p, const double & x, const double & y,
                                         const double & w, const double & h) {
  if (w < 0) return point_in_rectangle(p, x + w, y, -w, h);
  if (h < 0) return point_in_rectangle(p, x, y + h, w, -h);
  if (EDGE_IS_INSIDE)
    return GE(p.x, x) && LE(p.x, x + w) && GE(p.y, y) && LE(p.y, y + h);
  return GT(p.x, x) && LT(p.x, x + w) && GT(p.y, y) && LT(p.y, y + h);
}

//does point p lie within the rectangle with opposite corners a and b?
bool point_in_rectangle(const point & p, const point & a, const point & b) {
  double xl = std::min(a.x, b.x), yl = std::min(a.y, b.y);
  double xh = std::max(a.x, b.x), yh = std::max(a.y, b.y);
  return point_in_rectangle(p, xl, yl, xh - xl, yh - yl);
}

//area of rectangle with opposite corners
double rectangle_area(const point & a, const point & b) {
  return fabs((a.x - b.x) * (a.y - b.y));
}

//2D circle class - represented by (x - h)^2 + (y - k)^2 = r^2
//a generic circle has center point (h, k) and a radius r
struct circle {

  double h, k, r;

  circle(): h(0), k(0), r(0) {}
  circle(const double & R): h(0), k(0), r(fabs(R)) {}
  circle(const point & o, const double & R): h(o.x), k(o.y), r(fabs(R)) {}
  circle(const double & H, const double & K, const double & R):
    h(H), k(K), r(fabs(R)) {}

  //circumcircle with the diameter equal to the distance from a to b
  circle(const point & a, const point & b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = abs(a - point(h, k));
  }

  //circumcircle of 3 points - throws exception if abc are collinear/equal
  circle(const point & a, const point & b, const point & c) {
    double an = norm(b - c), bn = norm(a - c), cn = norm(a - b);
    double wa = an * (bn + cn - an);
    double wb = bn * (an + cn - bn);
    double wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0))
      throw std::runtime_error("No circle from collinear points.");
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = abs(a - point(h, k));
  }

  //circle from 2 points and a radius - many possible edge cases!
  //in the "normal" case, there will be 2 possible circles, one
  //centered at (h1, k1) and the other (h2, k2). Only one is used.
  //Note that (h1, k1) equals (h2, k2) if dist(a, b) = 2*r = d
  circle(const point & a, const point & b, const double & R) {
    r = fabs(R);
    if (LE(r, 0) && a == b) { //circle is a point
      h = a.x;
      k = a.y;
      return;
    }
    double d = abs(b - a);
    if (EQ(d, 0))
      throw std::runtime_error("Identical points, infinite circles.");
    if (LT(r * 2.0, d))
      throw std::runtime_error("Points too far away to make circle.");
    double v = sqrt(r * r - d * d / 4.0) / d;
    point m = (a + b) / 2.0;
    h = m.x + (a.y - b.y) * v;
    k = m.y + (b.x - a.x) * v;
    //other answer is (h, k) = (m.x-(a.y-b.y)*v, m.y-(b.x-a.x)*v)
  }

  bool operator == (const circle & c) const {
    return EQ(h, c.h) && EQ(k, c.k) && EQ(r, c.r);
  }

  bool operator != (const circle & c) const {
    return !(*this == c);
  }

  point center() const { return point(h, k); }
  bool contains(const point & p) const { return LE(norm(p - center()), r * r); }
  bool on_edge(const point & p) const { return EQ(norm(p - center()), r * r); }

  friend std::ostream & operator << (std::ostream & out, const circle & c) {
    out << std::showpos;
    out << "(x" << -(fabs(c.h) < eps ? 0 : c.h) << ")^2+";
    out << "(y" << -(fabs(c.k) < eps ? 0 : c.k) << ")^2";
    out << std::noshowpos;
    out << "=" << (fabs(c.r) < eps ? 0 : c.r * c.r);
    return out;
  }
};

//circle inscribed within points a, b, and c
circle incircle(const point & a, const point & b, const point & c) {
  double al = abs(b - c), bl = abs(a - c), cl = abs(a - b);
  double p = al + bl + cl;
  if (EQ(p, 0)) return circle(a.x, a.y, 0);
  return circle((al * a.x + bl * b.x + cl * c.x) / p,
                (al * a.y + bl * b.y + cl * c.y) / p,
                2.0 * triangle_area(a, b, c) / p);
}

//tangent line(s) to circle c passing through p. there are 3 cases:
//returns: 0, if there are no lines (p is strictly inside c)
//         1, if there is 1 tangent line (p is on the edge)
//         2, if there are 2 tangent lines (p is strictly outside)
//if there is only 1 tangent, then the line will be stored in l1
//if there are 2, then they will be stored in l1 and l2 respectively
int tangents(const circle & c, const point & p, line * l1 = 0, line * l2 = 0) {
  if (c.on_edge(p)) {
    if (l1 != 0) *l1 = line(point(c.h, c.k), p).perpendicular(p);
    return 1;
  }
  if (c.contains(p)) return 0;
  point q = (p - c.center()) / c.r;
  double n = norm(q), d = q.y * sqrt(norm(q) - 1.0);
  point t1((q.x - d) / n, c.k), t2((q.x + d) / n, c.k);
  if (NE(q.y, 0)) { //common case
    t1.y += c.r * (1.0 - t1.x * q.x) / q.y;
    t2.y += c.r * (1.0 - t2.x * q.x) / q.y;
  } else { //point at center horizontal, y = 0
    d = c.r * sqrt(1.0 - t1.x * t1.x);
    t1.y += d;
    t2.y -= d;
  }
  t1.x = t1.x * c.r + c.h;
  t2.x = t2.x * c.r + c.h;
  //note: here, t1 and t2 are the two points of tangencies
  if (l1 != 0) *l1 = line(p, t1);
  if (l2 != 0) *l2 = line(p, t2);
  return 2;
}

//determines the intersection(s) between a circle c and line l
//returns: 0, if the line does not intersect with the circle
//         1, if the line is tangent (one intersection)
//         2, if the line crosses through the circle
//if there is 1 intersection point, it will be stored in p
//if there are 2, they will be stored in p and q respectively
int intersection(const circle & c, const line & l,
                 point * p = 0, point * q = 0) {
  if (!l.valid())
    throw std::runtime_error("Invalid line for intersection.");
  double v = c.h * l.a + c.k * l.b + l.c;
  double aabb = l.a * l.a + l.b * l.b;
  double disc = v * v / aabb - c.r * c.r;
  if (disc > eps) return 0;
  double x0 = -l.a * l.c / aabb, y0 = -l.b * v / aabb;
  if (disc > -eps) {
    if (p != 0) *p = point(x0 + c.h, y0 + c.k);
    return 1;
  }
  double k = sqrt((disc /= -aabb) < 0 ? 0 : disc);
  if (p != 0) *p = point(x0 + k * l.b + c.h, y0 - k * l.a + c.k);
  if (q != 0) *q = point(x0 - k * l.b + c.h, y0 + k * l.a + c.k);
  return 2;
}

//determines the intersection points between two circles c1 and c2
//returns: -2, if circle c2 completely encloses circle c1
//         -1, if circle c1 completely encloses circle c2
//          0, if the circles are completely disjoint
//          1, if the circles are tangent (one intersection point)
//          2, if the circles intersect at two points
//          3, if the circles intersect at infinite points (c1 = c2)
//if one intersection, the intersection point is stored in p
//if two, the intersection points are stored in p and q respectively
int intersection(const circle & c1, const circle & c2,
                 point * p = 0, point * q = 0) {
  if (EQ(c1.h, c2.h) && EQ(c1.k, c2.k))
    return EQ(c1.r, c2.r) ? 3 : (c1.r > c2.r ? -1 : -2);
  point d12(c2.center() - c1.center());
  double d = abs(d12);
  if (GT(d, c1.r + c2.r)) return 0;
  if (LT(d, fabs(c1.r - c2.r))) return c1.r > c2.r ? -1 : -2;
  double a = (c1.r * c1.r - c2.r * c2.r + d * d) / (2 * d);
  double x0 = c1.h + (d12.x * a / d);
  double y0 = c1.k + (d12.y * a / d);
  double s = sqrt(c1.r * c1.r - a * a);
  double rx = -d12.y * s / d, ry = d12.x * s / d;
  if (EQ(rx, 0) && EQ(ry, 0)) {
    if (p != 0) *p = point(x0, y0);
    return 1;
  }
  if (p != 0) *p = point(x0 - rx, y0 - ry);
  if (q != 0) *q = point(x0 + rx, y0 + ry);
  return 2;
}

//intersection area of circles c1 and c2
double intersection_area(const circle & c1, const circle c2) {
  double r = std::min(c1.r, c2.r), R = std::max(c1.r, c2.r);
  double d = abs(c2.center() - c1.center());
  if (LE(d, R - r)) return PI * r * r;
  if (GE(d, R + r)) return 0;
  return r * r * acos((d * d + r * r - R * R) / 2 / d / r) +
         R * R * acos((d * d + R * R - r * r) / 2 / d / R) -
         0.5 * sqrt((-d + r + R) * (d + r - R) * (d - r + R) * (d + r + R));
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;
#define pt point

int main() {
  pt p, q;

  p = point(-10, 3);
  assert(pt(-18, 29) == p + pt(-3, 9) * 6 / 2 - pt(-1, 1));
  assert(EQ(109, p.norm()));
  assert(EQ(10.44030650891, p.abs()));
  assert(EQ(2.850135859112, p.arg()));
  assert(EQ(0,  p.dot(pt(3, 10))));
  assert(EQ(0,  p.cross(pt(10, -3))));
  assert(EQ(10, p.proj(pt(-10, 0))));
  assert(EQ(1,  p.normalize().abs()));
  assert(pt(-3, -10) == p.rot90());
  assert(pt(3, 12)   == p.rotateCW(pt(1, 1), PI / 2));
  assert(pt(1, -10)  == p.rotateCCW(pt(2, 2), PI / 2));
  assert(pt(10, -3)  == p.reflect(pt(0, 0)));
  assert(pt(-10, -3) == p.reflect(pt(-2, 0), pt(5, 0)));

  line l(2, -5, -8);
  line para = line(2, -5, -8).parallel(pt(-6, -2));
  line perp = line(2, -5, -8).perpendicular(pt(-6, -2));
  assert(l.parallel(para) && l.perpendicular(perp));
  assert(l.slope() == 0.4);
  assert(para == line(-0.4, 1, -0.4)); //-0.4x+1y-0.4=0
  assert(perp == line(2.5, 1, 17));    //2.5x+1y+17=0

  assert(EQ(angle_between(l, perp) * RAD, 90));

  assert(EQ(123,    reduce_deg(-(8 * 360) + 123)));
  assert(EQ(1.2345, reduce_rad(2 * PI * 8 + 1.2345)));
  assert(polar_point(4, PI) == pt(-4, 0));
  assert(polar_point(4, -PI/2) == pt(0, -4));
  assert(EQ(45,  polar_angle(pt(5, 5)) * RAD));
  assert(EQ(135, polar_angle(pt(-4, 4)) * RAD));
  assert(EQ(90,  angle(pt(5, 0), pt(0, 5), pt(-5, 0)) * RAD));
  assert(EQ(225, angle_between(pt(0, 5), pt(5, -5)) * RAD));
  assert(-1 == cross(pt(0, 0), pt(0, 1), pt(1, 0)));
  assert(+1 == turn(pt(0, 1), pt(0, 0), pt(-5, -5)));

  assert(EQ(5, dist(pt(-1, -1), pt(2, 3))));
  assert(EQ(25, dist2(pt(-1, -1), pt(2, 3))));
  assert(EQ(1.2, line_dist(pt(2, 1), line(-4, 3, -1))));
  assert(EQ(0.8, line_dist(pt(3, 3), pt(-1, -1), pt(2, 3))));
  assert(EQ(1.2, line_dist(pt(2, 1), pt(-1, -1), pt(2, 3))));
  assert(EQ(0.0, line_dist(line(-4, 3, -1), line(8, 6, 2))));
  assert(EQ(0.8, line_dist(line(-4, 3, -1), line(-8, 6, -10))));
  assert(EQ(1.0, seg_dist(pt(3, 3), pt(-1, -1), pt(2, 3))));
  assert(EQ(1.2, seg_dist(pt(2, 1), pt(-1, -1), pt(2, 3))));
  assert(EQ(0.0, seg_dist(pt(0, 2), pt(3, 3), pt(-1, -1), pt(2, 3))));
  assert(EQ(0.6, seg_dist(pt(-1, 0), pt(-2, 2), pt(-1, -1), pt(2, 3))));

  assert(line_intersection(line(-1, 1, 0), line(1, 1, -3), &p) == 0);
  assert(p == pt(1.5, 1.5));
  assert(line_intersection(pt(0, 0), pt(1, 1), pt(0, 4), pt(4, 0), &p) == 0);
  assert(p == pt(2, 2));

  //tests for segment intersection (examples in order from link below)
  //http://martin-thoma.com/how-to-check-if-two-line-segments-intersect/
  {
#define test(a,b,c,d,e,f,g,h) seg_intersection(pt(a,b),pt(c,d),pt(e,f),pt(g,h),&p,&q)
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

  assert(pt(2.5, 2.5) == closest_point(line(-1, -1, 5), pt(0, 0)));
  assert(pt(3, 0)     == closest_point(line(1, 0, -3), pt(0, 0)));
  assert(pt(0, 3)     == closest_point(line(0, 1, -3), pt(0, 0)));

  assert(pt(3, 0)  == closest_point(pt(3, 0), pt(3, 3), pt(0, 0)));
  assert(pt(2, -1) == closest_point(pt(2, -1), pt(4, -1), pt(0, 0)));
  assert(pt(4, -1) == closest_point(pt(2, -1), pt(4, -1), pt(5, 0)));

  assert(point_in_triangle(pt(0, 0), pt(-1, 0), pt(0, -2), pt(4, 0)));
  assert(!point_in_triangle(pt(0, 1), pt(-1, 0), pt(0, -2), pt(4, 0)));
  assert(point_in_triangle(pt(-2.44, 0.82), pt(-1, 0), pt(-3, 1), pt(4, 0)));
  assert(!point_in_triangle(pt(-2.44, 0.7), pt(-1, 0), pt(-3, 1), pt(4, 0)));

  assert(point_in_rectangle(pt(0, -1), 0, -3, 3, 2));
  assert(point_in_rectangle(pt(2, -2), 3, -3, -3, 2));
  assert(!point_in_rectangle(pt(0, 0), 3, -1, -3, -2));
  assert(point_in_rectangle(pt(2, -2), pt(3, -3), pt(0, -1)));
  assert(!point_in_rectangle(pt(-1, -2), pt(3, -3), pt(0, -1)));

  assert(EQ(6, triangle_area(pt(0, -1), pt(4, -1), pt(0, -4))));
  assert(EQ(6, triangle_area_sides(3, 4, 5)));
  assert(EQ(6, triangle_area_medians(3.605551275, 2.5, 4.272001873)));
  assert(EQ(6, triangle_area_altitudes(3, 4, 2.4)));

  circle c(-2, 5, sqrt(10)); //(x+2)^2+(y-5)^2=10
  assert(c == circle(point(-2, 5), sqrt(10)));
  assert(c == circle(point(1, 6), point(-5, 4)));
  assert(c == circle(point(-3, 2), point(-3, 8), point(-1, 8)));
  assert(c == incircle(point(-12, 5), point(3, 0), point(0, 9)));
  assert(c.contains(point(-2, 8)) && !c.contains(point(-2, 9)));
  assert(c.on_edge(point(-1, 2)) && !c.on_edge(point(-1.01, 2)));

  line l1, l2;
  assert(0 == tangents(circle(0, 0, 4), pt(1, 1), &l1, &l2));
  assert(1 == tangents(circle(0, 0, sqrt(2)), pt(1, 1), &l1, &l2));
  assert(l1 == line(-1, -1, 2));
  assert(2 == tangents(circle(0, 0, 2), pt(2, 2), &l1, &l2));
  assert(l1 == line(0, -2, 4));
  assert(l2 == line(2, 0, -4));

  assert(0 == intersection(circle(1, 1, 3), line(5, 3, -30), &p, &q));
  assert(1 == intersection(circle(1, 1, 3), line(0, 1, -4), &p, &q));
  assert(p == pt(1, 4));
  assert(2 == intersection(circle(1, 1, 3), line(0, 1, -1), &p, &q));
  assert(p == pt(4, 1));
  assert(q == pt(-2, 1));

  assert(-2 == intersection(circle(1, 1, 1), circle(0, 0, 3), &p, &q));
  assert(-1 == intersection(circle(0, 0, 3), circle(1, 1, 1), &p, &q));
  assert(0 == intersection(circle(5, 0, 4), circle(-5, 0, 4), &p, &q));
  assert(1 == intersection(circle(-5, 0, 5), circle(5, 0, 5), &p, &q));
  assert(p == pt(0, 0));
  assert(2 == intersection(circle(-0.5, 0, 1), circle(0.5, 0, 1), &p, &q));
  assert(p == pt(0, -sqrt(3) / 2));
  assert(q == pt(0, sqrt(3) / 2));

  //example where each circle passes through the other circle's center
  //http://math.stackexchange.com/a/402891
  double r = 3;
  double a = intersection_area(circle(-r / 2, 0, r), circle(r / 2, 0, r));
  assert(EQ(a, r * r * (2 * PI / 3 - sqrt(3) / 2)));

  return 0;
}
