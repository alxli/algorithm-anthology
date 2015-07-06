/*

5.0 - 2D Geometry Library

This is the entire chapter in one file for easy access due to cross-
dependencies. This file will try to take advantage of previously defined
classes, unlike the split-up sections which will be more portable in
definitions (where, for example, cross() may be implemented with the
formula in terms of x and y, instead of using the point arithmetic and
cross() function previously defined).

All of the functions below apply to a 2D cartesian coordinate system
where the positive x directions points to the "right" and the positive y
direction points "up".

Sections will be split up after this file is finalized.

*/

#include <algorithm> /* std::sort(), std::swap() */
#include <cmath>     /* fabs, sqrt, sin, cos, atan2 */
#include <limits>    /* std::numeric_limits */
#include <ostream>
#include <utility>   /* std::pair */
#include <vector>

const double posinf = std::numeric_limits<double>::max();
const double neginf = -std::numeric_limits<double>::max();
const double NaN = std::numeric_limits<double>::quiet_NaN();

const double eps = 1e-9;

#define EQ(a, b) (fabsl((a) - (b)) <= eps) /* equal to */
#define NE(a, b) (fabsl((a) - (b)) > eps)  /* not equal to */
#define LT(a, b) ((a) < (b) - eps)         /* less than */
#define GT(a, b) ((a) > (b) + eps)         /* greater than */
#define LE(a, b) ((a) <= (b) + eps)        /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)        /* greater than or equal to */

const double PI = acos(-1.0), RAD = 180 / PI, DEG = PI / 180;

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
    return EQ(x, p.x) ? GT(y, p.y) : GT(x, p.x);
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
  double proj(const point & p) const { return dot(p) / abs(); }
  point rot90() const { return point(-y, x); }

  //proportional unit vector of (x, y) such that x^2 + y^2 = 1
  point normalize() const {
    return (EQ(x, 0) && EQ(y, 0)) ? point(0, 0) : (point(x, y) / abs());
  }

  //rotate t radians CW about origin
  point rotateCW(double t) const {
    return point(x * cos(t) + y * sin(t), y * cos(t) - x * sin(t));
  }

  //rotate t radians CCW about origin
  point rotateCCW(double t) const {
    return point(x * cos(t) - y * sin(t), x * sin(t) + y * cos(t));
  }

  //rotate t radians CW about point p
  point rotateCW(const point & p, double t) const {
    return (*this - p).rotateCW(t) + p;
  }

  //rotate t radians CCW about point p
  point rotateCCW(const point & p, double t) const {
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

  void normalize() {
    if (a < 0 || (EQ(a, 0) && b < 0)) {
      a = -a; b = -b; c = -c;
    }
    if (!EQ(b, 0)) {
      a /= b; c /= b; b = 1;
    }
  }

  line() : a(0), b(0), c(0) {} //invalid/uninitialized line

  line(double A, double B, double C) : a(A), b(B), c(C) {
    normalize();
  }

  line(double slope, const point & p) {
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
    a = q.y - p.y;
    b = p.x - q.x;
    c = p.y * q.x - p.x * q.y;
    normalize();
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
    return EQ(b, 0) || EQ(b, 1);
  }

  bool horizontal() const { return valid() && EQ(a, 0); }
  bool vertical() const { return valid() && EQ(b, 0); }

  double slope() const {
    if (!valid() || EQ(b, 0)) return NaN; //vertical
    return -a;
  }

  //solve for x, given y
  //for horizontal lines, either +inf, -inf, or nan is returned
  double x(double y) const {
    if (!valid()) return NaN;
    if (EQ(a, 0)) { //horizontal line
      if (EQ(y, -c / b)) return NaN; //y = the line
      if (LT(y, -c / b)) return neginf; //y is below
      return posinf; //y is above
    }
    return (-c - b * y) / a;
  }

  //solve for y, given x
  //for vertical lines, either +inf, -inf, or nan is returned
  double y(double x) const {
    if (!valid()) return NaN;
    if (EQ(b, 0)) { //vertical line
      if (EQ(x, -c / a)) return NaN; //x is the line
      if (LT(x, -c / a)) return posinf; //x is right
      return neginf; //x is left
    }
    return (-c - a * x) / b;
  }

  //return the parallel line passing through point p
  line parallel(const point & p) const {
    return line(a, b, -a * p.x - b * p.y);
  }

  //return the perpendicular line passing through point p
  line perpendicular(const point & p) const {
    return line(-b, a, b * p.x - a * p.y);
  }

  //returns whether the line is parallel to l
  bool parallel(const line & l) const {
    return EQ(a, l.a) && EQ(b, l.b);
  }

  //returns whether the line is perpendicular to l
  bool perpendicular(const line & l) const {
    return EQ(-a * l.a, b * l.b);
  }

  friend std::ostream & operator << (std::ostream & out, const line & l) {
    out << (fabs(l.a) < eps ? 0 : l.a) << "x" << std::showpos;
    out << (fabs(l.b) < eps ? 0 : l.b) << "y";
    out << (fabs(l.c) < eps ? 0 : l.c) << "=0" << std::noshowpos;
    return out;
  }
};

