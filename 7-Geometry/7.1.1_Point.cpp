/*

A 2D point class template supporting epsilon comparisons. The coordinate type `T` defaults to
`double`. Integer types (e.g. `int`) fully support all exact operations. Floating-point-only
operations (`norm`, `arg`, `normalize`, `rotateCW/CCW`, `reflect` across a line) return coordinates
of type `fp_t`, which is `double` when `T` is integral and `T` itself otherwise.

Exact operations (return `TPoint<T>` or `T`, no precision lost for integers):
- element-wise arithmetic, `dot()`, `cross()`, `sqnorm()`, cardinal rotations, `reflect(point)`,
  comparisons.

Overflow warning: the exact products `dot()`, `cross()`, and `sqnorm()` grow like the squared
coordinate magnitude. With `TPoint<int>` these overflow a 32-bit `int` once coordinates exceed
~46000, so use `PointL` (`TPoint<long long>`) for larger integer coordinates.

Floating-point-only operations (return `TPoint<fp_t>` or `fp_t`):
- `norm()`, `arg()`, `proj()`, `normalize()`, `rotateCW()`, `rotateCCW()`, `reflect(line)`.
- `operator/` also promotes to `fp_t`.

Implicit conversion from `TPoint<U>` to `TPoint<T>` is provided when `U` is integral and `T` is
floating-point, so `PointI` can be passed wherever `PointD` is expected.

Non-floating-point coordinate types such as `Modular` or `Rational` compose for the exact
operations (arithmetic, `dot`, `cross`, `sqnorm`, comparisons): these route to exact `==`/`<`
rather than epsilon comparisons. The floating-point-only operations are simply not instantiated
unless called.

Type aliases:
- `PointI = TPoint<int>`: exact integer geometry (small coordinates only; see overflow warning)
- `PointL = TPoint<long long>`: exact integer geometry for large coordinates
- `PointD = TPoint<double>`: standard floating-point
- `PointLD = TPoint<long double>`: extra precision
- `Point = PointD`: default point type is double

Time Complexity:
- O(1) per call to the constructor and all other operations.

Space Complexity:
- O(1) for storage of the point.
- O(1) auxiliary for all operations.

*/

#include <cmath>
#include <cstdint>
#include <ostream>
#include <type_traits>
#include <utility>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  return std::is_integral_v<C> ? C(a) == C(b) : std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  return std::is_integral_v<C> ? C(a) < C(b) : C(a) < C(b) - static_cast<C>(EPS);
}

template<typename T>
struct TPoint {
  // fp_t is T for floating-point types, double for integral types.
  using fp_t = std::conditional_t<std::is_integral<T>::value, double, T>;

  T x, y;

  TPoint() : x(0), y(0) {}
  TPoint(T x, T y) : x(x), y(y) {}
  explicit TPoint(const std::pair<T, T> &p) : x(p.first), y(p.second) {}

  // Implicit conversion from integral to floating-point point (optional, can just use to_double()).
  template<
      typename U,
      typename = std::enable_if_t<std::is_integral<U>::value && std::is_floating_point<T>::value>>
  TPoint(const TPoint<U> &p) : x(static_cast<T>(p.x)), y(static_cast<T>(p.y)) {}

  bool operator==(const TPoint &p) const {
    if constexpr (std::is_floating_point<T>::value) {
      return EQ(x, p.x) && EQ(y, p.y);
    }
    return x == p.x && y == p.y;  // exact for ints and types like Modular/Rational
  }

  bool operator!=(const TPoint &p) const { return !(*this == p); }

  bool operator<(const TPoint &p) const {
    if constexpr (std::is_floating_point<T>::value) {
      return EQ(x, p.x) ? LT(y, p.y) : LT(x, p.x);
    }
    return x == p.x ? y < p.y : x < p.x;  // exact for ints and types like Modular/Rational
  }

  bool operator>(const TPoint &p) const { return p < *this; }
  bool operator<=(const TPoint &p) const { return !(*this > p); }
  bool operator>=(const TPoint &p) const { return !(*this < p); }
  TPoint operator+(const TPoint &p) const { return {x + p.x, y + p.y}; }
  TPoint operator-(const TPoint &p) const { return {x - p.x, y - p.y}; }
  TPoint operator+(T v) const { return {x + v, y + v}; }
  TPoint operator-(T v) const { return {x - v, y - v}; }
  TPoint operator*(T v) const { return {x * v, y * v}; }

