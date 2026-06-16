/*

A 3D point class template supporting vector arithmetic, dot and cross products, distances, angles,
unit vectors, normals, and rotation about an axis. Integer coordinates are suitable for exact
algebraic predicates (`dot`, `cross`, `sqnorm`) as long as intermediate products do not overflow;
metric operations return floating-point values.

Type aliases:
- `Point3I = Point3D<int>`
- `Point3L = Point3D<int64_t>`
- `Point3 = Point3D<double>`

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

#define EQ(a, b) (fabs((a) - (b)) <= EPS)

template<class T>
struct Point3D {
  using fp_t = std::conditional_t<std::is_integral<T>::value, double, T>;

  T x, y, z;

  Point3D(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}
  bool operator==(const Point3D &p) const { return std::tie(x, y, z) == std::tie(p.x, p.y, p.z); }
  bool operator!=(const Point3D &p) const { return !(*this == p); }
  bool operator<(const Point3D &p) const { return std::tie(x, y, z) < std::tie(p.x, p.y, p.z); }
  Point3D operator+(const Point3D &p) const { return {x + p.x, y + p.y, z + p.z}; }
  Point3D operator-(const Point3D &p) const { return {x - p.x, y - p.y, z - p.z}; }
  Point3D operator*(T k) const { return {x * k, y * k, z * k}; }
  Point3D<fp_t> operator/(fp_t k) const { return {(fp_t)x / k, (fp_t)y / k, (fp_t)z / k}; }
  T dot(const Point3D &p) const { return x * p.x + y * p.y + z * p.z; }
  Point3D cross(const Point3D &p) const {
    return {y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x};
  }
  T sqnorm() const { return x * x + y * y + z * z; }
  fp_t norm() const { return sqrt(static_cast<fp_t>(sqnorm())); }
  fp_t phi() const { return atan2(static_cast<fp_t>(y), static_cast<fp_t>(x)); }
  fp_t theta() const {
    return atan2(hypot(static_cast<fp_t>(x), static_cast<fp_t>(y)), static_cast<fp_t>(z));
  }
  Point3D<fp_t> unit() const { return *this / norm(); }
  Point3D<fp_t> normal(const Point3D &p) const { return cross(p).unit(); }
  Point3D<fp_t> rotate(fp_t angle, const Point3D &axis) const {
    fp_t s = sin(angle), c = cos(angle);
    Point3D<fp_t> u = axis.unit(), v((fp_t)x, (fp_t)y, (fp_t)z);
    return u * v.dot(u) * (1 - c) + v * c - v.cross(u) * s;
  }
};

using Point3I = Point3D<int>;
using Point3L = Point3D<int64_t>;
using Point3 = Point3D<double>;

/*** Example Usage ***/

int main() {
  Point3I a(1, 0, 0), b(0, 1, 0);
  assert(a.dot(b) == 0);
  assert(a.cross(b) == Point3I(0, 0, 1));
  assert(a.sqnorm() == 1);

  Point3 p(1, 0, 0);
  auto r = p.rotate(acos(-1.0) / 2, Point3(0, 0, 1));
  assert(EQ(r.x, 0) && EQ(r.y, 1) && EQ(r.z, 0));
  return 0;
}
