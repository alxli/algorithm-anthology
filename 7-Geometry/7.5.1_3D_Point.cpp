/*

A 3D point class templated on the coordinate type `T`. Algebraic operations preserve `T`, while
metric operations use `fp_t`, which is `T` for floating-point coordinates and `double` otherwise.

- `TPoint3<T>(x = 0, y = 0, z = 0)` constructs point (`x`, `y`, `z`).
- Operators `+`, `-`, `*`, and `/` preserve the coordinate type except that division promotes to
  `fp_t`. Comparisons are exact and lexicographic, while `EQ(p, q)` uses `EPS` for floating-point
  coordinates and remains exact for other types.
- `sqnorm()`, `dot(p)`, and `cross(p)` return exact algebraic results in coordinate type `T`.
- `norm()`, `phi()`, and `theta()` return the length, azimuth, and polar angle in `fp_t`.
- `unit()` returns the unit vector and requires a nonzero point; `normal(p)` returns a unit normal
  and requires nonparallel vectors.
- `rotate(angle, axis)` rotates the point about a nonzero `axis` using Rodrigues' formula.
- `Point3I`, `Point3L`, `Point3D`, and `Point3LD` use `int`, `int64_t`, `double`, and `long double`
  coordinates, respectively.

Overflow warning: the exact products `dot()`, `cross()`, and `sqnorm()` grow like the squared
coordinate magnitude. With `TPoint3<int>` these overflow a 32-bit `int` once coordinates exceed a
few tens of thousands, so use `Point3L` (`TPoint3<int64_t>`) for larger integer coordinates.

Time Complexity:
- O(1) per operation.

Space Complexity:
- O(1) for storage and auxiliary.

*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) == C(b) || std::fabs(C(a) - C(b)) <= EPS;
  return C(a) == C(b);
}

template<typename T>
struct TPoint3 {
  using fp_t = std::conditional_t<std::is_floating_point<T>::value, T, double>;

  T x, y, z;

  TPoint3(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}
  bool operator==(const TPoint3 &p) const { return std::tie(x, y, z) == std::tie(p.x, p.y, p.z); }

  friend bool EQ(const TPoint3 &a, const TPoint3 &b) {
    return EQ(a.x, b.x) && EQ(a.y, b.y) && EQ(a.z, b.z);
  }

  bool operator!=(const TPoint3 &p) const { return !(*this == p); }
  bool operator<(const TPoint3 &p) const { return std::tie(x, y, z) < std::tie(p.x, p.y, p.z); }
  TPoint3 operator+(const TPoint3 &p) const { return {x + p.x, y + p.y, z + p.z}; }
  TPoint3 operator-(const TPoint3 &p) const { return {x - p.x, y - p.y, z - p.z}; }
  TPoint3 operator*(T k) const { return {x * k, y * k, z * k}; }
  TPoint3<fp_t> operator/(fp_t k) const { return {fp_t(x) / k, fp_t(y) / k, fp_t(z) / k}; }
  T dot(const TPoint3 &p) const { return x * p.x + y * p.y + z * p.z; }  // Overflow warning.
  T sqnorm() const { return x * x + y * y + z * z; }                     // Overflow warning.
  fp_t norm() const { return std::hypot(std::hypot(fp_t(x), fp_t(y)), fp_t(z)); }
  fp_t phi() const { return std::atan2(fp_t(y), fp_t(x)); }
  fp_t theta() const { return std::atan2(std::hypot(fp_t(x), fp_t(y)), fp_t(z)); }
  TPoint3<fp_t> unit() const { return *this / norm(); }

  TPoint3 cross(const TPoint3 &p) const {
    return {y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x};  // Overflow warning.
  }

  TPoint3<fp_t> normal(const TPoint3 &p) const { return cross(p).unit(); }

  TPoint3<fp_t> rotate(fp_t angle, const TPoint3 &axis) const {
    fp_t s = std::sin(angle), c = std::cos(angle);
    TPoint3<fp_t> u = axis.unit(), v{fp_t(x), fp_t(y), fp_t(z)};
    return u * v.dot(u) * (1 - c) + v * c - v.cross(u) * s;
  }
};

using Point3I = TPoint3<int>;
using Point3L = TPoint3<int64_t>;
using Point3D = TPoint3<double>;
using Point3LD = TPoint3<long double>;

/*** Example Usage ***/

using namespace std;

int main() {
  Point3I a(1, 0, 0), b(0, 1, 0);
  assert(a.dot(b) == 0);
  assert(a.cross(b) == Point3I(0, 0, 1));
  assert(a.sqnorm() == 1);

  Point3D p(1, 0, 0);
  assert(EQ(p.rotate(acos(-1.0) / 2, Point3D(0, 0, 1)), Point3D(0, 1, 0)));
  return 0;
}