  // Division always promotes to fp_t to avoid integer truncation.
  TPoint<fp_t> operator/(fp_t v) const { return {(fp_t)x / v, (fp_t)y / v}; }

  // clang-format off
  TPoint &operator+=(const TPoint &p) { x += p.x; y += p.y; return *this; }
  TPoint &operator-=(const TPoint &p) { x -= p.x; y -= p.y; return *this; }
  TPoint &operator+=(T v) { x += v; y += v; return *this; }
  TPoint &operator-=(T v) { x -= v; y -= v; return *this; }
  TPoint &operator*=(T v) { x *= v; y *= v; return *this; }
  friend TPoint operator+(T v, const TPoint &p) { return p + v; }
  friend TPoint operator*(T v, const TPoint &p) { return p * v; }
  // clang-format on

  // --- Exact operations: return T or TPoint<T>, work for any coordinate type ---

  // Overflow warning: these products grow like the squared coordinate magnitude, so for integer T
  // with large coordinates (beyond a few thousand for 32-bit int) use a wider type such as PointL.
  T sqnorm() const { return x * x + y * y; }
  T dot(const TPoint &p) const { return x * p.x + y * p.y; }
  T cross(const TPoint &p) const { return x * p.y - y * p.x; }

  // --- Floating-point operations: return fp_t or TPoint<fp_t> ---

  fp_t norm() const { return sqrt(static_cast<fp_t>(sqnorm())); }
  fp_t arg() const { return atan2(static_cast<fp_t>(y), static_cast<fp_t>(x)); }
  fp_t proj(const TPoint &p) const { return static_cast<fp_t>(dot(p)) / p.norm(); }

  TPoint<fp_t> normalize() const {
    fp_t n = norm();
    if (n < static_cast<fp_t>(1e-30)) {  // guard against dividing by a near-zero norm, not a
      return {fp_t(0), fp_t(0)};         // geometric tolerance (EPS is too coarse here)
    }
    return {static_cast<fp_t>(x) / n, static_cast<fp_t>(y) / n};
  }

  // --- Cardinal rotations: exact for all coordinate types including int ---

  // Returns (x, y) rotated 90/180/270 degrees counter-clockwise about the origin.
  TPoint rotate90() const { return {-y, x}; }
  TPoint rotate180() const { return {-x, -y}; }
  TPoint rotate270() const { return {y, -x}; }

  // Returns (x, y) rotated 90/180/270 degrees counter-clockwise about point p.
  TPoint rotate90(const TPoint &p) const { return (*this - p).rotate90() + p; }
  TPoint rotate180(const TPoint &p) const { return (*this - p).rotate180() + p; }
  TPoint rotate270(const TPoint &p) const { return (*this - p).rotate270() + p; }

  // --- Arbitrary-angle rotations: always return floating-point ---

  // Returns (x, y) rotated t radians clockwise about the origin.
  TPoint<fp_t> rotateCW(fp_t t) const {
    fp_t fx = (fp_t)x, fy = (fp_t)y;
    return {fx * cos(t) + fy * sin(t), fy * cos(t) - fx * sin(t)};
  }

  // Returns (x, y) rotated t radians counter-clockwise about the origin.
  TPoint<fp_t> rotateCCW(fp_t t) const {
    fp_t fx = (fp_t)x, fy = (fp_t)y;
    return {fx * cos(t) - fy * sin(t), fx * sin(t) + fy * cos(t)};
  }

  // Returns (x, y) rotated t radians clockwise about point p.
  TPoint<fp_t> rotateCW(const TPoint &p, fp_t t) const {
    return TPoint<fp_t>{(fp_t)(x - p.x), (fp_t)(y - p.y)}.rotateCW(t) +
           TPoint<fp_t>{(fp_t)p.x, (fp_t)p.y};
  }

  // Returns (x, y) rotated t radians counter-clockwise about point p.
  TPoint<fp_t> rotateCCW(const TPoint &p, fp_t t) const {
    return TPoint<fp_t>{(fp_t)(x - p.x), (fp_t)(y - p.y)}.rotateCCW(t) +
           TPoint<fp_t>{(fp_t)p.x, (fp_t)p.y};
  }

