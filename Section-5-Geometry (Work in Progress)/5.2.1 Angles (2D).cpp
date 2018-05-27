/*

Angle calculations in two-dimensions. The constants DEG and RAD may be used as
multipliers to convert between degrees and radians. For example, if t is a value
in radians, then t*DEG is the equivalent angle in degrees.

- reduce_deg(t) takes an angle t degrees and returns an equivalent angle in the
  range [0, 360) degrees. E.g. -630 becomes 90.
- reduce_rad(t) takes an angle t radians and returns an equivalent angle in the
  range [0, 360) radians. E.g. 720.5 becomes 0.5.
- polar_point(r, t) returns a two-dimensional Cartesian point given radius r and
  angle t radians in polar coordinates (see std::polar()).
- polar_angle(p) returns the angle in radians of the line segment from (0, 0) to
  point p, relative counterclockwise to the positive x-axis.
- angle(a, o, b) returns the smallest angle in radians formed by the points a,
  o, b with vertex at point o.
- angle_between(a, b) returns the angle in radians of segment from point a to
  point b, relative counterclockwise to the positive x-axis.
- angle between(a1, b1, a2, b2) returns the smaller angle in radians between
  two lines a1*x + b1*y + c1 = 0 and a2*x + b2*y + c2 = 0, limited to [0, PI/2].
- cross(a, b, o) returns the magnitude (Euclidean norm) of the three-dimensional
  cross product between points a and b where the z-component is implicitly zero
  and the origin is implicitly shifted to point o. This operation is also equal
  to double the signed area of the triangle from these three points.
- turn(a, o, b) returns -1 if the path a->o->b forms a left turn on the plane, 0
  if the path forms a straight line segment, or 1 if it forms a right turn.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define GT(a, b) ((a) > (b) + EPS)

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }

const double PI = acos(-1.0), DEG = PI/180, RAD = 180/PI;

double reduce_deg(double t) {
  if (t < -360) {
    return reduce_deg(fmod(t, 360));
  }
  if (t < 0) {
    return t + 360;
  }
  return (t >= 360) ? fmod(t, 360) : t;
}

double reduce_rad(double t) {
  if (t < -2*PI) {
    return reduce_rad(fmod(t, 2*PI));
  }
  if (t < 0) {
    return t + 2*PI;
  }
  return (t >= 2*PI) ? fmod(t, 2*PI) : t;
}

point polar_point(double r, double t) {
  return point(r*cos(t), r*sin(t));
}

double polar_angle(const point &p) {
  double t = atan2(p.y, p.x);
  return (t < 0) ? (t + 2*PI) : t;
}

double angle(const point & a, const point & o, const point & b) {
  point u(o.x - a.x, o.y - a.y), v(o.x - b.x, o.y - b.y);
  return acos((u.x*v.x + u.y*v.y) / (norm(u)*norm(v)));
}

double angle_between(const point &a, const point &b) {
  double t = atan2(a.x*b.y - a.y*b.x, a.x*b.x + a.y*b.y);
  return (t < 0) ? (t + 2*PI) : t;
}

double angle_between(const double &a1, const double &b1,
                     const double &a2, const double &b2) {
  double t = atan2(a1*b2 - a2*b1, a1*a2 + b1*b2);
  if (t < 0) {
    t += PI;
  }
  return GT(t, PI / 2) ? (PI - t) : t;
}

double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x)*(b.y - o.y) - (a.y - o.y)*(b.x - o.x);
}

int turn(const point &a, const point &o, const point &b) {
  double c = cross(a, b, o);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

/*** Example Usage ***/

#include <cassert>

bool EQP(const point &a, const point &b) {
  return EQ(a.x, b.x) && EQ(a.y, b.y);
}

int main() {
  assert(EQ(123, reduce_deg(-8*360 + 123)));
  assert(EQ(1.2345, reduce_rad(2*PI*8 + 1.2345)));
  assert(EQP(polar_point(4, PI), point(-4, 0)));
  assert(EQP(polar_point(4, -PI/2), point(0, -4)));
  assert(EQ(45, polar_angle(point(5, 5))*RAD));
  assert(EQ(135*DEG, polar_angle(point(-4, 4))));
  assert(EQ(90*DEG, angle(point(5, 0), point(0, 5), point(-5, 0))));
  assert(EQ(225*DEG, angle_between(point(0, 5), point(5, -5))));
  assert(-1 == cross(point(0, 1), point(1, 0), point(0, 0)));
  assert(1 == turn(point(0, 1), point(0, 0), point(-5, -5)));
  return 0;
}
