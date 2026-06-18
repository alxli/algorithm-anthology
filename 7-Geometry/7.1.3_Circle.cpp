/*

A circle in two dimensions supporting epsilon comparisons. The circle centered at $(h, k)$ is
represented by the relation $(x - h)^2 + (y - k)^2 = r^2$, where the radius $r$ is normalized to a
nonnegative number.

This implementation stores and computes circle parameters in `double`. Point-accepting
constructors and predicates are templated on the point type `Pt` and only read `.x`/`.y`, so they
accept `Point`/`PointD`/`PointI` from 7.1.1 or any struct with numeric `.x` and `.y` fields.

- `Circle()` constructs the zero-radius circle centered at the origin.
- `Circle(r)` constructs a circle of radius `abs(r)` centered at the origin.
- `Circle(h, k, r)` constructs a circle of radius `abs(r)` centered at (`h`, `k`).
- `Circle(o, r)` constructs a circle of radius `abs(r)` centered at point `o`.
- `Circle(a, b)` constructs the circle whose diameter is segment `a`-`b`.
- `Circle(a, b, c)` constructs the circumcircle through non-collinear points `a`, `b`, and `c`,
  throwing if the points are collinear.
- `Circle(a, b, r)` constructs one circle of radius `abs(r)` passing through points `a` and `b`,
  throwing if no finite circle is determined by the inputs.
- `operator==` and `operator!=` compare centers and radii using `EPS`.
- `contains(p)` returns whether point `p` lies inside or on the circle.
- `on_edge(p)` returns whether point `p` lies on the circle boundary.
- `incircle(a, b, c)` returns the circle inscribed in triangle `abc`.
- `in_circumcircle(a, b, c, d)` returns 1 if `d` lies strictly inside the circle through `a`, `b`,
  and `c`, 0 if it lies on the circle, or $-1$ if it lies outside. This has exact reasoning for
  integral points by evaluating the determinant directly without constructing a `Circle` or taking
  square roots. For integer inputs, the sign test is exact provided the chosen intermediate type
  does not overflow. The determinant has degree four in the coordinate magnitude, so use a wider
  type for large integer coordinates.

Time Complexity:
- O(1) per call to the constructors and all other operations.

Space Complexity:
- O(1) for storage of the circle.
- O(1) auxiliary for all operations.

*/
#include <cmath>
#include <cstdint>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

const double EPS = 1e-9;

// clang-format off
template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  return std::is_integral_v<C> ? C(a) == C(b) : std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
}
template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  return std::is_integral_v<C> ? C(a) < C(b) : C(a) < C(b) - static_cast<C>(EPS);
}
template<typename T, typename U> bool LE(T a, U b) { return !LT(b, a); }
// clang-format on

// SFINAE guard: valid only when Pt exposes numeric .x/.y members. This keeps the templated point
// constructors from hijacking calls like Circle(double, double, double).
template<typename Pt>
using if_point = decltype(std::declval<const Pt &>().x, std::declval<const Pt &>().y, void());

struct Circle {
  double h, k, r;

  Circle() : h(0), k(0), r(0) {}
  explicit Circle(double r) : h(0), k(0), r(fabs(r)) {}
  Circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  template<typename Pt, typename = if_point<Pt>>
  Circle(const Pt &o, double r) : h(o.x), k(o.y), r(fabs(r)) {}

  // Circle with the line segment ab as a diameter.
  template<typename Pt, typename = if_point<Pt>>
  Circle(const Pt &a, const Pt &b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = hypot(a.x - h, a.y - k);
  }

  // Circumcircle of three points.
  template<typename Pt, typename = if_point<Pt>>
  Circle(const Pt &a, const Pt &b, const Pt &c) {
    double an =
        static_cast<double>(b.x - c.x) * (b.x - c.x) + static_cast<double>(b.y - c.y) * (b.y - c.y);
    double bn =
        static_cast<double>(a.x - c.x) * (a.x - c.x) + static_cast<double>(a.y - c.y) * (a.y - c.y);
    double cn =
        static_cast<double>(a.x - b.x) * (a.x - b.x) + static_cast<double>(a.y - b.y) * (a.y - b.y);
    double wa = an * (bn + cn - an);
    double wb = bn * (an + cn - bn);
    double wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circumcircle from collinear points.");
    }
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = hypot(a.x - h, a.y - k);
  }

  // Circle of radius r that contains points a and b. In the general case, there will be two
  // possible circles and only one is chosen arbitrarily. However if the diameter is equal to
  // dist(a, b) = 2*r, then there is only one possible center. If points a and b are identical, then
  // there are infinite circles. If the points are too far away relative to the radius, then there
  // is no possible Circle. In the latter two cases, an exception is thrown.
  template<typename Pt, typename = if_point<Pt>>
  Circle(const Pt &a, const Pt &b, double r) : r(fabs(r)) {
    if (LE(r, 0) && EQ(a.x, b.x) && EQ(a.y, b.y)) {  // Zero-radius circle.
      h = a.x;
      k = a.y;
      return;
    }
    double d = hypot(b.x - a.x, b.y - a.y);
    if (EQ(d, 0)) {
      throw std::runtime_error("Identical points, infinite circles.");
    }
    if (LT(r * 2.0, d)) {
      throw std::runtime_error("Points too far away to make Circle.");
    }
    double v = sqrt(r * r - d * d / 4.0) / d;
    double mx = (a.x + b.x) / 2.0, my = (a.y + b.y) / 2.0;
    h = mx + v * (a.y - b.y);
    k = my + v * (b.x - a.x);
    // The other answer is (h, k) = (mx - v*(a.y - b.y), my - v*(b.x - a.x)).
  }

  bool operator==(const Circle &c) const { return EQ(h, c.h) && EQ(k, c.k) && EQ(r, c.r); }
  bool operator!=(const Circle &c) const { return !(*this == c); }

  template<typename Pt>
  bool contains(const Pt &p) const {
    return LE(
        static_cast<double>(p.x - h) * (p.x - h) + static_cast<double>(p.y - k) * (p.y - k), r * r
    );
  }

  template<typename Pt>
  bool on_edge(const Pt &p) const {
    return EQ(
        static_cast<double>(p.x - h) * (p.x - h) + static_cast<double>(p.y - k) * (p.y - k), r * r
    );
  }

  friend std::ostream &operator<<(std::ostream &out, const Circle &c) {
    return out << std::showpos << "(x" << -(fabs(c.h) < EPS ? 0 : c.h) << ")^2+"
               << "(y" << -(fabs(c.k) < EPS ? 0 : c.k) << ")^2" << std::noshowpos << "="
               << (fabs(c.r) < EPS ? 0 : c.r * c.r);
  }
};