  // --- Reflections ---

  // Returns (x, y) reflected across point p. Exact for any coordinate type.
  TPoint reflect(const TPoint &p) const { return {2 * p.x - x, 2 * p.y - y}; }

  // Returns (x, y) reflected across the line containing points p and q.
  // Always returns floating-point coordinates.
  TPoint<fp_t> reflect(const TPoint &p, const TPoint &q) const {
    TPoint<fp_t> fp{(fp_t)p.x, (fp_t)p.y};
    if (p == q) {
      return TPoint<fp_t>{(fp_t)x, (fp_t)y}.reflect(fp);
    }
    TPoint<fp_t> r{(fp_t)(x - p.x), (fp_t)(y - p.y)}, s{(fp_t)(q.x - p.x), (fp_t)(q.y - p.y)};
    fp_t ssq = s.sqnorm();
    r = TPoint<fp_t>{(r.x * s.x + r.y * s.y) / ssq, (r.x * s.y - r.y * s.x) / ssq};
    return TPoint<fp_t>{r.x * s.x - r.y * s.y + fp.x, r.x * s.y + r.y * s.x + fp.y};
  }

  // --- Explicit type conversions ---

  TPoint<double> to_double() const { return {static_cast<double>(x), static_cast<double>(y)}; }

  TPoint<long double> to_ldouble() const {
    return {static_cast<long double>(x), static_cast<long double>(y)};
  }

  // --- Friend free-function versions ---

  // clang-format off
  friend T sqnorm(const TPoint &p) { return p.sqnorm(); }
  friend fp_t norm(const TPoint &p) { return p.norm(); }
  friend fp_t arg(const TPoint &p) { return p.arg(); }
  friend T dot(const TPoint &p, const TPoint &q) { return p.dot(q); }
  friend T cross(const TPoint &p, const TPoint &q) { return p.cross(q); }
  friend fp_t proj(const TPoint &p, const TPoint &q) { return p.proj(q); }
  friend TPoint<fp_t> normalize(const TPoint &p) { return p.normalize(); }
  friend TPoint rotate90(const TPoint &p) { return p.rotate90(); }
  friend TPoint rotate180(const TPoint &p) { return p.rotate180(); }
  friend TPoint rotate270(const TPoint &p) { return p.rotate270(); }
  friend TPoint rotate90(const TPoint &p, const TPoint &q) { return p.rotate90(q); }
  friend TPoint rotate180(const TPoint &p, const TPoint &q) { return p.rotate180(q); }
  friend TPoint rotate270(const TPoint &p, const TPoint &q) { return p.rotate270(q); }
  friend TPoint<fp_t> rotateCW(const TPoint &p, fp_t t) { return p.rotateCW(t); }
  friend TPoint<fp_t> rotateCCW(const TPoint &p, fp_t t) { return p.rotateCCW(t); }
  friend TPoint<fp_t> rotateCW(const TPoint &p, const TPoint &q, fp_t t) { return p.rotateCW(q, t); }
  friend TPoint<fp_t> rotateCCW(const TPoint &p, const TPoint &q, fp_t t) { return p.rotateCCW(q, t); }
  friend TPoint reflect(const TPoint &p, const TPoint &q) { return p.reflect(q); }
  friend TPoint<fp_t> reflect(const TPoint &p, const TPoint &a, const TPoint &b) { return p.reflect(a, b); }
  // clang-format on

  friend std::ostream &operator<<(std::ostream &out, const TPoint &p) {
    if constexpr (std::is_floating_point<T>::value) {
      return out << "(" << (fabs(p.x) < EPS ? 0 : p.x) << "," << (fabs(p.y) < EPS ? 0 : p.y) << ")";
    }
    return out << "(" << p.x << "," << p.y << ")";
  }
};

using PointI = TPoint<int>;
using PointL = TPoint<long long>;
using PointD = TPoint<double>;
using PointLD = TPoint<long double>;
using Point = PointD;  // Default point type is double.

/*** Example Usage ***/

#include <cassert>

const double PI = acos(-1.0);

int main() {
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
