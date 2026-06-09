/*

Angle calculations in two dimensions. All operations are inherently floating-point (`atan2`, `acos`,
trigonometry), so these functions use the local double-coordinate `PointD` type. Convert integer
points to `PointD` when asking for angles. The constants `DEG` and `RAD` may be used as multipliers
to convert between degrees and radians. For example, if $t$ is a value in degrees, then
$t \cdot$ `DEG` is the equivalent angle in radians; if $t$ is in radians, then $t \cdot$ `RAD` is
the equivalent angle in degrees.

- `reduce_deg(t)` takes an angle `t` degrees and returns an equivalent angle in the range $[0, 360)$
  degrees. E.g. $-630$ becomes $90$.
- `reduce_rad(t)` takes an angle `t` radians and returns an equivalent angle in the range $[0,
  2\pi)$ radians. E.g. $720.5$ becomes $0.5$.
- `polar_point(r, t)` returns a two-dimensional Cartesian point given radius `r` and angle `t`
  radians in polar coordinates (see `std::polar()`).
- `polar_angle(p)` returns the angle in radians of the line segment from $(0, 0)$ to point `p`,
  relative counterclockwise to the positive $x$-axis.
- `angle(a, o, b)` returns the smallest angle in radians formed by the points `a`, `o`, `b` with
  vertex at point `o`.
- `angle_between(a, b)` returns the angle in radians of segment from point `a` to point `b`,
  relative counterclockwise to the positive $x$-axis.
- `angle_between(a1, b1, a2, b2)` returns the smaller angle in radians between two lines
  $a_1 x + b_1 y + c_1 = 0$ and $a_2 x + b_2 y + c_2 = 0$, limited to $[0, \pi / 2]$.
- `cross(a, b, o)` returns the magnitude (Euclidean norm) of the three-dimensional cross product
  between points `a` and `b` where the `z`-component is implicitly zero and the origin is implicitly
  shifted to point `o`. This operation is also equal to double the signed area of the triangle from
  these three points.
- `turn(a, o, b)` returns $-1$ if the path `a` $\to$ `o` $\to$ `b` forms a left turn on the plane, 0
  if the path forms a straight line segment, or 1 if it forms a right turn.

Time Complexity:
- O(1) for all operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>

const double EPS = 1e-9;
const double PI = acos(-1.0);
const double DEG = PI / 180;
const double RAD = 180 / PI;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

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
  if (t < -2 * PI) {
    return reduce_rad(fmod(t, 2 * PI));
  }
  if (t < 0) {
    return t + 2 * PI;
  }
  return (t >= 2 * PI) ? fmod(t, 2 * PI) : t;
}

template<class OutPt>
OutPt polar_point(double r, double t) {
  return OutPt(r * cos(t), r * sin(t));
}

template<class Pt>
double polar_angle(const Pt &p) {
  double t = atan2((double)p.y, (double)p.x);
  return (t < 0) ? (t + 2 * PI) : t;
}

template<class Pt>
double angle(const Pt &a, const Pt &o, const Pt &b) {
  double ux = o.x - a.x, uy = o.y - a.y;
  double vx = o.x - b.x, vy = o.y - b.y;
  double cosine = (ux * vx + uy * vy) / (std::hypot(ux, uy) * std::hypot(vx, vy));
  return acos(std::max(-1.0, std::min(1.0, cosine)));
}

template<class Pt>
double angle_between(const Pt &a, const Pt &b) {
  double t = atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
  return (t < 0) ? (t + 2 * PI) : t;
}

double angle_between(const double &a1, const double &b1, const double &a2, const double &b2) {
  double t = atan2(a1 * b2 - a2 * b1, a1 * a2 + b1 * b2);
  if (t < 0) {
    t += PI;
  }
  return LT(PI / 2, t) ? (PI - t) : t;
}

template<class Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o = Pt(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

// Built on cross(a, b, o), whose sign is the opposite of the path-direction cross product, so a
// positive cross here is a right turn: this returns +1 for right/clockwise (NOT the more common
// +1 = counter-clockwise convention), -1 for left/counter-clockwise, and 0 if collinear.
template<class Pt>
int turn(const Pt &a, const Pt &o, const Pt &b) {
  auto c = cross(a, b, o);
  return LT(c, 0) ? -1 : (LT(0, c) ? 1 : 0);
}

/*** Example Usage ***/

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
};

#include <cassert>

int main() {
  assert(EQ(123, reduce_deg(-8 * 360 + 123)));
  assert(EQ(1.2345, reduce_rad(2 * PI * 8 + 1.2345)));
  assert(polar_point<Point>(4, PI) == Point(-4, 0));
  assert(polar_point<Point>(4, -PI / 2) == Point(0, -4));
  assert(EQ(45, polar_angle(Point(5, 5)) * RAD));
  assert(EQ(135 * DEG, polar_angle(Point(-4, 4))));
  assert(EQ(90 * DEG, angle(Point(5, 0), Point(0, 5), Point(-5, 0))));
  assert(EQ(225 * DEG, angle_between(Point(0, 5), Point(5, -5))));
  assert(-1 == cross(Point(0, 1), Point(1, 0), Point(0, 0)));
  assert(1 == turn(Point(0, 1), Point(0, 0), Point(-5, -5)));
  return 0;
}
