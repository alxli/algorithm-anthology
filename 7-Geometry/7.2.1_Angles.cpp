/*

Angle calculations in two dimensions. The constants `DEG` and `RAD` may be used as multipliers to
convert between degrees and radians. For example, if $t$ is a value in radians, then $t \cdot
\text{DEG}$ is the equivalent angle in degrees.

- `reduce_deg(t)` takes an angle $t$ degrees and returns an equivalent angle in the range $[0, 360)$
  degrees. E.g. $-630$ becomes $90$.
- `reduce_rad(t)` takes an angle $t$ radians and returns an equivalent angle in the range $[0,
  2\pi)$ radians. E.g. $720.5$ becomes $0.5$.
- `polar_point(r, t)` returns a two-dimensional Cartesian point given radius $r$ and angle $t$
  radians in polar coordinates (see `std::polar()`).
- `polar_angle(p)` returns the angle in radians of the line segment from $(0, 0)$ to point $p$,
  relative counterclockwise to the positive $x$-axis.
- `angle(a, o, b)` returns the smallest angle in radians formed by the points $a$, $o$, $b$ with
  vertex at point $o$.
- `angle_between(a, b)` returns the angle in radians of segment from point $a$ to point $b$,
  relative counterclockwise to the positive $x$-axis.
- `angle_between(a1, b1, a2, b2)` returns the smaller angle in radians between two lines $a_1x +
  b_1y + c_1 = 0$ and $a_2x + b_2y + c_2 = 0$, limited to $[0, \pi / 2]$.
- `cross(a, b, o)` returns the magnitude (Euclidean norm) of the three-dimensional cross product
  between points $a$ and $b$ where the $z$-component is implicitly zero and the origin is implicitly
  shifted to point $o$. This operation is also equal to double the signed area of the triangle from
  these three points.
- `turn(a, o, b)` returns $-1$ if the path $a \to o \to b$ forms a left turn on the plane, 0 if the
  path forms a straight line segment, or 1 if it forms a right turn.

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

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double sqnorm(const Point &a) {
  return a.x * a.x + a.y * a.y;
}

double norm(const Point &a) {
  return sqrt(sqnorm(a));
}

const double PI = acos(-1.0), DEG = PI / 180, RAD = 180 / PI;

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

Point polar_point(double r, double t) {
  return Point(r * cos(t), r * sin(t));
}

double polar_angle(const Point &p) {
  double t = atan2(p.y, p.x);
  return (t < 0) ? (t + 2 * PI) : t;
}

double angle(const Point &a, const Point &o, const Point &b) {
  Point u(o.x - a.x, o.y - a.y), v(o.x - b.x, o.y - b.y);
  return acos((u.x * v.x + u.y * v.y) / (norm(u) * norm(v)));
}

double angle_between(const Point &a, const Point &b) {
  double t = atan2(a.x * b.y - a.y * b.x, a.x * b.x + a.y * b.y);
  return (t < 0) ? (t + 2 * PI) : t;
}

double angle_between(const double &a1, const double &b1, const double &a2, const double &b2) {
  double t = atan2(a1 * b2 - a2 * b1, a1 * a2 + b1 * b2);
  if (t < 0) {
    t += PI;
  }
  return GT(t, PI / 2) ? (PI - t) : t;
}

double cross(const Point &a, const Point &b, const Point &o = Point(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

int turn(const Point &a, const Point &o, const Point &b) {
  double c = cross(a, b, o);
  return LT(c, 0) ? -1 : (GT(c, 0) ? 1 : 0);
}

/*** Example Usage ***/

#include <cassert>

bool EQP(const Point &a, const Point &b) {
  return EQ(a.x, b.x) && EQ(a.y, b.y);
}

int main() {
  assert(EQ(123, reduce_deg(-8 * 360 + 123)));
  assert(EQ(1.2345, reduce_rad(2 * PI * 8 + 1.2345)));
  assert(EQP(polar_point(4, PI), Point(-4, 0)));
  assert(EQP(polar_point(4, -PI / 2), Point(0, -4)));
  assert(EQ(45, polar_angle(Point(5, 5)) * RAD));
  assert(EQ(135 * DEG, polar_angle(Point(-4, 4))));
  assert(EQ(90 * DEG, angle(Point(5, 0), Point(0, 5), Point(-5, 0))));
  assert(EQ(225 * DEG, angle_between(Point(0, 5), Point(5, -5))));
  assert(-1 == cross(Point(0, 1), Point(1, 0), Point(0, 0)));
  assert(1 == turn(Point(0, 1), Point(0, 0), Point(-5, -5)));
  return 0;
}
