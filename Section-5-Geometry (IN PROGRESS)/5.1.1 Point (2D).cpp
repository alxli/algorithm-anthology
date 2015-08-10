/*

5.1.1 - 2D Point Class

This class is very similar to std::complex, except it uses epsilon
comparisons and also supports other operations such as reflection
and rotation. In addition, this class supports many arithmetic
operations (e.g. overloaded operators for vector addition, subtraction,
multiplication, and division; dot/cross products, etc.) pertaining to
2D cartesian vectors.

All operations are O(1) in time and space.

*/

#include <cmath>   /* atan(), fabs(), sqrt() */
#include <ostream>
#include <utility> /* std::pair */

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */

struct point {

  double x, y;

  point() : x(0), y(0) {}
  point(const point & p) : x(p.x), y(p.y) {}
  point(const std::pair<double, double> & p) : x(p.first), y(p.second) {}
  point(const double & a, const double & b) : x(a), y(b) {}

  bool operator < (const point & p) const {
    return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
  }

  bool operator > (const point & p) const {
    return EQ(x, p.x) ? LT(p.y, y) : LT(p.x, x);
  }

  bool operator == (const point & p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator != (const point & p) const { return !(*this == p); }
  bool operator <= (const point & p) const { return !(*this > p); }
  bool operator >= (const point & p) const { return !(*this < p); }
  point operator + (const point & p) const { return point(x + p.x, y + p.y); }
  point operator - (const point & p) const { return point(x - p.x, y - p.y); }
  point operator + (const double & v) const { return point(x + v, y + v); }
  point operator - (const double & v) const { return point(x - v, y - v); }
  point operator * (const double & v) const { return point(x * v, y * v); }
  point operator / (const double & v) const { return point(x / v, y / v); }
  point & operator += (const point & p) { x += p.x; y += p.y; return *this; }
  point & operator -= (const point & p) { x -= p.x; y -= p.y; return *this; }
  point & operator += (const double & v) { x += v; y += v; return *this; }
  point & operator -= (const double & v) { x -= v; y -= v; return *this; }
  point & operator *= (const double & v) { x *= v; y *= v; return *this; }
  point & operator /= (const double & v) { x /= v; y /= v; return *this; }
  friend point operator + (const double & v, const point & p) { return p + v; }
  friend point operator * (const double & v, const point & p) { return p * v; }

  double norm() const { return x * x + y * y; }
  double abs() const { return sqrt(x * x + y * y); }
  double arg() const { return atan2(y, x); }
  double dot(const point & p) const { return x * p.x + y * p.y; }
  double cross(const point & p) const { return x * p.y - y * p.x; }
  double proj(const point & p) const { return dot(p) / p.abs(); } //onto p
  point rot90() const { return point(-y, x); }

  //proportional unit vector of (x, y) such that x^2 + y^2 = 1
  point normalize() const {
    return (EQ(x, 0) && EQ(y, 0)) ? point(0, 0) : (point(x, y) / abs());
  }

  //rotate t radians CW about origin
  point rotateCW(const double & t) const {
    return point(x * cos(t) + y * sin(t), y * cos(t) - x * sin(t));
  }

  //rotate t radians CCW about origin
  point rotateCCW(const double & t) const {
    return point(x * cos(t) - y * sin(t), x * sin(t) + y * cos(t));
  }

  //rotate t radians CW about point p
  point rotateCW(const point & p, const double & t) const {
    return (*this - p).rotateCW(t) + p;
  }

  //rotate t radians CCW about point p
  point rotateCCW(const point & p, const double & t) const {
    return (*this - p).rotateCCW(t) + p;
  }

  //reflect across point p
  point reflect(const point & p) const {
    return point(2 * p.x - x, 2 * p.y - y);
  }

  //reflect across the line containing points p and q
  point reflect(const point & p, const point & q) const {
    if (p == q) return reflect(p);
    point r(*this - p), s = q - p;
    r = point(r.x * s.x + r.y * s.y, r.x * s.y - r.y * s.x) / s.norm();
    r = point(r.x * s.x - r.y * s.y, r.x * s.y + r.y * s.x) + p;
    return r;
  }

  friend double norm(const point & p) { return p.norm(); }
  friend double abs(const point & p) { return p.abs(); }
  friend double arg(const point & p) { return p.arg(); }
  friend double dot(const point & p, const point & q) { return p.dot(q); }
  friend double cross(const point & p, const point & q) { return p.cross(q); }
  friend double proj(const point & p, const point & q) { return p.proj(q); }
  friend point rot90(const point & p) { return p.rot90(); }
  friend point normalize(const point & p) { return p.normalize(); }
  friend point rotateCW(const point & p, const double & t) { return p.rotateCW(t); }
  friend point rotateCCW(const point & p, const double & t) { return p.rotateCCW(t); }
  friend point rotateCW(const point & p, const point & q, const double & t) { return p.rotateCW(q, t); }
  friend point rotateCCW(const point & p, const point & q, const double & t) { return p.rotateCCW(q, t); }
  friend point reflect(const point & p, const point & q) { return p.reflect(q); }
  friend point reflect(const point & p, const point & a, const point & b) { return p.reflect(a, b); }

  friend std::ostream & operator << (std::ostream & out, const point & p) {
    out << "(";
    out << (fabs(p.x) < eps ? 0 : p.x) << ",";
    out << (fabs(p.y) < eps ? 0 : p.y) << ")";
    return out;
  }
};

/*** Example Usage ***/

#include <cassert>
#define pt point

const double PI = acos(-1.0);

int main() {
  pt p(-10, 3);
  assert(pt(-18, 29) == p + pt(-3, 9) * 6 / 2 - pt(-1, 1));
  assert(EQ(109, p.norm()));
  assert(EQ(10.44030650891, p.abs()));
  assert(EQ(2.850135859112, p.arg()));
  assert(EQ(0,  p.dot(pt(3, 10))));
  assert(EQ(0,  p.cross(pt(10, -3))));
  assert(EQ(10, p.proj(pt(-10, 0))));
  assert(EQ(1,  p.normalize().abs()));
  assert(pt(-3, -10) == p.rot90());
  assert(pt(3, 12)   == p.rotateCW(pt(1, 1), PI / 2));
  assert(pt(1, -10)  == p.rotateCCW(pt(2, 2), PI / 2));
  assert(pt(10, -3)  == p.reflect(pt(0, 0)));
  assert(pt(-10, -3) == p.reflect(pt(-2, 0), pt(5, 0)));
  return 0;
}
