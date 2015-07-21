#include <cmath>     /* fabs(), sqrt() */
#include <ostream>
#include <stdexcept> /* std::runtime_error() */
#include <utility>   /* std::pair */

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */

typedef std::pair<double, double> point;
#define x first
#define y second

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double abs(const point & a) { return sqrt(norm(a)); }

struct circle {

  double h, k, r;

  circle(): h(0), k(0), r(0) {}
  circle(const double & R): h(0), k(0), r(fabs(R)) {}
  circle(const point & o, const double & R): h(o.x), k(o.y), r(fabs(R)) {}
  circle(const double & H, const double & K, const double & R):
    h(H), k(K), r(fabs(R)) {}

  //circumcircle with the diameter equal to the distance from a to b
  circle(const point & a, const point & b) {
    h = (a.x + b.x) / 2.0;
    k = (a.y + b.y) / 2.0;
    r = abs(point(a.x - h, a.y - k));
  }

  //circumcircle of 3 points - throws exception if abc are collinear/equal
  circle(const point & a, const point & b, const point & c) {
    double an = norm(point(b.x - c.x, b.y - c.y));
    double bn = norm(point(a.x - c.x, a.y - c.y));
    double cn = norm(point(a.x - b.x, a.y - b.y));
    double wa = an * (bn + cn - an);
    double wb = bn * (an + cn - bn);
    double wc = cn * (an + bn - cn);
    double w = wa + wb + wc;
    if (fabs(w) < eps)
      throw std::runtime_error("No circle from collinear points.");
    h = (wa * a.x + wb * b.x + wc * c.x) / w;
    k = (wa * a.y + wb * b.y + wc * c.y) / w;
    r = abs(point(a.x - h, a.y - k));
  }

  //circle from 2 points and a radius - many possible edge cases!
  //in the "normal" case, there will be 2 possible circles, one
  //centered at (h1, k1) and the other (h2, k2). Only one is used.
  //Note that (h1, k1) equals (h2, k2) if dist(a, b) = 2*r = d
  circle(const point & a, const point & b, const double & R) {
    r = fabs(R);
    if (LE(r, 0) && a == b) { //circle is a point
      h = a.x;
      k = a.y;
      return;
    }
    double d = abs(point(b.x - a.x, b.y - a.y));
    if (EQ(d, 0))
      throw std::runtime_error("Identical points, infinite circles.");
    if (GT(d, r * 2.0))
      throw std::runtime_error("Points too far away to make circle.");
    double v = sqrt(r * r - d * d / 4.0) / d;
    point m((a.x + b.x) / 2.0, (a.y + b.y) / 2.0);
    h = m.x + (a.y - b.y) * v;
    k = m.y + (b.x - a.x) * v;
    //other answer is (h, k) = (m.x-(a.y-b.y)*v, m.y-(b.x-a.x)*v)
  }

  bool operator == (const circle & c) const {
    return EQ(h, c.h) && EQ(k, c.k) && EQ(r, c.r);
  }

  bool operator != (const circle & c) const {
    return !(*this == c);
  }

  bool contains(const point & p) const {
    return LE(norm(point(p.x - h, p.y - k)), r * r);
  }

  bool on_edge(const point & p) const {
    return EQ(norm(point(p.x - h, p.y - k)), r * r);
  }

  point center() const {
    return point(h, k);
  }

  friend std::ostream & operator << (std::ostream & out, const circle & c) {
    out << std::showpos;
    out << "(x" << -(fabs(c.h) < eps ? 0 : c.h) << ")^2+";
    out << "(y" << -(fabs(c.k) < eps ? 0 : c.k) << ")^2";
    out << std::noshowpos;
    out << "=" << (fabs(c.r) < eps ? 0 : c.r * c.r);
    return out;
  }
};

//circle inscribed within points a, b, and c
circle incircle(const point & a, const point & b, const point & c) {
  double al = abs(point(b.x - c.x, b.y - c.y));
  double bl = abs(point(a.x - c.x, a.y - c.y));
  double cl = abs(point(a.x - b.x, a.y - b.y));
  double p = al + bl + cl;
  if (EQ(p, 0)) return circle(a.x, a.y, 0);
  circle res;
  res.h = (al * a.x + bl * b.x + cl * c.x) / p;
  res.k = (al * a.y + bl * b.y + cl * c.y) / p;
  res.r = fabs((a.x - c.x) * (b.y - c.y) - (a.y - c.y) * (b.x - c.x)) / p;
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  circle c(-2, 5, sqrt(10)); //(x+2)^2+(y-5)^2=10
  assert(c == circle(point(-2, 5), sqrt(10)));
  assert(c == circle(point(1, 6), point(-5, 4)));
  assert(c == circle(point(-3, 2), point(-3, 8), point(-1, 8)));
  assert(c == incircle(point(-12, 5), point(3, 0), point(0, 9)));
  assert(c.contains(point(-2, 8)) && !c.contains(point(-2, 9)));
  assert(c.on_edge(point(-1, 2)) && !c.on_edge(point(-1.01, 2)));
  return 0;
}
