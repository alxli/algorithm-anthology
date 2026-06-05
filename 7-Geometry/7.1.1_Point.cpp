/*

A 2D point class template supporting epsilon comparisons. The coordinate type T defaults to
`double`. Integer types (e.g. `int`) fully support all exact operations. Floating-point-only
operations (`norm`, `arg`, `normalize`, `rotateCW/CCW`, `reflect` across a line) return coordinates
of type `fp_type<T>`, which is `double` when T is integral and T itself otherwise.

Exact operations (return `Point<T>` or T, no precision lost for integers):
- element-wise arithmetic, `dot()`, `cross()`, `sqnorm()`, cardinal rotations, `reflect(point)`,
  comparisons.

Floating-point-only operations (return `Point<fp_type<T>>` or `fp_type<T>`):
- `norm()`, `arg()`, `proj()`, `normalize()`, `rotateCW()`, `rotateCCW()`, `reflect(line)`.
- `operator /` also promotes to `fp_type<T>`.

Implicit conversion from `Point<U>` to `Point<T>` is provided when U is integral and T is
floating-point, so `PointI` can be passed wherever `PointD` is expected.

Type aliases:
- `PointI  = Point<int>`: exact integer geometry
- `PointD  = Point<double>`: standard floating-point (default)
- `PointLD = Point<long double>`: extra precision

See also `std::complex`.

Time Complexity:
- O(1) per call to the constructor and all other operations.

Space Complexity:
- O(1) for storage of the point.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <ostream>
#include <type_traits>
#include <utility>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)

template<typename T>
struct point {
  // fp_t is T for floating-point types, double for integral types.
  using fp_t = std::conditional_t<std::is_integral<T>::value, double, T>;

  T x, y;

  point() : x(0), y(0) {}
  point(T x, T y) : x(x), y(y) {}
  explicit point(const std::pair<T, T> &p) : x(p.first), y(p.second) {}

  // Implicit conversion from integral point to floating-point point.
  template<
      typename U,
      typename = std::enable_if_t<std::is_integral<U>::value && std::is_floating_point<T>::value>>
  point(const point<U> &p) : x(static_cast<T>(p.x)), y(static_cast<T>(p.y)) {}

  bool operator==(const point &p) const {
    if (std::is_integral<T>::value) {
      return x == p.x && y == p.y;
    }
    return EQ(x, p.x) && EQ(y, p.y);
  }

  bool operator!=(const point &p) const { return !(*this == p); }

  bool operator<(const point &p) const {
    if (std::is_integral<T>::value) {
      return x != p.x ? x < p.x : y < p.y;
    }
    return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
  }

  bool operator>(const point &p) const { return p < *this; }
  bool operator<=(const point &p) const { return !(*this > p); }
  bool operator>=(const point &p) const { return !(*this < p); }
  point operator+(const point &p) const { return {x + p.x, y + p.y}; }
  point operator-(const point &p) const { return {x - p.x, y - p.y}; }
  point operator+(T v) const { return {x + v, y + v}; }
  point operator-(T v) const { return {x - v, y - v}; }
  point operator*(T v) const { return {x * v, y * v}; }
  // Division always promotes to fp_t to avoid integer truncation.
  point<fp_t> operator/(fp_t v) const { return {(fp_t)x / v, (fp_t)y / v}; }

  // clang-format off
  point &operator+=(const point &p) { x += p.x; y += p.y; return *this; }
  point &operator-=(const point &p) { x -= p.x; y -= p.y; return *this; }
  point &operator+=(T v) { x += v; y += v; return *this; }
  point &operator-=(T v) { x -= v; y -= v; return *this; }
  point &operator*=(T v) { x *= v; y *= v; return *this; }
  friend point operator+(T v, const point &p) { return p + v; }
  friend point operator*(T v, const point &p) { return p * v; }
  // clang-format on

  // --- Exact operations: return T or point<T>, work for any coordinate type ---

  T sqnorm() const { return x * x + y * y; }
  T dot(const point &p) const { return x * p.x + y * p.y; }
  T cross(const point &p) const { return x * p.y - y * p.x; }

  // --- Floating-point operations: return fp_t or point<fp_t> ---

  fp_t norm() const { return sqrt(static_cast<fp_t>(sqnorm())); }
  fp_t arg() const { return atan2(static_cast<fp_t>(y), static_cast<fp_t>(x)); }
  fp_t proj(const point &p) const { return static_cast<fp_t>(dot(p)) / p.norm(); }

  point<fp_t> normalize() const {
    fp_t n = norm();
    if (n < static_cast<fp_t>(1e-30)) {
      return {fp_t(0), fp_t(0)};
    }
    return {static_cast<fp_t>(x) / n, static_cast<fp_t>(y) / n};
  }

  // --- Cardinal rotations: exact for all coordinate types including int ---

  // Returns (x, y) rotated 90/180/270 degrees counter-clockwise about the origin.
  point rotate90() const { return {-y, x}; }
  point rotate180() const { return {-x, -y}; }
  point rotate270() const { return {y, -x}; }

  // Returns (x, y) rotated 90/180/270 degrees counter-clockwise about point p.
  point rotate90(const point &p) const { return (*this - p).rotate90() + p; }
  point rotate180(const point &p) const { return (*this - p).rotate180() + p; }
  point rotate270(const point &p) const { return (*this - p).rotate270() + p; }

  // --- Arbitrary-angle rotations: always return floating-point ---

  // Returns (x, y) rotated t radians clockwise about the origin.
  point<fp_t> rotateCW(fp_t t) const {
    fp_t fx = (fp_t)x, fy = (fp_t)y;
    return {fx * cos(t) + fy * sin(t), fy * cos(t) - fx * sin(t)};
  }

  // Returns (x, y) rotated t radians counter-clockwise about the origin.
  point<fp_t> rotateCCW(fp_t t) const {
    fp_t fx = (fp_t)x, fy = (fp_t)y;
    return {fx * cos(t) - fy * sin(t), fx * sin(t) + fy * cos(t)};
  }

  // Returns (x, y) rotated t radians clockwise about point p.
  point<fp_t> rotateCW(const point &p, fp_t t) const {
    return point<fp_t>{(fp_t)(x - p.x), (fp_t)(y - p.y)}.rotateCW(t) +
           point<fp_t>{(fp_t)p.x, (fp_t)p.y};
  }

  // Returns (x, y) rotated t radians counter-clockwise about point p.
  point<fp_t> rotateCCW(const point &p, fp_t t) const {
    return point<fp_t>{(fp_t)(x - p.x), (fp_t)(y - p.y)}.rotateCCW(t) +
           point<fp_t>{(fp_t)p.x, (fp_t)p.y};
  }

  // --- Reflections ---

  // Returns (x, y) reflected across point p. Exact for any coordinate type.
  point reflect(const point &p) const { return {2 * p.x - x, 2 * p.y - y}; }

  // Returns (x, y) reflected across the line containing points p and q.
  // Always returns floating-point coordinates.
  point<fp_t> reflect(const point &p, const point &q) const {
    point<fp_t> fp{(fp_t)p.x, (fp_t)p.y};
    if (p == q) {
      return point<fp_t>{(fp_t)x, (fp_t)y}.reflect(fp);
    }
    point<fp_t> r{(fp_t)(x - p.x), (fp_t)(y - p.y)};
    point<fp_t> s{(fp_t)(q.x - p.x), (fp_t)(q.y - p.y)};
    fp_t ssq = s.sqnorm();
    r = point<fp_t>{(r.x * s.x + r.y * s.y) / ssq, (r.x * s.y - r.y * s.x) / ssq};
    return point<fp_t>{r.x * s.x - r.y * s.y + fp.x, r.x * s.y + r.y * s.x + fp.y};
  }

  // --- Explicit type conversions ---

  point<double> to_double() const { return {(double)x, (double)y}; }
  point<long double> to_ldouble() const { return {(long double)x, (long double)y}; }

  // --- Friend free-function versions ---

  friend T sqnorm(const point &p) { return p.sqnorm(); }
  friend fp_t norm(const point &p) { return p.norm(); }
  friend fp_t arg(const point &p) { return p.arg(); }
  friend T dot(const point &p, const point &q) { return p.dot(q); }
  friend T cross(const point &p, const point &q) { return p.cross(q); }
  friend fp_t proj(const point &p, const point &q) { return p.proj(q); }
  friend point<fp_t> normalize(const point &p) { return p.normalize(); }
  friend point rotate90(const point &p) { return p.rotate90(); }
  friend point rotate180(const point &p) { return p.rotate180(); }
  friend point rotate270(const point &p) { return p.rotate270(); }
  friend point rotate90(const point &p, const point &q) { return p.rotate90(q); }
  friend point rotate180(const point &p, const point &q) { return p.rotate180(q); }
  friend point rotate270(const point &p, const point &q) { return p.rotate270(q); }
  friend point<fp_t> rotateCW(const point &p, fp_t t) { return p.rotateCW(t); }
  friend point<fp_t> rotateCCW(const point &p, fp_t t) { return p.rotateCCW(t); }
  friend point<fp_t> rotateCW(const point &p, const point &q, fp_t t) { return p.rotateCW(q, t); }
  friend point<fp_t> rotateCCW(const point &p, const point &q, fp_t t) { return p.rotateCCW(q, t); }
  friend point reflect(const point &p, const point &q) { return p.reflect(q); }

  friend point<fp_t> reflect(const point &p, const point &a, const point &b) {
    return p.reflect(a, b);
  }

  friend std::ostream &operator<<(std::ostream &out, const point &p) {
    if (std::is_integral<T>::value) {
      return out << "(" << p.x << "," << p.y << ")";
    }
    return out << "(" << (fabs(p.x) < EPS ? 0 : p.x) << "," << (fabs(p.y) < EPS ? 0 : p.y) << ")";
  }
};

using PointI = point<int>;
using PointD = point<double>;
using PointLD = point<long double>;
using Point = PointD;  // Default point type is double.

/*** Example Usage ***/