//distance and squared distance from point a to point b
double dist(const point & a, const point & b) { return (b - a).abs(); }
double dist2(const point & a, const point & b) { return (b - a).norm(); }

//minimum distance from point p to line l
double dist(const point & p, const line & l) {
  return fabs(l.a * p.x + l.b * p.y + l.c) / point(l.a, l.b).abs();
}

//minimum distance from point p to line segment ab
double dist(const point & p, const point & a, const point & b) {
  if (a == b) return dist(p, a);
  point ab(b - a), ap(p - a);
  double norm = ab.norm(), dot = ab.dot(ap);
  if (LE(dot, 0) || EQ(norm, 0)) return ap.abs();
  if (GE(dot, norm)) return (ap - ab).abs();
  return (ap - ab * (dot / norm)).abs();
}

//like std::polar(), but returns a point instead of an std::complex
point polar_point(double r, double theta) {
  return point(r * cos(theta), r * sin(theta));
}

//angle of segment (0, 0) to p, relative (CCW) to the +'ve x-axis in radians
double polar_angle(const point & p) {
  double t = p.arg();
  return t < 0 ? t + 2 * PI : t;
}

//angle of line segment ab relative (CCW) to the +'ve x-axis in radians
double angle_between(const point & a, const point & b) {
  double t = atan2(a.cross(b), a.dot(b)); //or, t = b.arg() - a.arg()
  return t < 0 ? t + 2 * PI : t;
}

