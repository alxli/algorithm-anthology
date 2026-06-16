/*

Given an oriented triangular mesh of a closed polyhedron, compute its signed volume. Each triangle
contributes the signed volume of the tetrahedron formed by the triangle and the origin:
`dot(cross(a, b), c) / 6`. If faces are oriented outward, the result is positive for the orientation
used by `convex_hull_3d()` in 7.5.2; take `abs()` if only the magnitude is needed.

- `signed_polyhedron_volume(points, faces)` returns the signed volume. Each face is an index triple
  with fields `.a`, `.b`, `.c`.

Time Complexity:
- O(f), where $f$ is the number of triangular faces.

Space Complexity:
- O(1) auxiliary.

*/

#include <cassert>
#include <cmath>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)

struct Point3 {
  double x, y, z;
  Point3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
  double dot(const Point3 &p) const { return x * p.x + y * p.y + z * p.z; }
  Point3 cross(const Point3 &p) const {
    return {y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x};
  }
};

struct Face {
  int a, b, c;
  Face(int a = 0, int b = 0, int c = 0) : a(a), b(b), c(c) {}
};

template<class Pt, class F>
double signed_polyhedron_volume(const std::vector<Pt> &p, const std::vector<F> &faces) {
  double volume = 0;
  for (const F &f : faces) {
    volume += p[f.a].cross(p[f.b]).dot(p[f.c]);
  }
  return volume / 6.0;
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<Point3> p{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  vector<Face> faces{
      Face(0, 2, 1),
      Face(0, 1, 3),
      Face(0, 3, 2),
      Face(1, 2, 3),
  };
  assert(EQ(signed_polyhedron_volume(p, faces), 1.0 / 6.0));
  return 0;
}
