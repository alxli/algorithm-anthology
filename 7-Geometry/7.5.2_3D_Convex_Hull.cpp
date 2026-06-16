/*

Given points in 3D, compute the triangular faces of their convex hull with an incremental
algorithm. Start with a tetrahedron, delete every visible face for each new point, and stitch the
horizon edges to that point. Returned faces are oriented outward.

- `convex_hull_3d(points)` returns a list of `Face{a, b, c}` index triples into `points`.

Preconditions: at least four points, and no four points coplanar. For coplanar or highly
degenerate inputs, perturbing points slightly or using a more robust library is recommended.

Time Complexity:
- O(n^2), where $n$ is the number of points.

Space Complexity:
- O(n^2) auxiliary.

*/

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <vector>

const double EPS = 1e-9;

struct Point3 {
  double x, y, z;
  Point3(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}
  Point3 operator-(const Point3 &p) const { return {x - p.x, y - p.y, z - p.z}; }
  Point3 operator*(double k) const { return {x * k, y * k, z * k}; }
  double dot(const Point3 &p) const { return x * p.x + y * p.y + z * p.z; }
  Point3 cross(const Point3 &p) const {
    return {y * p.z - z * p.y, z * p.x - x * p.z, x * p.y - y * p.x};
  }
};

struct Face {
  int a, b, c;
  Face(int a = 0, int b = 0, int c = 0) : a(a), b(b), c(c) {}
};

std::vector<Face> convex_hull_3d(const std::vector<Point3> &p) {
  int n = static_cast<int>(p.size());
  assert(n >= 4);
  std::vector<std::vector<std::array<int, 2>>> edge(
      n, std::vector<std::array<int, 2>>(n, {-1, -1})
  );
  std::vector<Face> faces;

  auto normal = [&](const Face &f) { return (p[f.b] - p[f.a]).cross(p[f.c] - p[f.a]); };
  auto visible = [&](const Face &f, int i) { return normal(f).dot(p[i] - p[f.a]) > EPS; };
  auto add_edge = [&](int a, int b, int c) {
    if (a > b) {
      std::swap(a, b);
    }
    auto &e = edge[a][b];
    (e[0] == -1 ? e[0] : e[1]) = c;
  };
  auto remove_edge = [&](int a, int b, int c) {
    if (a > b) {
      std::swap(a, b);
    }
    auto &e = edge[a][b];
    (e[0] == c ? e[0] : e[1]) = -1;
  };
  auto edge_count = [&](int a, int b) {
    if (a > b) {
      std::swap(a, b);
    }
    return (edge[a][b][0] != -1) + (edge[a][b][1] != -1);
  };
  auto make_face = [&](int a, int b, int c, int inside) {
    Face f(a, b, c);
    if (normal(f).dot(p[inside] - p[a]) > 0) {
      std::swap(f.b, f.c);
    }
    add_edge(f.a, f.b, f.c);
    add_edge(f.b, f.c, f.a);
    add_edge(f.c, f.a, f.b);
    faces.push_back(f);
  };

  for (int i = 0; i < 4; i++) {
    for (int j = i + 1; j < 4; j++) {
      for (int k = j + 1; k < 4; k++) {
        make_face(i, j, k, 6 - i - j - k);
      }
    }
  }

  for (int i = 4; i < n; i++) {
    for (int j = 0; j < static_cast<int>(faces.size()); j++) {
      Face f = faces[j];
      if (visible(f, i)) {
        remove_edge(f.a, f.b, f.c);
        remove_edge(f.b, f.c, f.a);
        remove_edge(f.c, f.a, f.b);
        std::swap(faces[j--], faces.back());
        faces.pop_back();
      }
    }
    int old_faces = static_cast<int>(faces.size());
    for (int j = 0; j < old_faces; j++) {
      Face f = faces[j];
      if (edge_count(f.a, f.b) != 2) {
        make_face(f.a, f.b, i, f.c);
      }
      if (edge_count(f.b, f.c) != 2) {
        make_face(f.b, f.c, i, f.a);
      }
      if (edge_count(f.c, f.a) != 2) {
        make_face(f.c, f.a, i, f.b);
      }
    }
  }
  return faces;
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<Point3> tetra{{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  auto faces = convex_hull_3d(tetra);
  assert(faces.size() == 4);

  tetra.push_back(Point3(1, 1, 1));
  faces = convex_hull_3d(tetra);
  assert(faces.size() == 6);
  return 0;
}