//the minimum angle between two lines in the range [0, PI/2]
double angle_between(const line & l1, const line & l2) {
  if (l1.parallel(l2)) return 0; //parallel or equivalent lines
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
int turn(const point & a, const point & o, const point & b) {
  double c = cross(o, a, b);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

//intersection of line l1 and line l2
//returns: -1 if lines do not intersect,
//          0 if there is exactly one intersection point, or
//         +1 if there are infinite intersection
//in the 2nd case, the intersection point is optionally stored into p
int intersection(const line & l1, const line & l2, point * p = 0) {
  if (l1.parallel(l2)) return (l1 == l2) ? 1 : -1;
  if (p != 0) {
    p->x = (l1.b * l1.c - l1.b * l2.c) / (l2.a * l1.b - l1.a * l2.b);
    if (!EQ(l1.b, 0)) p->y = -(l1.a * p->x + l1.c) / l1.b;
    else p->y = -(l2.a * p->x + l2.c) / l2.b;
  }
  return 0;
}

//Line Segment Intersection (http://stackoverflow.com/a/565282)

//should we consider barely touching segments an intersection?
const bool TOUCH_IS_INTERSECT = true;

//does [l, h] contain m?
//precondition: l <= h
bool contain(double l, double m, double h) {
  if (TOUCH_IS_INTERSECT) return LE(l, m) && LE(m, h);
  return LT(l, m) && LT(m, h);
}

//does [l1, h1] overlap with [l2, h2]?
//precondition: l1 <= h1 and l2 <= h2
bool overlap(double l1, double h1, double l2, double h2) {
  if (TOUCH_IS_INTERSECT) return LE(l1, h2) && LE(l2, h1);
  return LT(l1, h2) && LT(l2, h1);
}

//intersection of line segment ab with line segment cd
//returns: -1 if segments do not intersect,
//          0 if there is exactly one intersection point
//         +1 if the intersection is another line segment
//In case 2, the intersection point is stored into p
//In case 3, the intersection segment is stored into p and q
int intersection(const point & a, const point & b,
                 const point & c, const point & d,
                 point * p = 0, point * q = 0) {
  point ab(b - a), ac(c - a), cd(d - c);
  double c1 = ab.cross(cd), c2 = ac.cross(ab);
  if (EQ(c1, 0) && EQ(c2, 0)) { //collinear
    double t0 = ac.dot(ab) / ab.dot(ab);
    double t1 = t0 + cd.dot(ab) / ab.dot(ab);
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
    *p = a + t * ab;
    return 0; //non-parallel with one intersection
  }
  return -1; //non-parallel with no intersections
}

//determins the point on line L that is closest to point p
//this always lies on the line through p perpendicular to l.
point closest_point(const line & l, const point & p) {
  if (EQ(l.a, 0)) return point(p.x, -l.c); //horizontal line
  if (EQ(l.b, 0)) return point(-l.c, p.y); //vertical line
  line perp = l.perpendicular(p);
  point res;
  intersection(l, perp, &res);
  return res;
}

//determines the point on segment ab closest to point p
point closest_point(const point & a, const point & b, const point & p) {
  if (a == b) return a;
  point ap(p - a), ab(b - a);
  double t = ap.dot(ab) / ab.norm();
  if (t <= 0) return a;
  if (t >= 1) return b;
  return point(a.x + t * ab.x, a.y + t * ab.y);
}

/*** Polygons ***/

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
double triangle_area_sides(double a, double b, double c) {
  double s = (a + b + c) / 2.0;
  return sqrt(s * (s - a) * (s - b) * (s - c));
}

//triangle area from its medians (a median is a line segment
//joining a vertex to the midpoint of the opposing side)
double triangle_area_medians(double m1, double m2, double m3) {
  return 4.0 * triangle_area_sides(m1, m2, m3) / 3.0;
}

//triangle area from its altitudes (an altitude is the shortest line
//segment between a vertex and its possibly extended opposite side)
double triangle_area_altitudes(double h1, double h2, double h3) {
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
bool point_in_rectangle(const point & p, double x, double y,
                                         double w, double h) {
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

//sorting points of a polygon clockwise around a specified center point
//this could be the arithmetic average of all the points (centroid)
//note: different center choices will yield different polygons

//finds the centroid from a range [lo, hi) of points in O(N)
template<class It> point centroid(It lo, It hi) {
  point res(0, 0);
  int points = 0;
  while (lo != hi) {
    res += *(lo++);
    points++;
  }
  return res / (double)points;
}

point ctr;

//just define your own ctr and call std::sort(lo, hi, cw_comp)
bool cw_comp(const point & a, const point & b) {
  if (GE(a.x - ctr.x, 0) && LT(b.x - ctr.x, 0)) return true;
  if (LT(a.x - ctr.x, 0) && GE(b.x - ctr.x, 0)) return false;
  if (EQ(a.x - ctr.x, 0) && EQ(b.x - ctr.x, 0)) {
    if (GE(a.y - ctr.y, 0) || GE(b.y - ctr.y, 0))
      return a.y > b.y;
    return b.y > a.y;
  }
  double det = cross(ctr, a, b);
  if (EQ(det, 0)) return (a - ctr).norm() > (b - ctr).norm();
  return det < 0;
}

bool ccw_comp(const point & a, const point & b) {
  return cw_comp(b, a);
}

//convex hull from a range [lo, hi) of points
//monotone chain in O(n log n) to find hull points in CW order
//notes: the range of input points will be sorted lexicographically
//       change GE comparisons to LE to produce hull points in CCW order
//       the last point in the returned list is the same as the first
template<class It> std::vector<point> convex_hull(It lo, It hi) {
  int k = 0;
  std::vector<point> res(2 * (int)(hi - lo));
  std::sort(lo, hi); //compare by x, then by y if x-values are equal
  for (It it = lo; it != hi; ++it) {
    while (k >= 2 && GE(cross(res[k - 2], res[k - 1], *it), 0)) k--;
    res[k++] = *it;
  }
  int t = k + 1;
  //for (i = n - 2; i >= 0; i--)
  for (It it = hi - 2; it != lo - 1; --it) {
    while (k >= t && GE(cross(res[k - 2], res[k - 1], *it), 0)) k--;
    res[k++] = *it;
  }
  res.resize(k); //use k - 1 if you don't want a repeated point
  return res;
}

//return whether point p is in polygon specified by range [lo, hi) in O(n)
//[lo, hi) must be iterators to the polygon in clockwise order
template<class It> bool point_in_polygon(const point & p, It lo, It hi) {
  int cnt = 0;
  for (It i = lo, j = hi - 1; i != hi; j = i++) {
    if (EQ(i->y, p.y) && (EQ(i->x, p.x) ||
                          EQ(j->y, p.y) && (LE(i->x, p.x) || LE(j->x, p.x))))
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
#include <iostream>
using namespace std;

int main() {
#define pt point
  pt a(0, 4);
  cout << a.normalize().abs() << "\n";                    //1
  cout << polar_point(5, 90 * DEG) << "\n";               //(0,5)
  cout << polar_angle(pt(-10, -10)) * RAD << "\n";        //225
  cout << a.rotateCW(pt(1, 1), 90 * DEG) << "\n";         //(4,2)
  cout << a.rotateCCW(pt(2, 2), 90 * DEG) << "\n";        //(0,0)
  cout << a.reflect(pt(-2, 0), pt(5, 0)) << "\n";         //(0,-4)
  cout << dist(pt(-10, 0), pt(5, 6)) << "\n";             //16.1555
  cout << dist(pt(-2, 4), pt(0, 3), pt(3, 0)) << "\n";    //2.23607
  cout << dist(pt(-2, 4), line(1, 1, -3)) << "\n";        //0.707107
  cout << angle_between(pt(10, 100), pt(-5, 12)) << "\n"; //0.49446
  cout << turn(pt(1, 1), pt(1, 3), pt(0, 3)) << "\n";     //-1

  line l(2, -5, -8);
  line para = line(2, -5, -8).parallel(pt(-6, -2));
  line perp = line(2, -5, -8).perpendicular(pt(-6, -2));
  cout << (l.parallel(para) && l.perpendicular(perp)) << "\n"; //1
  cout << l.slope() << "\n";                                   //0.4
  cout << para << "\n";                                        //-0.4x+1y-0.4=0
  cout << perp << "\n";                                        //2.5x+1y+17=0
  cout << angle_between(l, perp) * RAD << "\n";                //90

  pt r;
  cout << intersection(line(-1, 1, 0), line(1, 1, -3), &r) << "\n"; //0
  cout << r << "\n";                                                //(1.5,1.5)

  //tests for segment intersection (examples in order from link below)
  //http://martin-thoma.com/how-to-check-if-two-line-segments-intersect/
  {
#define test(a,b,c,d,e,f,g,h) intersection(pt(a,b),pt(c,d),pt(e,f),pt(g,h),&p,&q)
    pt p, q;
    //intersection is a point
    assert(0 == test(-4,0,4,0,0,-4,0,4));   cout << p << "\n"; //(0,0)
    assert(0 == test(0,0,10,10,2,2,16,4));  cout << p << "\n"; //(2,2)
    assert(0 == test(-2,2,-2,-2,-2,0,0,0)); cout << p << "\n"; //(-2,0)
    assert(0 == test(0,4,4,4,4,0,4,8));     cout << p << "\n"; //(4,4)

    //intersection is a segment
    assert(1 == test(10,10,0,0,2,2,6,6));
    cout << p << " " << q << "\n";          //(2,2) (6,6)
    assert(1 == test(6,8,14,-2,14,-2,6,8));
    cout << p << " " << q << "\n";          //(6,8) (14,-2)

    //no intersection
    assert(-1 == test(6,8,8,10,12,12,4,4));
    assert(-1 == test(-4,2,-8,8,0,0,-4,6));
    assert(-1 == test(4,4,4,6,0,2,0,0));
    assert(-1 == test(4,4,6,4,0,2,0,0));
    assert(-1 == test(-2,-2,4,4,10,10,6,6));
    assert(-1 == test(0,0,2,2,4,0,1,4));
    assert(-1 == test(2,2,2,8,4,4,6,4));
    assert(-1 == test(4,2,4,4,0,8,10,0));
  }

  cout << closest_point(line(-1,-1,5), pt(0,0)) << "\n";      //(2.5,2.5)
  cout << closest_point(line(1,0,-3), pt(0,0)) << "\n";       //(3,0)
  cout << closest_point(line(0,1,-3), pt(0,0)) << "\n";       //(0,3)

  cout << closest_point(pt(3,0), pt(3,3), pt(0,0)) << "\n";   //(3,0)
  cout << closest_point(pt(2,-1), pt(4,-1), pt(0,0)) << "\n"; //(2,-1)
  cout << closest_point(pt(2,-1), pt(4,-1), pt(5,0)) << "\n"; //(4,-1)

  assert(point_in_triangle(pt(0,0), pt(-1,0), pt(0,-2), pt(4,0)));
  assert(!point_in_triangle(pt(0,1), pt(-1,0), pt(0,-2), pt(4,0)));
  assert(point_in_triangle(pt(-2.44, 0.82), pt(-1,0), pt(-3,1), pt(4,0)));
  assert(!point_in_triangle(pt(-2.44, 0.7), pt(-1,0), pt(-3,1), pt(4,0)));

  assert(point_in_rectangle(pt(0,-1), 0, -3, 3, 2));
  assert(point_in_rectangle(pt(2,-2), 3, -3, -3, 2));
  assert(!point_in_rectangle(pt(0,0), 3, -1, -3, -2));
  assert(point_in_rectangle(pt(2,-2), pt(3,-3), pt(0,-1)));
  assert(!point_in_rectangle(pt(-1,-2), pt(3,-3), pt(0,-1)));

  assert(6 == triangle_area(pt(0,-1), pt(4,-1), pt(0,-4)));
  assert(6 == triangle_area_sides(3, 4, 5));
  assert(EQ(6, triangle_area_medians(3.605551275, 2.5, 4.272001873)));
  assert(6 == triangle_area_altitudes(3, 4, 2.4));


  return 0;
}
