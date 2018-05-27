/*

This file is an amalgamation of everything in sections 5.1 and 5.2, implemented
in a cross-dependent manner. All of these algorithms apply to a two-dimensional
Cartesian coordinate system where the positive x direction points towards the
right and the positive y direction points up on the page.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) for storage of all data types.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <utility>
#include <vector>

const double M_NAN = std::numeric_limits<double>::quiet_NaN();
const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define NE(a, b) (fabs((a) - (b)) > EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define GE(a, b) ((a) >= (b) - EPS)

// A two-dimensional point class that behaves like std::complex while supporting
// epsilon comparisons.
struct point {
  double x, y;

  point() : x(0), y(0) {}
  point(double x, double y) : x(x), y(y) {}
  point(const point &p) : x(p.x), y(p.y) {}
  point(const std::pair<double, double> &p) : x(p.first), y(p.second) {}

  bool operator<(const point &p) const {
    return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
  }

  bool operator>(const point &p) const {
    return EQ(x, p.x) ? LT(p.y, y) : LT(p.x, x);
  }

  bool operator==(const point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const point &p) const { return !(*this == p); }
  bool operator<=(const point &p) const { return !(*this > p); }
  bool operator>=(const point &p) const { return !(*this < p); }
  point operator+(const point &p) const { return point(x + p.x, y + p.y); }
  point operator-(const point &p) const { return point(x - p.x, y - p.y); }
  point operator+(double v) const { return point(x + v, y + v); }
  point operator-(double v) const { return point(x - v, y - v); }
  point operator*(double v) const { return point(x * v, y * v); }
  point operator/(double v) const { return point(x / v, y / v); }
  point& operator+=(const point &p) { x += p.x; y += p.y; return *this; }
  point& operator-=(const point &p) { x -= p.x; y -= p.y; return *this; }
  point& operator+=(double v) { x += v; y += v; return *this; }
  point& operator-=(double v) { x -= v; y -= v; return *this; }
  point& operator*=(double v) { x *= v; y *= v; return *this; }
  point& operator/=(double v) { x /= v; y /= v; return *this; }
  friend point operator+(double v, const point &p) { return p + v; }
  friend point operator*(double v, const point &p) { return p * v; }

  double sqnorm() const { return x*x + y*y; }
  double norm() const { return sqrt(x*x + y*y); }
  double arg() const { return atan2(y, x); }
  double dot(const point &p) const { return x*p.x + y*p.y; }
  double cross(const point &p) const { return x*p.y - y*p.x; }
  double proj(const point &p) const { return dot(p) / p.norm(); }

  // Returns a proportional unit vector (p, q) = c(x, y) where p^2 + q^2 = 1.
  point normalize() const {
    return (EQ(x, 0) && EQ(y, 0)) ? point(0, 0) : (point(x, y) / norm());
  }

  // Returns (x, y) rotated 90 degrees clockwise about the origin.
  point rotate90() const { return point(-y, x); }

  // Returns (x, y) rotated t radians clockwise about the origin.
  point rotateCW(double t) const {
    return point(x*cos(t) + y*sin(t), y*cos(t) - x*sin(t));
  }

  // Returns (x, y) rotated t radians counter-clockwise about the origin.
  point rotateCCW(double t) const {
    return point(x*cos(t) - y*sin(t), x*sin(t) + y*cos(t));
  }

  // Returns (x, y) rotated t radians clockwise about point p.
  point rotateCW(const point &p, double t) const {
    return (*this - p).rotateCW(t) + p;
  }

  // Returns (x, y) rotated t radians counter-clockwise about the point p.
  point rotateCCW(const point &p, double t) const {
    return (*this - p).rotateCCW(t) + p;
  }

  // Returns (x, y) reflected across point p.
  point reflect(const point &p) const {
    return point(2*p.x - x, 2*p.y - y);
  }

  // Returns (x, y) reflected across the line containing points p and q.
  point reflect(const point &p, const point &q) const {
    if (p == q) {
      return reflect(p);
    }
    point r(*this - p), s = q - p;
    r = point(r.x*s.x + r.y*s.y, r.x*s.y - r.y*s.x) / s.sqnorm();
    r = point(r.x*s.x - r.y*s.y, r.x*s.y + r.y*s.x) + p;
    return r;
  }

  friend double sqnorm(const point &p) { return p.sqnorm(); }
  friend double norm(const point &p) { return p.norm(); }
  friend double arg(const point &p) { return p.arg(); }
  friend double dot(const point &p, const point &q) { return p.dot(q); }
  friend double cross(const point &p, const point &q) { return p.cross(q); }
  friend double proj(const point &p, const point &q) { return p.proj(q); }
  friend point normalize(const point &p) { return p.normalize(); }
  friend point rotate90(const point &p) { return p.rotate90(); }

  friend point rotateCW(const point &p, double t) {
    return p.rotateCW(t);
  }

  friend point rotateCCW(const point &p, double t) {
    return p.rotateCCW(t);
  }

  friend point rotateCW(const point &p, const point &q, double t) {
    return p.rotateCW(q, t);
  }

  friend point rotateCCW(const point &p, const point &q, double t) {
    return p.rotateCCW(q, t);
  }

  friend point reflect(const point &p, const point &q) {
    return p.reflect(q);
  }

  friend point reflect(const point &p, const point &a, const point &b) {
    return p.reflect(a, b);
  }

  friend std::ostream& operator<<(std::ostream &out, const point &p) {
    return out << "(" << (fabs(p.x) < EPS ? 0 : p.x) << ","
                      << (fabs(p.y) < EPS ? 0 : p.y) << ")";
  }
};

// A two-dimensional line class stored of the form ax + by + c = 0, normalized
// such that b is always either 1 (for normal line) or 0 (for vertical lines).
struct line {
  double a, b, c;

  line() : a(0), b(0), c(0) {}  // Invalid or uninitialized line.

  line(double a, double b, double c) {
    if (!EQ(b, 0)) {
      this->a = a / b;
      this->c = c / b;
      this->b = 1;
    } else {
      this->c = c / a;
      this->a = 1;
      this->b = 0;
    }
  }

  line(double slope, const point &p) {
    a = -slope;
    b = 1;
    c = slope * p.x - p.y;
  }

  line(const point &p, const point &q) : a(0), b(0), c(0) {
    if (EQ(p.x, q.x)) {
      if (NE(p.y, q.y)) {  // Vertical line.
        a = 1;
        b = 0;
        c = -p.x;
      }  // Else, invalid line.
    } else {
      a = -(p.y - q.y) / (p.x - q.x);
      b = 1;
      c = -(a*p.x) - (b*p.y);
    }
  }

  bool operator==(const line &l) const {
    return EQ(a, l.a) && EQ(b, l.b) && EQ(c, l.c);
  }

  bool operator!=(const line &l) const {
    return !(*this == l);
  }

  // Returns whether the line is initialized and normalized.
  bool valid() const {
    if (EQ(a, 0)) {
      return !EQ(b, 0);
    }
    return EQ(b, 1) || (EQ(b, 0) && EQ(a, 1));
  }

  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }
  double slope() const { return (!valid() || EQ(b, 0)) ? M_NAN : -a; }

  // Solve for x at a given y. If the line is horizontal, then either -INF, INF,
  // or NAN is returned based on whether y is below, above, or on the line.
  double x(double y) const {
    if (!valid() || EQ(a, 0)) {
      return M_NAN;  // Invalid or horizontal line.
    }
    return (-c - b*y) / a;
  }

  // Solve for y at a given x. If the line is vertical, then either -INF, INF,
  // or NAN is returned based on whether x is left of, right of, or on the line.
  double y(double x) const {
    if (!valid() || EQ(b, 0)) {
      return M_NAN;  // Invalid or vertical line.
    }
    return (-c - a*x) / b;
  }

  bool contains(const point &p) const { return EQ(a*p.x + b*p.y + c, 0); }
  bool is_parallel(const line &l) const { return EQ(a, l.a) && EQ(b, l.b); }
  bool is_perpendicular(const line &l) const { return EQ(-a*l.a, b*l.b); }

  // Return the parallel line passing through point p.
  line parallel(const point &p) const {
    return line(a, b, -a*p.x - b*p.y);
  }

  // Return the perpendicular line passing through point p.
  line perpendicular(const point &p) const {
    return line(-b, a, b*p.x - a*p.y);
  }

  friend std::ostream& operator<<(std::ostream &out, const line &l) {
    return out << (fabs(l.a) < EPS ? 0 : l.a) << "x" << std::showpos
               << (fabs(l.b) < EPS ? 0 : l.b) << "y"
               << (fabs(l.c) < EPS ? 0 : l.c) << "=0" << std::noshowpos;
  }
};

const double PI = acos(-1.0), DEG = PI/180, RAD = 180/PI;

// Returns t degrees reduced to the range [0, 360). E.g. -630 becomes 90.
double reduce_deg(double t) {
  if (t < -360) {
    return reduce_deg(fmod(t, 360));
  }
  if (t < 0) {
    return t + 360;
  }
  return (t >= 360) ? fmod(t, 360) : t;
}

// Returns t radians reduced to the range [0, 2*pi). E.g. 720.5 becomes 0.5.
double reduce_rad(double t) {
  if (t < -2*PI) {
    return reduce_rad(fmod(t, 2*PI));
  }
  if (t < 0) {
    return t + 2*PI;
  }
  return (t >= 2*PI) ? fmod(t, 2*PI) : t;
}

// Returns a two-dimensional Cartesian point given radius r and angle t radians
// in polar coordinates, analogous to std::polar().
point polar_point(double r, double t) {
  return point(r*cos(t), r*sin(t));
}

// Returns the angle in radians of the segment from (0, 0) to point p, relative
// counterclockwise to the positive x-axis.
double polar_angle(const point &p) {
  double t = arg(p);
  return (t < 0) ? (t + 2*PI) : t;
}

// Returns the smallest angle in radians formed by the points a, o, b with
// vertex at point o.
double angle(const point &a, const point &o, const point &b) {
  point u(o - a), v(o - b);
  return acos(u.dot(v) / (norm(u)*norm(v)));
}

// Returns the angle in radians of segment from point a to point b, relative
// counterclockwise to the positive x-axis.
double angle_between(const point &a, const point &b) {
  double t = atan2(a.cross(b), a.dot(b));  // Equivalently, b.arg() - a.arg().
  return (t < 0) ? (t + 2*PI) : t;
}

// Returns the smaller angle in radians between two lines, limited to [0, PI/2].
double angle_between(const line &l1, const line &l2) {
  double t = atan2(l1.a*l2.b - l2.a*l1.b, l1.a*l2.a + l1.b*l2.b);
  if (t < 0) {
    t += PI;
  }
  return GT(t, PI / 2) ? (PI - t) : t;
}

// Returns the magnitude (Euclidean norm) of the three-dimensional cross product
// between points a and b where the z-component is implicitly zero and the
// origin is implicitly shifted to point o. This operation is also equal to
// double the signed area of the triangle from these three points.
double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a - o).cross(b - o);
}

// Returns -1 if the path a->o->b forms a left turn on the plane, 0 if the path
// forms a straight line segment (collinear), or 1 if it forms a right turn.
int turn(const point &a, const point &o, const point &b) {
  double c = cross(a, b, o);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

// Returns the distance and squared distance between points a and b.
double dist(const point &a, const point &b) { return norm(b - a); }
double sqdist(const point &a, const point &b) { return sqnorm(b - a); }

// Returns the distance from point p to line l. If l is invalid (l.a = l.b = 0),
// then -INF, INF, or NaN is returned based on the sign of l.c.
double line_dist(const point &p, const line &l) {
  return fabs(l.a*p.x + l.b*p.y + l.c) / sqrt(l.a*l.a + l.b*l.b);
}

// Returns the distance from point p to the line (not segment) containing points
// a and b. If a = b, then the distance from p to the single point is returned.
double line_dist(const point &p, const point &a, const point &b) {
  return (a == b) ? dist(p, a)
                  : norm(a + (p - a).dot(b - a)*(b - a) / sqdist(a, b) - p);
}

// Returns the distance between two lines. If the lines are non-parallel then
// the distance is considered to be 0. Otherwise, the distance is considered to
// be the perpendicular distance from any point on one line to the other line.
double line_dist(const line &l1, const line &l2) {
  if (EQ(l1.a*l2.b, l2.a*l1.b)) {
    double factor = EQ(l1.b, 0) ? (l1.a / l2.a) : (l1.b / l2.b);
    return EQ(l1.c, l2.c*factor) ? 0
             : fabs(l2.c*factor - l1.c) / sqrt(l1.a*l1.a + l1.b*l1.b);
  }
  return 0;
}

// Returns the distance from point p to the line segment ab.
double seg_dist(const point &p, const point &a, const point &b) {
  if (a == b) {
    return dist(p, a);
  }
  point ab(b - a), ap(p - a);
  double n = sqnorm(ab), d = ab.dot(ap);
  if (LE(d, 0) || EQ(n, 0)) {
    return norm(ap);
  }
  return GE(d, n) ? norm(ap - ab) : norm(ap - ab*(d / n));
}

// Determines whether lines l1 and l2 intersect. Returns -1 if the lines are
// parallel and there is no intersection, 0 if there is exactly one intersection
// (in which case the intersection point is stored into pointer p if it's not
// NULL), or 1 if the lines are equal and there are infinite intersections.
int line_intersection(const line &l1, const line &l2, point *p = NULL) {
  if (l1.is_parallel(l2)) {
    return (l1 == l2) ? 1 : -1;
  }
  if (p != NULL) {
    p->x = (l1.b*l1.c - l1.b*l2.c) / (l2.a*l1.b - l1.a*l2.b);
    if (!EQ(l1.b, 0)) {
      p->y = -(l1.a*p->x + l1.c) / l1.b;
    } else {
      p->y = -(l2.a*p->x + l2.c) / l2.b;
    }
  }
  return 0;
}

// Determines whether the two lines (not segments) through points p1, p2 and
// through points p3 and p4 intersect. Returns -1 if the lines are parallel and
// there is no intersection, 0 if there is exactly one intersection (in which
// case the intersection point is stored into pointer p if it's not NULL), or 1
// if the lines are equal and there are infinite intersections.
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

// Determines whether the line segment ab intersects the line segment cd.
// Returns -1 if the segments do not intersect, 0 if there is exactly one
// intersection point (in which case it is stored into pointer p if it's not
// NULL), or 1 if the intersection is another line segment (in which case the
// two endpoints are stored into pointers p and q if they are not NULL). If the
// segments are barely touching (close within EPS), then the result will depend
// on the setting of TOUCH_IS_INTERSECT.
int seg_intersection(const point &a, const point &b,
                     const point &c, const point &d,
                     point *p = NULL, point *q = NULL) {
  static const bool TOUCH_IS_INTERSECT = true;
  point ab(b - a), ac(c - a), cd(d - c);
  double c1 = ab.cross(cd), c2 = ac.cross(ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    double t0 = ac.dot(ab) / sqnorm(ab);
    double t1 = t0 + cd.dot(ab) / sqnorm(ab);
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
  double t = ac.cross(cd)/c1, u = c2/c1;
  bool t_between_01 = TOUCH_IS_INTERSECT ? (LE(0, t) && LE(t, 1))
                                         : (LT(0, t) && LT(t, 1));
  bool u_between_01 = TOUCH_IS_INTERSECT ? (LE(0, u) && LE(u, 1))
                                         : (LT(0, u) && LT(u, 1));
  if (t_between_01 && u_between_01) {
    if (p != NULL) {
      *p = a + t*ab;
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

// Returns the minimum distance from any point on the line segment ab to any
// point on the line segment cd. This is 0 if the segments touch or intersect.
double seg_dist(const point & a, const point & b,
                const point & c, const point & d) {
  return (seg_intersection(a, b, c, d) >= 0) ? 0
           : std::min(std::min(seg_dist(a, c, d), seg_dist(b, c, d)),
                      std::min(seg_dist(c, a, b), seg_dist(d, a, b)));
}

// Returns the point on line l that is closest to point p. The result always
// lies on the line through p that is perpendicular to l.
point closest_point(const line &l, const point &p) {
  if (EQ(l.a, 0)) {
    return point(p.x, -l.c);  // Horizontal line.
  }
  if (EQ(l.b, 0)) {
    return point(-l.c, p.y);  // Vertical line.
  }
  point res;
  line_intersection(l, l.perpendicular(p), &res);
  return res;
}

// Returns the point on line segment ab that is closest to point p.
point closest_point(const point &a, const point &b, const point &p) {
  if (a == b) {
    return a;
  }
  point ap(p - a), ab(b - a);
  double t = ap.dot(ab) / sqnorm(ab);
  return (t <= 0) ? a : ((t >= 1) ? b : point(a.x + t*ab.x, a.y + t*ab.y));
}

// Returns whether points p1 and p2 lie on the same side of the line containing
// points a and b. If one or both points lie exactly on the line, then the
// result will depend on the setting of EDGE_IS_SAME_SIDE.
bool same_side(const point &p1, const point &p2,
               const point &a, const point &b) {
  static const bool EDGE_IS_SAME_SIDE = true;
  point ab(b - a);
  double c1 = ab.cross(p1 - a), c2 = ab.cross(p2 - a);
  return EDGE_IS_SAME_SIDE ? GE(c1*c2, 0) : GT(c1*c2, 0);
}

// Returns whether point p lies within the triangle abc. If the point lies on or
// close to an edge (by roughly EPS), then the result will depend on the setting
// of EDGE_IS_SAME_SIDE in the function above.
bool point_in_triangle(const point &p,
                       const point &a, const point &b, const point &c) {
  return same_side(p, a, b, c) &&
         same_side(p, b, a, c) &&
         same_side(p, c, a, b);
}

// Returns the area of the triangle abc.
double triangle_area(const point &a, const point &b, const point &c) {
  return fabs(cross(a, b, c)) / 2.0;
}

// Returns the area of a triangle with side lengths a, b, and c. The given
// lengths must be non-negative and form a valid triangle.
double triangle_area_sides(double a, double b, double c) {
  double s = (a + b + c) / 2.0;
  return sqrt(s*(s - a)*(s - b)*(s - c));
}

// Returns the area of a triangle with medians of lengths m1, m2, and m3. The
// median of a triangle is a line segment joining a vertex to the midpoint of
// the opposing edge.
double triangle_area_medians(double m1, double m2,
                             double m3) {
  return 4.0*triangle_area_sides(m1, m2, m3) / 3.0;
}

// Returns the area of a triangle with altitudes h1, h2, and h3. An altitude of
// a triangle is the shortest line between a vertex and the infinite line that
// is extended from its opposite edge.
double triangle_area_altitudes(double h1, double h2,
                               double h3) {
  if (EQ(h1, 0) || EQ(h2, 0) || EQ(h3, 0)) {
    return 0;
  }
  double x = h1*h1, y = h2*h2, z = h3*h3;
  double v = 2.0/(x*y) + 2.0/(x*z) + 2.0/(y*z);
  return 1.0/sqrt(v - 1.0/(x*x) - 1.0/(y*y) - 1.0/(z*z));
}

// Returns whether point p lies within the rectangle defined by a vertex at
// (x, y), a width of w, and a height of h. Note that negative widths and
// heights are supported. If the point lies on or close to an edge (by roughly
// EPS), then the result will depend on the setting of EDGE_IS_INSIDE.
bool point_in_rectangle(const point &p, double x, double y,
                                        double w, double h) {
  static const bool EDGE_IS_INSIDE = true;
  if (w < 0) {
    return point_in_rectangle(p, x + w, y, -w, h);
  }
  if (h < 0) {
    return point_in_rectangle(p, x, y + h, w, -h);
  }
  return EDGE_IS_INSIDE
           ? (GE(p.x, x) && LE(p.x, x + w) && GE(p.y, y) && LE(p.y, y + h))
           : (GT(p.x, x) && LT(p.x, x + w) && GT(p.y, y) && LT(p.y, y + h));
}

// Returns whether point p lies within the rectangle with opposing vertices a
// and b. If the point lies on or close to an edge (by roughly EPS), then the
// result will depend on the setting of EDGE_IS_INSIDE in the function above.
bool point_in_rectangle(const point &p, const point &a, const point &b) {
  double xl = std::min(a.x, b.x), yl = std::min(a.y, b.y);
  double xh = std::max(a.x, b.x), yh = std::max(a.y, b.y);
  return point_in_rectangle(p, xl, yl, xh - xl, yh - yl);
}

// Returns the area of a rectangle with opposing vertices a and b.
double rectangle_area(const point &a, const point &b) {
  return fabs((a.x - b.x)*(a.y - b.y));
}

// A two-dimensional circle class represented by (x - h)^2 + (y - k)^2 = r^2,
// where (h, k) is the center and r is the radius.
struct circle {
  double h, k, r;

  circle() : h(0), k(0), r(0) {}
  circle(double r) : h(0), k(0), r(fabs(r)) {}
  circle(const point &o, double r) : h(o.x), k(o.y), r(fabs(r)) {}
  circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  // Circle with the line segment ab as a diameter.
  circle(const point &a, const point &b) {
    h = (a.x + b.x)/2.0;
    k = (a.y + b.y)/2.0;
    r = norm(a - point(h, k));
  }

  // Circumcircle of three points.
  circle(const point &a, const point &b, const point &c) {
    double an = sqnorm(b - c), bn = sqnorm(a - c), cn = sqnorm(a - b);
    double wa = an*(bn + cn - an);
    double wb = bn*(an + cn - bn);
    double wc = cn*(an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circle from collinear points.");
    }
    h = (wa*a.x + wb*b.x + wc*c.x)/w;
    k = (wa*a.y + wb*b.y + wc*c.y)/w;
    r = norm(a - point(h, k));
  }

  // Circle of radius r that contains points a and b. In the general case, there
  // will be two possible circles and only one is chosen arbitrarily. However if
  // the diameter is equal to dist(a, b) = 2*r, then there is only one possible
  // center. If points a and b are identical, then there are infinite circles.
  // If the points are too far away relative to the radius, then there is no
  // possible circle. In the latter two cases, an exception is thrown.
  circle(const point &a, const point &b, double r) : r(fabs(r)) {
    if (LE(r, 0) && a == b) {  // Circle with zero area.
      h = a.x;
      k = a.y;
      return;
    }
    double d = norm(b - a);
    if (EQ(d, 0)) {
      throw std::runtime_error("Identical points, infinite circles.");
    }
    if (LT(r*2.0, d)) {
      throw std::runtime_error("Points too far away to make circle.");
    }
    double v = sqrt(r*r - d*d/4.0) / d;
    point m = (a + b) / 2.0;
    h = m.x + v*(a.y - b.y);
    k = m.y + v*(b.x - a.x);
    // The other answer is (h, k) = (m.x - v*(a.y - b.y), m.y - v*(b.x - a.x)).
  }

  bool operator==(const circle &c) const {
    return EQ(h, c.h) && EQ(k, c.k) && EQ(r, c.r);
  }

  bool operator!=(const circle &c) const {
    return !(*this == c);
  }

  point center() const { return point(h, k); }
  bool contains(const point &p) const { return LE(sqnorm(p - center()), r*r); }
  bool on_edge(const point &p) const { return EQ(sqnorm(p - center()), r*r); }

  friend std::ostream& operator<<(std::ostream &out, const circle &c) {
    return out << std::showpos << "(x" << -(fabs(c.h) < EPS ? 0 : c.h) << ")^2+"
                               << "(y" << -(fabs(c.k) < EPS ? 0 : c.k) << ")^2"
               << std::noshowpos << "=" << (fabs(c.r) < EPS ? 0 : c.r*c.r);
  }
};

// Returns the circle inscribed inside the triangle abc.
circle incircle(const point &a, const point &b, const point &c) {
  double al = norm(b - c), bl = norm(a - c), cl = norm(a - b), p = al + bl + cl;
  return EQ(p, 0) ? circle(a.x, a.y, 0)
                  : circle((al*a.x + bl*b.x + cl*c.x) / p,
                           (al*a.y + bl*b.y + cl*c.y) / p,
                           2.0*triangle_area(a, b, c) / p);
}

// Determines the line(s) tangent to circle c that passes through point p.
// Returns -1 if there is no tangent line because p is strictly inside c, 0 if
// there is exactly one tangent line because p is on the boundary of c (in which
// case the line will be stored into pointer l1 if it's not NULL), or 1 if there
// are two tangent lines because p is strictly outside of c (in which case the
// lines will be stored into pointers l1 and l2 if they are not NULL).
int tangent(const circle &c, const point &p, line *l1 = NULL, line *l2 = NULL) {
  if (c.on_edge(p)) {
    if (l1 != NULL) {
      *l1 = line(point(c.h, c.k), p).perpendicular(p);
    }
    return 0;
  }
  if (c.contains(p)) {
    return -1;
  }
  point q = (p - c.center()) / c.r;
  double n = sqnorm(q), d = q.y*sqrt(sqnorm(q) - 1.0);
  point t1((q.x - d) / n, c.k), t2((q.x + d) / n, c.k);
  if (NE(q.y, 0)) {
    t1.y += c.r*(1.0 - t1.x*q.x) / q.y;
    t2.y += c.r*(1.0 - t2.x*q.x) / q.y;
  } else {
    d = c.r*sqrt(1.0 - t1.x*t1.x);
    t1.y += d;
    t2.y -= d;
  }
  t1.x = t1.x*c.r + c.h;
  t2.x = t2.x*c.r + c.h;
  if (l1 != NULL && l2 != NULL) {
    *l1 = line(p, t1);
    *l2 = line(p, t2);
  }
  return 1;
}

// Determines the intersection between the circle c and line l. Returns -1 if
// there is no intersection, 0 if the line is one intersection point because the
// line is tangent (in which case it will be stored into pointer p if it's not
// NULL), or 1 if there are two intersection points because the line crosses
// through the circle (in which case they will be stored into pointers p and q
// if they are not NULL).
int intersection(const circle &c, const line &l,
                 point *p = NULL, point *q = NULL) {
  if (!l.valid()) {
    throw std::runtime_error("Invalid line for intersection.");
  }
  double v = c.h*l.a + c.k*l.b + l.c;
  double aabb = l.a*l.a + l.b*l.b;
  double disc = v*v / aabb - c.r*c.r;
  if (disc > EPS) {
    return -1;
  }
  double x0 = -l.a*l.c / aabb, y0 = -l.b*v / aabb;
  if (disc > -EPS) {
    if (p != NULL) {
      *p = point(x0 + c.h, y0 + c.k);
    }
    return 0;
  }
  double k = sqrt(std::max(0.0, disc / -aabb));
  if (p != NULL && q != NULL) {
    *p = point(x0 + k*l.b + c.h, y0 - k*l.a + c.k);
    *q = point(x0 - k*l.b + c.h, y0 + k*l.a + c.k);
  }
  return 1;
}

// Determines the intersection between two circles c1 and c2.
// Returns: -2 if circle c2 completely encloses circle c1,
//          -1 if circle c1 completely encloses circle c2,
//           0 if the circles are completely disjoint,
//           1 if the circles are tangent with one intersection (stored in p),
//           2 if the circles intersect at two points (stored in p and q),
//           3 if the circles are equal and intersect at infinite points.
int intersection(const circle &c1, const circle &c2,
                 point *p = NULL, point *q = NULL) {
  if (EQ(c1.h, c2.h) && EQ(c1.k, c2.k)) {
    return EQ(c1.r, c2.r) ? 3 : (c1.r > c2.r ? -1 : -2);
  }
  point d12(c2.center() - c1.center());
  double d = norm(d12);
  if (GT(d, c1.r + c2.r)) {
    return 0;
  }
  if (LT(d, fabs(c1.r - c2.r))) {
    return c1.r > c2.r ? -1 : -2;
  }
  double a = (c1.r*c1.r - c2.r*c2.r + d*d) / (2*d);
  double x0 = c1.h + (d12.x*a / d), y0 = c1.k + (d12.y*a / d);
  double s = sqrt(c1.r*c1.r - a*a), rx = -d12.y*s / d, ry = d12.x*s / d;
  if (EQ(rx, 0) && EQ(ry, 0)) {
    if (p != NULL) {
      *p = point(x0, y0);
    }
    return 1;
  }
  if (p != NULL && q != NULL) {
    *p = point(x0 - rx, y0 - ry);
    *q = point(x0 + rx, y0 + ry);
  }
  return 2;
}

// Returns the intersection area of circles c1 and c2.
double intersection_area(const circle &c1, const circle &c2) {
  double r = std::min(c1.r, c2.r), R = std::max(c1.r, c2.r);
  double d = norm(c2.center() - c1.center());
  if (LE(d, R - r)) {
    return PI*r*r;
  }
  if (GE(d, R + r)) {
    return 0;
  }
  return r*r*acos((d*d + r*r - R*R) / 2 / d / r) +
         R*R*acos((d*d + R*R - r*r) / 2 / d / R) -
         0.5*sqrt((-d + r + R)*(d + r - R)*(d - r + R)*(d + r + R));
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

#define pt point

int main() {
  pt p, q;

  p = point(-10, 3);
  assert(pt(-18, 29) == p + pt(-3, 9)*6 / 2 - pt(-1, 1));
  assert(EQ(109, p.sqnorm()));
  assert(EQ(10.44030650891, p.norm()));
  assert(EQ(2.850135859112, p.arg()));
  assert(EQ(0, p.dot(pt(3, 10))));
  assert(EQ(0, p.cross(pt(10, -3))));
  assert(EQ(10, p.proj(pt(-10, 0))));
  assert(EQ(1, p.normalize().norm()));
  assert(pt(-3, -10) == p.rotate90());
  assert(pt(3, 12) == p.rotateCW(pt(1, 1), PI / 2));
  assert(pt(1, -10) == p.rotateCCW(pt(2, 2), PI / 2));
  assert(pt(10, -3) == p.reflect(pt(0, 0)));
  assert(pt(-10, -3) == p.reflect(pt(-2, 0), pt(5, 0)));

  line l(2, -5, -8);
  line para = line(2, -5, -8).parallel(pt(-6, -2));
  line perp = line(2, -5, -8).perpendicular(pt(-6, -2));
  assert(l.is_parallel(para) && l.is_perpendicular(perp));
  assert(l.slope() == 0.4);
  assert(para == line(-0.4, 1, -0.4));  // -0.4x + y - 0.4 = 0.
  assert(perp == line(2.5, 1, 17));  // 2.5x + y + 17 = 0.

  assert(EQ(angle_between(l, perp), 90*DEG));

  assert(EQ(123, reduce_deg(-8*360 + 123)));
  assert(EQ(1.2345, reduce_rad(2*PI*8 + 1.2345)));
  assert(polar_point(4, PI) == pt(-4, 0));
  assert(polar_point(4, -PI/2) == pt(0, -4));
  assert(EQ(45, polar_angle(pt(5, 5))*RAD));
  assert(EQ(135, polar_angle(pt(-4, 4))*RAD));
  assert(EQ(90, angle(pt(5, 0), pt(0, 5), pt(-5, 0))*RAD));
  assert(EQ(225, angle_between(pt(0, 5), pt(5, -5))*RAD));
  assert(-1 == cross(pt(0, 1), pt(1, 0), pt(0, 0)));
  assert(1 == turn(pt(0, 1), pt(0, 0), pt(-5, -5)));

  assert(EQ(5, dist(pt(-1, -1), pt(2, 3))));
  assert(EQ(25, sqdist(pt(-1, -1), pt(2, 3))));
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

  {
    #define test(a, b, c, d, e, f, g, h) \
         seg_intersection(pt(a, b), pt(c, d), pt(e, f), pt(g, h), &p, &q)

    // Intersection is a point.
    assert(0 == test(-4, 0, 4, 0, 0, -4, 0, 4) && p == pt(0, 0));
    assert(0 == test(0, 0, 10, 10, 2, 2, 16, 4) && p == pt(2, 2));
    assert(0 == test(-2, 2, -2, -2, -2, 0, 0, 0) && p == pt(-2, 0));
    assert(0 == test(0, 4, 4, 4, 4, 0, 4, 8) && p == pt(4, 4));

    // Intersection is a segment.
    assert(1 == test(10, 10, 0, 0, 2, 2, 6, 6));
    assert(p == pt(2, 2) && q == pt(6, 6));
    assert(1 == test(6, 8, 14, -2, 14, -2, 6, 8));
    assert(p == pt(6, 8) && q == pt(14, -2));

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

  assert(pt(2.5, 2.5) == closest_point(line(-1, -1, 5), pt(0, 0)));
  assert(pt(3, 0) == closest_point(line(1, 0, -3), pt(0, 0)));
  assert(pt(0, 3) == closest_point(line(0, 1, -3), pt(0, 0)));

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

  circle c(-2, 5, sqrt(10));
  assert(c == circle(point(-2, 5), sqrt(10)));
  assert(c == circle(point(1, 6), point(-5, 4)));
  assert(c == circle(point(-3, 2), point(-3, 8), point(-1, 8)));
  assert(c == incircle(point(-12, 5), point(3, 0), point(0, 9)));
  assert(c.contains(point(-2, 8)) && !c.contains(point(-2, 9)));
  assert(c.on_edge(point(-1, 2)) && !c.on_edge(point(-1.01, 2)));

  line l1, l2;
  assert(-1 == tangent(circle(0, 0, 4), pt(1, 1), &l1, &l2));
  assert(0 == tangent(circle(0, 0, sqrt(2)), pt(1, 1), &l1, &l2));
  assert(l1 == line(-1, -1, 2));
  assert(1 == tangent(circle(0, 0, 2), pt(2, 2), &l1, &l2));
  assert(l1 == line(0, -2, 4));
  assert(l2 == line(2, 0, -4));

  assert(-1 == intersection(circle(1, 1, 3), line(5, 3, -30), &p, &q));
  assert(0 == intersection(circle(1, 1, 3), line(0, 1, -4), &p, &q));
  assert(p == pt(1, 4));
  assert(1 == intersection(circle(1, 1, 3), line(0, 1, -1), &p, &q));
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

  // Each circle passes through the other's center.
  double r = 3, a = intersection_area(circle(-r/2, 0, r), circle(r/2, 0, r));
  assert(EQ(a, r*r*(2*PI / 3 - sqrt(3) / 2)));

  return 0;
}
