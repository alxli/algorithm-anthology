/*

Angle calculations in two dimensions. All operations are inherently floating-point (`atan2`, `acos`,
trigonometry), so these functions use the local double-coordinate `PointD` type. Convert integer
points to `PointD` when asking for angles. The constants `DEG` and `RAD` may be used as multipliers
to convert between degrees and radians. For example, if `t` is a value in degrees, then `t * DEG` is
the equivalent angle in radians; if `t` is in radians, then `t * RAD` is the equivalent angle in
degrees.

- `reduce_deg(t)` takes an angle `t` degrees and returns an equivalent angle in the range $[0, 360)$
  degrees (e.g. $-630$ becomes $90$).
- `reduce_rad(t)` takes an angle `t` radians and returns an equivalent angle in the range
  $[0, 2\pi)$ radians (e.g. $720.5$ becomes $0.5$).
- `polar_x(r, t)` and `polar_y(r, t)` return the Cartesian coordinates of the point at radius `r`
  and angle `t` radians in polar coordinates (see `std::polar()`).
- `polar_angle(p)` returns the angle in radians of the line segment from $(0, 0)$ to point `p`,
  relative counterclockwise to the positive $x$-axis.
- `angle(a, o, b)` returns the smallest angle in radians formed by the points `a`, `o`, `b` with
  vertex at point `o`. The points `a` and `b` must differ from `o`.
- `angle_between(a, b)` returns the directed counter-clockwise angle in $[0, 2\pi)$ from vector `a`
  to vector `b`, treating both points as vectors from the origin. Both vectors must be nonzero.
- `angle_between(a1, b1, a2, b2)` returns the smaller angle in radians between two lines whose
  normal vectors are (`a1`, `b1`) and (`a2`, `b2`), limited to $[0, \pi / 2]$. Both coefficient
  pairs must represent valid lines.
- `cross(a, b, o = Pt(0, 0))` returns the signed $z$-component of the three-dimensional cross
  product between points `a` and `b`, where the $z$-coordinates are implicitly zero and the origin
  is shifted to point `o`. This is also double the signed area of the triangle from these three
  points.
- `turn(a, o, b)` returns $1$ if the path `a` $\to$ `o` $\to$ `b` forms a left turn on the plane,
  $0$ if the path forms a straight line segment, or $-1$ if it forms a right turn.

Overflow warning: `cross()` and `turn()` preserve the point's coordinate type, so their products may
overflow for integral points. Use a 64-bit coordinate type for large integer coordinates.

Time Complexity:
- O(1) per operation.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <type_traits>

const double EPS = 1e-9;
const double PI = std::acos(-1.0);
const double DEG = PI / 180;
const double RAD = 180 / PI;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) == C(b) || std::fabs(C(a) - C(b)) <= EPS;
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - EPS;
  return C(a) < C(b);
}

double reduce_deg(double t) {
  if (t < -360) {
    return reduce_deg(std::fmod(t, 360));
  }
  if (t < 0) {
    return t + 360;
  }
  return (t >= 360) ? std::fmod(t, 360) : t;
}

double reduce_rad(double t) {
  if (t < -2 * PI) {
    return reduce_rad(std::fmod(t, 2 * PI));
  }
  if (t < 0) {
    return t + 2 * PI;
  }
  return (t >= 2 * PI) ? std::fmod(t, 2 * PI) : t;
}

double polar_x(double r, double t) {
  return r * std::cos(t);
}

double polar_y(double r, double t) {
  return r * std::sin(t);
}

template<typename Pt>
double polar_angle(const Pt &p) {
  double t = std::atan2(static_cast<double>(p.y), static_cast<double>(p.x));
  return (t < 0) ? (t + 2 * PI) : t;
}

template<typename Pt>
double angle(const Pt &a, const Pt &o, const Pt &b) {
  double ux = o.x - a.x, uy = o.y - a.y;
  double vx = o.x - b.x, vy = o.y - b.y;
  double cosine = (ux * vx + uy * vy) / (std::hypot(ux, uy) * std::hypot(vx, vy));
  return std::acos(std::clamp(cosine, -1.0, 1.0));
}

template<typename Pt>
double angle_between(const Pt &a, const Pt &b) {
  double t = std::atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
  return (t < 0) ? (t + 2 * PI) : t;
}

double angle_between(double a1, double b1, double a2, double b2) {
  double t = std::atan2(a1 * b2 - a2 * b1, a1 * a2 + b1 * b2);
  if (t < 0) {
    t += PI;
  }
  return LT(PI / 2, t) ? (PI - t) : t;
}

template<typename Pt>
auto cross(const Pt &a, const Pt &b, const Pt &o = Pt(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

template<typename Pt>
int turn(const Pt &a, const Pt &o, const Pt &b) {
  auto c = cross(a, b, o);
  return LT(c, 0) ? 1 : (LT(0, c) ? -1 : 0);
}

/*** Example Usage ***/

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  friend bool EQ(const Point &a, const Point &b) { return EQ(a.x, b.x) && EQ(a.y, b.y); }
};

#include <cassert>

int main() {
  assert(EQ(123, reduce_deg(-8 * 360 + 123)));
  assert(EQ(1.2345, reduce_rad(2 * PI * 8 + 1.2345)));
  assert(EQ(-4, polar_x(4, PI)) && EQ(0, polar_y(4, PI)));
  assert(EQ(0, polar_x(4, -PI / 2)) && EQ(-4, polar_y(4, -PI / 2)));
  assert(EQ(45, polar_angle(Point(5, 5)) * RAD));
  assert(EQ(135 * DEG, polar_angle(Point(-4, 4))));
  assert(EQ(90 * DEG, angle(Point(5, 0), Point(0, 5), Point(-5, 0))));
  assert(EQ(225 * DEG, angle_between(Point(0, 5), Point(5, -5))));
  assert(-1 == cross(Point(0, 1), Point(1, 0), Point(0, 0)));
  assert(-1 == turn(Point(0, 1), Point(0, 0), Point(-5, -5)));
  return 0;
}