// Returns the Circle inscribed inside the triangle abc.
template<typename Pt>
Circle incircle(const Pt &a, const Pt &b, const Pt &c) {
  double al = hypot(b.x - c.x, b.y - c.y);
  double bl = hypot(a.x - c.x, a.y - c.y);
  double cl = hypot(a.x - b.x, a.y - b.y);
  double l = al + bl + cl;
  double px = a.x - c.x, py = a.y - c.y, qx = b.x - c.x, qy = b.y - c.y;
  return EQ(l, 0) ? Circle(a.x, a.y, 0)
                  : Circle(
                        (al * a.x + bl * b.x + cl * c.x) / l, (al * a.y + bl * b.y + cl * c.y) / l,
                        fabs(px * qy - py * qx) / l
                    );
}

// Returns +1 if point d lies strictly inside the circle through a, b, c, 0 if d lies exactly on
// it, or -1 if d is outside. This uses a determinant (no square root), so it is exact for integer
// coordinates: the result is computed in `int64_t` for integral inputs (watch overflow for large
// coordinates, where the determinant is degree four) and `long double` otherwise. The points a, b,
// c must not be collinear; the result does not depend on their orientation.
template<typename Pt>
int in_circumcircle(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  using CoordT = decltype(a.x + a.x);
  using W = std::conditional_t<std::is_integral<CoordT>::value, int64_t, long double>;
  W adx = (W)a.x - d.x, ady = (W)a.y - d.y;
  W bdx = (W)b.x - d.x, bdy = (W)b.y - d.y;
  W cdx = (W)c.x - d.x, cdy = (W)c.y - d.y;
  W det = (adx * adx + ady * ady) * (bdx * cdy - cdx * bdy) +
          (bdx * bdx + bdy * bdy) * (cdx * ady - adx * cdy) +
          (cdx * cdx + cdy * cdy) * (adx * bdy - bdx * ady);
  W orient = ((W)b.x - a.x) * ((W)c.y - a.y) - ((W)b.y - a.y) * ((W)c.x - a.x);
  W val = orient > 0 ? det : -det;
  return val > 0 ? 1 : (val < 0 ? -1 : 0);
}

/*** Example Usage ***/

#include <cassert>

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
};

int main() {
  Circle c(-2, 5, sqrt(10));
  assert(c == Circle(Point(-2, 5), sqrt(10)));
  assert(c == Circle(Point(1, 6), Point(-5, 4)));
  assert(c == Circle(Point(-3, 2), Point(-3, 8), Point(-1, 8)));
  assert(c == incircle(Point(-12, 5), Point(3, 0), Point(0, 9)));
  assert(c.contains(Point(-2, 8)) && !c.contains(Point(-2, 9)));
  assert(c.on_edge(Point(-1, 2)) && !c.on_edge(Point(-1.01, 2)));

  // Integer-coordinate points are accepted; the Circle is computed in double.
  assert(c == Circle(PointI(1, 6), PointI(-5, 4)));
  assert(c == Circle(PointI(-3, 2), PointI(-3, 8), PointI(-1, 8)));
  assert(c == incircle(PointI(-12, 5), PointI(3, 0), PointI(0, 9)));
  assert(c.contains(PointI(-2, 8)) && !c.contains(PointI(-2, 9)));
  assert(c.on_edge(PointI(-1, 2)));

  // Exact integer in-circle predicate: unit circle through (1,0), (0,1), (-1,0).
  assert(in_circumcircle(PointI(1, 0), PointI(0, 1), PointI(-1, 0), PointI(0, 0)) == 1);  // inside
  assert(
      in_circumcircle(PointI(1, 0), PointI(0, 1), PointI(-1, 0), PointI(2, 0)) == -1
  );  // outside
  assert(
      in_circumcircle(PointI(1, 0), PointI(0, 1), PointI(-1, 0), PointI(0, -1)) == 0
  );  // on edge
  // Orientation-independent: reversing a, b, c gives the same answer.
  assert(in_circumcircle(PointI(-1, 0), PointI(0, 1), PointI(1, 0), PointI(0, 0)) == 1);
  return 0;
}
