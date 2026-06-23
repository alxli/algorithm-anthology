/*

A 3D point class template supporting vector arithmetic, dot and cross products, distances, angles,
unit vectors, normals, and rotation about an axis. Point equality and ordering are exact, while
`EQ(p, q)` provides epsilon equality for floating-point coordinates. Integer coordinates are
suitable for exact algebraic predicates (`dot`, `cross`, `sqnorm`) as long as intermediate products
do not overflow; metric operations preserve floating-point coordinates and otherwise convert them
to `double`. For `Modular`, this uses the stored representative and is not finite-field geometry.

Type aliases:
- `Point3I = TPoint3<int>`
- `Point3L = TPoint3<int64_t>`
- `Point3D = TPoint3<double>`
- `Point3LD = TPoint3<long double>`

Time Complexity:
- O(1) per operation.

Space Complexity:
- O(1) for storage and auxiliary space.

*/

#include <cassert>
#include <cmath>
#include <cstdint>
#include <tuple>
#include <type_traits>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
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
  TPoint3<fp_t> operator/(fp_t k) const { return {(fp_t)x / k, (fp_t)y / k, (fp_t)z / k}; }
  T dot(const TPoint3 &p) const { return x * p.x + y * p.y + z * p.z; }
  T sqnorm() const { return x * x + y * y + z * z; }

  fp_t norm() const {
    return hypot(hypot(static_cast<fp_t>(x), static_cast<fp_t>(y)), static_cast<fp_t>(z));
  }

  fp_t phi() const { return atan2(static_cast<fp_t>(y), static_cast<fp_t>(x)); }
  TPoint3<fp_t> unit() const { return *this / norm(); }

  TPoint3 cross(const TPoint3 &p) const {
    return {y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x};
  }

  TPoint3<fp_t> normal(const TPoint3 &p) const { return cross(p).unit(); }

  fp_t theta() const {
    return atan2(hypot(static_cast<fp_t>(x), static_cast<fp_t>(y)), static_cast<fp_t>(z));
  }

  TPoint3<fp_t> rotate(fp_t angle, const TPoint3 &axis) const {
    fp_t s = sin(angle), c = cos(angle);
    TPoint3<fp_t> u = axis.unit(), v((fp_t)x, (fp_t)y, (fp_t)z);
    return u * v.dot(u) * (1 - c) + v * c - v.cross(u) * s;
  }
};

using Point3I = TPoint3<int>;
using Point3L = TPoint3<int64_t>;
using Point3D = TPoint3<double>;
using Point3LD = TPoint3<long double>;

/*** Example Usage ***/

int main() {
  Point3I a(1, 0, 0), b(0, 1, 0);
  assert(a.dot(b) == 0);
  assert(a.cross(b) == Point3I(0, 0, 1));
  assert(a.sqnorm() == 1);

  Point3D p(1, 0, 0);
  auto r = p.rotate(acos(-1.0) / 2, Point3D(0, 0, 1));
  assert(EQ(r, Point3D(0, 1, 0)));
  return 0;
}
