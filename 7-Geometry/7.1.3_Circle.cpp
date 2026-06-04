/*

A circle in two dimensions supporting epsilon comparisons. The circle centered at $(h, k)$ is
represented by the relation $(x - h)^2 + (y - k)^2 = r^2$, where the radius $r$ is normalized to a
non-negative number. Operations include constructing a circle from a line segment, constructing a
circumcircle, accessing `center()`, checking if a point falls inside the circle with `contains()` or
on its edge with `on_edge()`, and constructing an incircle with `incircle()`.

Time Complexity:
- O(1) per call to the constructors and all other operations.

Space Complexity:
- O(1) for storage of the circle.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <ostream>
#include <stdexcept>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LE(a, b) ((a) <= (b) + EPS)
#define LT(a, b) ((a) < (b) - EPS)

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

struct Circle {
  double h, k, r;

  Circle() : h(0), k(0), r(0) {}
  explicit Circle(double r) : h(0), k(0), r(fabs(r)) {}
  Circle(const Point &o, double r) : h(o.x), k(o.y), r(fabs(r)) {}
  Circle(double h, double k, double r) : h(h), k(k), r(fabs(r)) {}

  // Circle with the line segment ab as a diameter.
  Circle(const Point &a, const Point &b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = norm(Point(a.x - h, a.y - k));
  }

  // Circumcircle of three points.
  Circle(const Point &a, const Point &b, const Point &c) {
    double an = sqnorm(Point(b.x - c.x, b.y - c.y));
    double bn = sqnorm(Point(a.x - c.x, a.y - c.y));
    double cn = sqnorm(Point(a.x - b.x, a.y - b.y));
    double wa = an * (bn + cn - an);
    double wb = bn * (an + cn - bn);
    double wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circumcircle from collinear points.");
    }
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = norm(Point(a.x - h, a.y - k));
  }

  // Circle of radius r that contains points a and b. In the general case, there will be two
  // possible circles and only one is chosen arbitrarily. However if the diameter is equal to
  // dist(a, b) = 2*r, then there is only one possible center. If points a and b are identical, then
  // there are infinite circles. If the points are too far away relative to the radius, then there
  // is no possible Circle. In the latter two cases, an exception is thrown.
  Circle(const Point &a, const Point &b, double r) : r(fabs(r)) {
    if (LE(r, 0) && a == b) {  // Circle with zero area.
      h = a.x;
      k = a.y;
      return;
    }
    double d = norm(Point(b.x - a.x, b.y - a.y));
    if (EQ(d, 0)) {
      throw std::runtime_error("Identical points, infinite circles.");
    }
    if (LT(r * 2.0, d)) {
      throw std::runtime_error("Points too far away to make Circle.");
    }
    double v = sqrt(r * r - d * d / 4.0) / d;
    Point m((a.x + b.x) / 2.0, (a.y + b.y) / 2.0);
    h = m.x + v * (a.y - b.y);
    k = m.y + v * (b.x - a.x);
    // The other answer is (h, k) = (m.x - v*(a.y - b.y), m.y - v*(b.x - a.x)).
  }

  bool operator==(const Circle &c) const { return EQ(h, c.h) && EQ(k, c.k) && EQ(r, c.r); }
  bool operator!=(const Circle &c) const { return !(*this == c); }
  Point center() const { return Point(h, k); }
  bool contains(const Point &p) const { return LE(sqnorm(Point(p.x - h, p.y - k)), r * r); }
  bool on_edge(const Point &p) const { return EQ(sqnorm(Point(p.x - h, p.y - k)), r * r); }

  friend std::ostream &operator<<(std::ostream &out, const Circle &c) {
    return out << std::showpos << "(x" << -(fabs(c.h) < EPS ? 0 : c.h) << ")^2+"
               << "(y" << -(fabs(c.k) < EPS ? 0 : c.k) << ")^2" << std::noshowpos << "="
               << (fabs(c.r) < EPS ? 0 : c.r * c.r);
  }
};

// Returns the Circle inscribed inside the triangle abc.
Circle incircle(const Point &a, const Point &b, const Point &c) {
  double al = norm(Point(b.x - c.x, b.y - c.y));
  double bl = norm(Point(a.x - c.x, a.y - c.y));
  double cl = norm(Point(a.x - b.x, a.y - b.y));
  double l = al + bl + cl;
  Point p(a.x - c.x, a.y - c.y), q(b.x - c.x, b.y - c.y);
  return EQ(l, 0) ? Circle(a.x, a.y, 0)
                  : Circle(
                        (al * a.x + bl * b.x + cl * c.x) / l, (al * a.y + bl * b.y + cl * c.y) / l,
                        fabs(p.x * q.y - p.y * q.x) / l
                    );
}

/*** Example Usage ***/

#include <cassert>

int main() {
  Circle c(-2, 5, sqrt(10));
  assert(c == Circle(Point(-2, 5), sqrt(10)));
  assert(c == Circle(Point(1, 6), Point(-5, 4)));
  assert(c == Circle(Point(-3, 2), Point(-3, 8), Point(-1, 8)));
  assert(c == incircle(Point(-12, 5), Point(3, 0), Point(0, 9)));
  assert(c.contains(Point(-2, 8)) && !c.contains(Point(-2, 9)));
  assert(c.on_edge(Point(-1, 2)) && !c.on_edge(Point(-1.01, 2)));
  return 0;
}
