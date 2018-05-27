/*

A circle in two dimensions supporting epsilon comparisons. The circle centered
at (h, k) is represented by the relation (x - h)^2 + (y - k)^2 = r^2, where the
radius r is normalized to a non-negative number. Operations include constructing
a circle from a line segment, constructing a circumcircle, checking if a point
falls inside the circle or on its edge, and constructing an incircle.

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

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double norm(const point &a) { return sqrt(sqnorm(a)); }

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
    r = norm(point(a.x - h, a.y - k));
  }

  // Circumcircle of three points.
  circle(const point &a, const point &b, const point &c) {
    double an = sqnorm(point(b.x - c.x, b.y - c.y));
    double bn = sqnorm(point(a.x - c.x, a.y - c.y));
    double cn = sqnorm(point(a.x - b.x, a.y - b.y));
    double wa = an*(bn + cn - an);
    double wb = bn*(an + cn - bn);
    double wc = cn*(an + bn - cn);
    double w = wa + wb + wc;
    if (EQ(w, 0)) {
      throw std::runtime_error("No circumcircle from collinear points.");
    }
    h = (wa*a.x + wb*b.x + wc*c.x)/w;
    k = (wa*a.y + wb*b.y + wc*c.y)/w;
    r = norm(point(a.x - h, a.y - k));
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
    double d = norm(point(b.x - a.x, b.y - a.y));
    if (EQ(d, 0)) {
      throw std::runtime_error("Identical points, infinite circles.");
    }
    if (LT(r*2.0, d)) {
      throw std::runtime_error("Points too far away to make circle.");
    }
    double v = sqrt(r*r - d*d/4.0) / d;
    point m((a.x + b.x)/2.0, (a.y + b.y)/2.0);
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

  bool contains(const point &p) const {
    return LE(sqnorm(point(p.x - h, p.y - k)), r*r);
  }

  bool on_edge(const point &p) const {
    return EQ(sqnorm(point(p.x - h, p.y - k)), r*r);
  }

  friend std::ostream& operator<<(std::ostream &out, const circle &c) {
    return out << std::showpos << "(x" << -(fabs(c.h) < EPS ? 0 : c.h) << ")^2+"
                               << "(y" << -(fabs(c.k) < EPS ? 0 : c.k) << ")^2"
               << std::noshowpos << "=" << (fabs(c.r) < EPS ? 0 : c.r*c.r);
  }
};

// Returns the circle inscribed inside the triangle abc.
circle incircle(const point &a, const point &b, const point &c) {
  double al = norm(point(b.x - c.x, b.y - c.y));
  double bl = norm(point(a.x - c.x, a.y - c.y));
  double cl = norm(point(a.x - b.x, a.y - b.y));
  double l = al + bl + cl;
  point p(a.x - c.x, a.y - c.y), q(b.x - c.x, b.y - c.y);
  return EQ(l, 0) ? circle(a.x, a.y, 0)
                  : circle((al*a.x + bl*b.x + cl*c.x) / l,
                           (al*a.y + bl*b.y + cl*c.y) / l,
                           fabs(p.x*q.y - p.y*q.x) / l);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  circle c(-2, 5, sqrt(10));
  assert(c == circle(point(-2, 5), sqrt(10)));
  assert(c == circle(point(1, 6), point(-5, 4)));
  assert(c == circle(point(-3, 2), point(-3, 8), point(-1, 8)));
  assert(c == incircle(point(-12, 5), point(3, 0), point(0, 9)));
  assert(c.contains(point(-2, 8)) && !c.contains(point(-2, 9)));
  assert(c.on_edge(point(-1, 2)) && !c.on_edge(point(-1.01, 2)));
  return 0;
}
