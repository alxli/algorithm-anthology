/*

A circle in two dimensions supporting epsilon comparisons. The circle centered at $(h, k)$ is
represented by the relation $(x - h)^2 + (y - k)^2 = r^2$, where the radius $r$ is normalized to a
non-negative number. Operations include constructing a circle from a line segment, constructing a
circumcircle, checking if a point falls inside the circle with `contains()` or on its edge with
`on_edge()`, and constructing an incircle with `incircle()`.

Circle is floating-point by nature (the radius requires a square root), so it always stores and
computes in `double`. The center of a circumcircle is rational and the radius irrational even for
integer inputs, so there is no exact integer `Circle` to store; instead, every point-accepting
constructor and predicate is templated on the point type `Pt` and only reads `.x`/ `.y`, accepting
`Point`/ `PointD`/ `PointI` from 7.1.1 or any struct with numeric `.x`/ `.y` fields.

When exact integer reasoning about a circumcircle is needed, use `in_circumcircle(a, b, c, d)`,
which tests whether `d` lies inside the circle through `a`, `b`, `c` using a determinant (no square
root) and is therefore exact for integer coordinates without ever building a `Circle`.

Time Complexity:
- O(1) per call to the constructors and all other operations.

Space Complexity:
- O(1) for storage of the circle.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <ostream>
#include <stdexcept>
#include <type_traits>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

// SFINAE guard: valid only when Pt exposes numeric .x/.y members. This keeps the templated point
// constructors from hijacking calls like Circle(double, double, double).
template<class Pt>
using if_point = decltype(std::declval<const Pt &>().x, std::declval<const Pt &>().y, void());

struct Circle {
  double h, k, r;

  Circle() : h(0), k(0), r(0) {}
  explicit Circle(double r) : h(0), k(0), r(fabs(r)) {}
  Circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  template<class Pt, class = if_point<Pt>>
  Circle(const Pt &o, double r) : h(o.x), k(o.y), r(fabs(r)) {}

  // Circle with the line segment ab as a diameter.
  template<class Pt, class = if_point<Pt>>
  Circle(const Pt &a, const Pt &b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = hypot(a.x - h, a.y - k);
  }

  // Circumcircle of three points.
  template<class Pt, class = if_point<Pt>>
  Circle(const Pt &a, const Pt &b, const Pt &c) {
    double an = (double)(b.x - c.x) * (b.x - c.x) + (double)(b.y - c.y) * (b.y - c.y);
    double bn = (double)(a.x - c.x) * (a.x - c.x) + (double)(a.y - c.y) * (a.y - c.y);
    double cn = (double)(a.x - b.x) * (a.x - b.x) + (double)(a.y - b.y) * (a.y - b.y);
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
  template<class Pt, class = if_point<Pt>>
  Circle(const Pt &a, const Pt &b, double r) : r(fabs(r)) {
    if (LE(r, 0) && EQ(a.x, b.x) && EQ(a.y, b.y)) {  // Circle with zero area.
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

  template<class Pt>
  bool contains(const Pt &p) const {
    return LE((double)(p.x - h) * (p.x - h) + (double)(p.y - k) * (p.y - k), r * r);
  }

  template<class Pt>
  bool on_edge(const Pt &p) const {
    return EQ((double)(p.x - h) * (p.x - h) + (double)(p.y - k) * (p.y - k), r * r);
  }

  friend std::ostream &operator<<(std::ostream &out, const Circle &c) {
    return out << std::showpos << "(x" << -(fabs(c.h) < EPS ? 0 : c.h) << ")^2+"
               << "(y" << -(fabs(c.k) < EPS ? 0 : c.k) << ")^2" << std::noshowpos << "="
               << (fabs(c.r) < EPS ? 0 : c.r * c.r);
  }
};

// Returns the Circle inscribed inside the triangle abc.
template<class Pt>
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
// coordinates: the result is computed in `long long` for integral inputs (watch overflow for large
// coordinates, where the determinant is degree four) and `long double` otherwise. The points a, b,
// c must not be collinear; the result does not depend on their orientation.
template<class Pt>
int in_circumcircle(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  using CoordT = decltype(a.x + a.x);
  using W = std::conditional_t<std::is_integral<CoordT>::value, long long, long double>;
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
