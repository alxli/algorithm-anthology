/*

A two-dimensional real-valued point class supporting epsilon comparisons.
Operations include element-wise arithmetic, norm, arg, dot product, cross
product, projection, rotation, and reflection. See also std::complex.

Time Complexity:
- O(1) per call to the constructor and all other operations.

Space Complexity:
- O(1) for storage of the point.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <ostream>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

struct point {
  double x, y;

  point() : x(0), y(0) {}
  point(double x, double y) : x(x), y(y) {}
  point(const point &p) : x(p.x), y(p.y) {}
  point(const std::pair<double, double> &p) : x(p.first), y(p.second) {}

  bool operator<(const point &p) const {
    return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
  }

  bool operator>(const point &p) const {
    return EQ(x, p.x) ? LT(p.y, y) : LT(p.x, x);
  }

  bool operator==(const point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const point &p) const { return !(*this == p); }
  bool operator<=(const point &p) const { return !(*this > p); }
  bool operator>=(const point &p) const { return !(*this < p); }
  point operator+(const point &p) const { return point(x + p.x, y + p.y); }
  point operator-(const point &p) const { return point(x - p.x, y - p.y); }
  point operator+(double v) const { return point(x + v, y + v); }
  point operator-(double v) const { return point(x - v, y - v); }
  point operator*(double v) const { return point(x * v, y * v); }
  point operator/(double v) const { return point(x / v, y / v); }
  point& operator+=(const point &p) { x += p.x; y += p.y; return *this; }
  point& operator-=(const point &p) { x -= p.x; y -= p.y; return *this; }
  point& operator+=(double v) { x += v; y += v; return *this; }
  point& operator-=(double v) { x -= v; y -= v; return *this; }
  point& operator*=(double v) { x *= v; y *= v; return *this; }
  point& operator/=(double v) { x /= v; y /= v; return *this; }
  friend point operator+(double v, const point &p) { return p + v; }
  friend point operator*(double v, const point &p) { return p * v; }

  double sqnorm() const { return x*x + y*y; }
  double norm() const { return sqrt(x*x + y*y); }
  double arg() const { return atan2(y, x); }
  double dot(const point &p) const { return x*p.x + y*p.y; }
  double cross(const point &p) const { return x*p.y - y*p.x; }
  double proj(const point &p) const { return dot(p) / p.norm(); }

  // Returns a proportional unit vector (p, q) = c(x, y) where p^2 + q^2 = 1.
  point normalize() const {
    return (EQ(x, 0) && EQ(y, 0)) ? point(0, 0) : (point(x, y) / norm());
  }

  // Returns (x, y) rotated 90 degrees clockwise about the origin.
  point rotate90() const { return point(-y, x); }

  // Returns (x, y) rotated t radians clockwise about the origin.
  point rotateCW(double t) const {
    return point(x*cos(t) + y*sin(t), y*cos(t) - x*sin(t));
  }

  // Returns (x, y) rotated t radians counter-clockwise about the origin.
  point rotateCCW(double t) const {
    return point(x*cos(t) - y*sin(t), x*sin(t) + y*cos(t));
  }

  // Returns (x, y) rotated t radians clockwise about point p.
  point rotateCW(const point &p, double t) const {
    return (*this - p).rotateCW(t) + p;
  }

  // Returns (x, y) rotated t radians counter-clockwise about the point p.
  point rotateCCW(const point &p, double t) const {
    return (*this - p).rotateCCW(t) + p;
  }

  // Returns (x, y) reflected across point p.
  point reflect(const point &p) const {
    return point(2*p.x - x, 2*p.y - y);
  }

  // Returns (x, y) reflected across the line containing points p and q.
  point reflect(const point &p, const point &q) const {
    if (p == q) {
      return reflect(p);
    }
    point r(*this - p), s = q - p;
    r = point(r.x*s.x + r.y*s.y, r.x*s.y - r.y*s.x) / s.sqnorm();
    r = point(r.x*s.x - r.y*s.y, r.x*s.y + r.y*s.x) + p;
    return r;
  }

  friend double sqnorm(const point &p) { return p.sqnorm(); }
  friend double norm(const point &p) { return p.norm(); }
  friend double arg(const point &p) { return p.arg(); }
  friend double dot(const point &p, const point &q) { return p.dot(q); }
  friend double cross(const point &p, const point &q) { return p.cross(q); }
  friend double proj(const point &p, const point &q) { return p.proj(q); }
  friend point normalize(const point &p) { return p.normalize(); }
  friend point rotate90(const point &p) { return p.rotate90(); }

  friend point rotateCW(const point &p, double t) {
    return p.rotateCW(t);
  }

  friend point rotateCCW(const point &p, double t) {
    return p.rotateCCW(t);
  }

  friend point rotateCW(const point &p, const point &q, double t) {
    return p.rotateCW(q, t);
  }

  friend point rotateCCW(const point &p, const point &q, double t) {
    return p.rotateCCW(q, t);
  }

  friend point reflect(const point &p, const point &q) {
    return p.reflect(q);
  }

  friend point reflect(const point &p, const point &a, const point &b) {
    return p.reflect(a, b);
  }

  friend std::ostream& operator<<(std::ostream &out, const point &p) {
    return out << "(" << (fabs(p.x) < EPS ? 0 : p.x) << ","
                      << (fabs(p.y) < EPS ? 0 : p.y) << ")";
  }
};

/*** Example Usage ***/

#include <cassert>
#define pt point

const double PI = acos(-1.0);

int main() {
  pt p(-10, 3), q;
  assert(pt(-18, 29) == p + pt(-3, 9)*6 / 2 - pt(-1, 1));
  assert(EQ(109, p.sqnorm()));
  assert(EQ(10.44030650891, p.norm()));
  assert(EQ(2.850135859112, p.arg()));
  assert(EQ(0, p.dot(pt(3, 10))));
  assert(EQ(0, p.cross(pt(10, -3))));
  assert(EQ(10, p.proj(pt(-10, 0))));
  assert(EQ(1, p.normalize().norm()));
  assert(pt(-3, -10) == p.rotate90());
  assert(pt(3, 12) == p.rotateCW(pt(1, 1), PI / 2));
  assert(pt(1, -10) == p.rotateCCW(pt(2, 2), PI / 2));
  assert(pt(10, -3) == p.reflect(pt(0, 0)));
  assert(pt(-10, -3) == p.reflect(pt(-2, 0), pt(5, 0)));
  return 0;
}
