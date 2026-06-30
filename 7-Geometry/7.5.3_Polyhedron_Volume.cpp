/*

Given an oriented triangular mesh of a closed polyhedron, compute its signed volume. Each triangle
contributes the signed volume of the tetrahedron formed by the triangle $abc$ and the origin
$((a \times b) \cdot c) / 6$. If faces are oriented outward, the result is positive for the
orientation used by `convex_hull_3d()` in 7.5.2; take `abs()` if only the magnitude is needed.

- `signed_polyhedron_volume(points, faces)` returns the signed volume. Each face is an index triple.

Time Complexity:
- O(f) per call, where $f$ is the number of triangular faces.

Space Complexity:
- O(1) auxiliary.

*/

#include <cmath>
#include <tuple>
#include <type_traits>
#include <vector>

struct Point3D {
  double x, y, z;
  Point3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
  double dot(const Point3D &p) const { return x * p.x + y * p.y + z * p.z; }
  Point3D cross(const Point3D &p) const {
    return {y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x};
  }
};

template<typename Pt, typename F>
double signed_polyhedron_volume(const std::vector<Pt> &p, const std::vector<F> &faces) {
  double volume = 0;
  for (const auto &[a, b, c] : faces) {
    volume += p[a].cross(p[b]).dot(p[c]);
  }
  return volume / 6.0;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  using Face = std::tuple<int, int, int>;  // (a, b, c)
  vector<Point3D> p{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  vector<Face> faces{
      Face(0, 2, 1),
      Face(0, 1, 3),
      Face(0, 3, 2),
      Face(1, 2, 3),
  };
  assert(EQ(signed_polyhedron_volume(p, faces), 1.0 / 6.0));
  return 0;
}