#include <cassert>

const double PI = acos(-1.0);

int main() {
  // Float point - same API as before.
  Point p(-10, 3);
  assert(Point(-18, 29) == p + Point(-3, 9) * 6.0 / 2.0 - Point(-1, 1));
  assert(EQ(109, p.sqnorm()));
  assert(EQ(10.44030650891, p.norm()));
  assert(EQ(2.850135859112, p.arg()));
  assert(EQ(0, p.dot(Point(3, 10))));
  assert(EQ(0, p.cross(Point(10, -3))));
  assert(EQ(10, p.proj(Point(-10, 0))));
  assert(EQ(1, p.normalize().norm()));
  assert(Point(-3, -10) == p.rotate90());
  assert(Point(10, -3) == p.rotate180());
  assert(Point(3, 10) == p.rotate270());
  assert(Point(3, 12) == p.rotateCW(Point(1, 1), PI / 2));
  assert(Point(1, -10) == p.rotateCCW(Point(2, 2), PI / 2));
  assert(Point(10, -3) == p.reflect(Point(0, 0)));
  assert(Point(-10, -3) == p.reflect(Point(-2, 0), Point(5, 0)));

  // Integer point - exact arithmetic, float-only ops return PointD.
  PointI a(3, 4), b(1, 0);
  assert(a + b == PointI(4, 4));
  assert(a - b == PointI(2, 4));
  assert(a * 2 == PointI(6, 8));
  assert(a.sqnorm() == 25);
  assert(a.dot(b) == 3);
  assert(a.cross(b) == -4);
  assert(EQ(a.norm(), 5.0));
  assert(a.rotate90() == PointI(-4, 3));
  assert(a.rotate180() == PointI(-3, -4));
  assert(a.rotate270() == PointI(4, -3));
  PointD anorm = a.normalize();  // returns PointD
  assert(EQ(anorm.norm(), 1.0));
  PointD adiv = a / 2.0;  // returns PointD (division always promotes)
  assert(EQ(adiv.x, 1.5) && EQ(adiv.y, 2.0));
  auto rotated = PointI(1, 0).rotateCCW(PI / 4);  // returns PointD (arbitrary rotation promotes)
  double root2over2 = sqrt(2.0) / 2.0;
  assert(EQ(rotated.x, root2over2) && EQ(rotated.y, root2over2));

  // Implicit conversion PointI -> PointD.
  PointD promoted = PointI(1, 2);
  assert(promoted == PointD(1, 2));

  return 0;
}
