/*

A 2D real-valued point class supporting epsilon comparisons. Operations include element-wise
arithmetic, `norm()`, `arg()`, `dot()`, `cross()`, `proj()`, `normalize()`, `rotate90()`,
`rotateCW()`, `rotateCCW()`, and `reflect()`. See also `std::complex`.

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

struct Point {
  double x, y;

  Point() : x(0), y(0) {}
  Point(double x, double y) : x(x), y(y) {}
  Point(const Point &p) : x(p.x), y(p.y) {}
  Point(const std::pair<double, double> &p) : x(p.first), y(p.second) {}

  // clang-format off
  bool operator<(const Point &p) const { return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x); }
  bool operator>(const Point &p) const { return EQ(x, p.x) ? LT(p.y, y) : LT(p.x, x); }
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<=(const Point &p) const { return !(*this > p); }
  bool operator>=(const Point &p) const { return !(*this < p); }
  Point operator+(const Point &p) const { return Point(x + p.x, y + p.y); }
  Point operator-(const Point &p) const { return Point(x - p.x, y - p.y); }
  Point operator+(double v) const { return Point(x + v, y + v); }
  Point operator-(double v) const { return Point(x - v, y - v); }
  Point operator*(double v) const { return Point(x * v, y * v); }
  Point operator/(double v) const { return Point(x / v, y / v); }
  Point &operator+=(const Point &p) { x += p.x; y += p.y; return *this; }
  Point &operator-=(const Point &p) { x -= p.x; y -= p.y; return *this; }
  Point &operator+=(double v) { x += v; y += v; return *this; }
  Point &operator-=(double v) { x -= v; y -= v; return *this; }
  Point &operator*=(double v) { x *= v; y *= v; return *this; }
  Point &operator/=(double v) { x /= v; y /= v; return *this; }
  friend Point operator+(double v, const Point &p) { return p + v; }
  friend Point operator*(double v, const Point &p) { return p * v; }
  // clang-format on

  double sqnorm() const { return x * x + y * y; }
  double norm() const { return sqrt(x * x + y * y); }
  double arg() const { return atan2(y, x); }
  double dot(const Point &p) const { return x * p.x + y * p.y; }
  double cross(const Point &p) const { return x * p.y - y * p.x; }
  double proj(const Point &p) const { return dot(p) / p.norm(); }

  // Returns a proportional unit vector (p, q) = c(x, y) where p^2 + q^2 = 1.
  Point normalize() const { return (EQ(x, 0) && EQ(y, 0)) ? Point(0, 0) : (Point(x, y) / norm()); }

  // Returns (x, y) rotated 90 degrees clockwise about the origin.
  Point rotate90() const { return Point(-y, x); }

  // Returns (x, y) rotated t radians clockwise about the origin.
  Point rotateCW(double t) const { return Point(x * cos(t) + y * sin(t), y * cos(t) - x * sin(t)); }

  // Returns (x, y) rotated t radians counter-clockwise about the origin.
  Point rotateCCW(double t) const {
    return Point(x * cos(t) - y * sin(t), x * sin(t) + y * cos(t));
  }

  // Returns (x, y) rotated t radians clockwise about Point p.
  Point rotateCW(const Point &p, double t) const { return (*this - p).rotateCW(t) + p; }

  // Returns (x, y) rotated t radians counter-clockwise about the Point p.
  Point rotateCCW(const Point &p, double t) const { return (*this - p).rotateCCW(t) + p; }

  // Returns (x, y) reflected across Point p.
  Point reflect(const Point &p) const { return Point(2 * p.x - x, 2 * p.y - y); }

  // Returns (x, y) reflected across the line containing Points p and q.
  Point reflect(const Point &p, const Point &q) const {
    if (p == q) {
      return reflect(p);
    }
    Point r(*this - p), s = q - p;
    r = Point(r.x * s.x + r.y * s.y, r.x * s.y - r.y * s.x) / s.sqnorm();
    r = Point(r.x * s.x - r.y * s.y, r.x * s.y + r.y * s.x) + p;
    return r;
  }

  friend double sqnorm(const Point &p) { return p.sqnorm(); }
  friend double norm(const Point &p) { return p.norm(); }
  friend double arg(const Point &p) { return p.arg(); }
  friend double dot(const Point &p, const Point &q) { return p.dot(q); }
  friend double cross(const Point &p, const Point &q) { return p.cross(q); }
  friend double proj(const Point &p, const Point &q) { return p.proj(q); }
  friend Point normalize(const Point &p) { return p.normalize(); }
  friend Point rotate90(const Point &p) { return p.rotate90(); }
  friend Point rotateCW(const Point &p, double t) { return p.rotateCW(t); }
  friend Point rotateCCW(const Point &p, double t) { return p.rotateCCW(t); }
  friend Point rotateCW(const Point &p, const Point &q, double t) { return p.rotateCW(q, t); }
  friend Point rotateCCW(const Point &p, const Point &q, double t) { return p.rotateCCW(q, t); }
  friend Point reflect(const Point &p, const Point &q) { return p.reflect(q); }
  friend Point reflect(const Point &p, const Point &a, const Point &b) { return p.reflect(a, b); }

  friend std::ostream &operator<<(std::ostream &out, const Point &p) {
    return out << "(" << (fabs(p.x) < EPS ? 0 : p.x) << "," << (fabs(p.y) < EPS ? 0 : p.y) << ")";
  }
};

/*** Example Usage ***/

#include <cassert>

const double PI = acos(-1.0);

int main() {
  Point p(-10, 3), q;
  assert(Point(-18, 29) == p + Point(-3, 9) * 6 / 2 - Point(-1, 1));
  assert(EQ(109, p.sqnorm()));
  assert(EQ(10.44030650891, p.norm()));
  assert(EQ(2.850135859112, p.arg()));
  assert(EQ(0, p.dot(Point(3, 10))));
  assert(EQ(0, p.cross(Point(10, -3))));
  assert(EQ(10, p.proj(Point(-10, 0))));
  assert(EQ(1, p.normalize().norm()));
  assert(Point(-3, -10) == p.rotate90());
  assert(Point(3, 12) == p.rotateCW(Point(1, 1), PI / 2));
  assert(Point(1, -10) == p.rotateCCW(Point(2, 2), PI / 2));
  assert(Point(10, -3) == p.reflect(Point(0, 0)));
  assert(Point(-10, -3) == p.reflect(Point(-2, 0), Point(5, 0)));
  return 0;
}
