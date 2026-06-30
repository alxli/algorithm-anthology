/*

Given a set $P$ of distinct two-dimensional points, a Delaunay triangulation of $P$ is a
triangulation of the convex hull of $P$ such that no point of $P$ lies strictly inside the
circumcircle of any triangle in the triangulation. The Delaunay triangulation is not necessarily
unique when four or more points are cocircular.

This implementation produces one valid triangulation using a simple brute-force algorithm. Each
candidate triangle is tested against the empty-circumcircle condition, and candidates whose edges
would properly cross already accepted triangles are rejected.

- `delaunay_triangulation(p)` returns a Delaunay triangulation of the distinct points in `p` as a
  vector of clockwise-oriented vertex triples, or an empty vector if no triangulation exists (e.g.
  fewer than three points, or all points collinear).

All arithmetic uses the point's own coordinate type, so integer inputs yield an exact triangulation.
The lifted-paraboloid test scales as $\sim 96 \cdot C^4$ where $C$ is the coordinate magnitude, so a
64-bit integer coordinate type is safe up to $|C| \leq \sim 17600$. For floating-point coordinates
the test is subject to the usual rounding error.

Time Complexity:
- O(n^6) per call, where $n$ is the number of points. The empty-circumcircle test contributes O(n^4)
  while O(n^2) comes from checking candidate triangles against previously accepted triangles.

Space Complexity:
- O(n) auxiliary, excluding the returned triangulation.

*/

#include <cmath>
#include <tuple>
#include <type_traits>
#include <vector>

const double EPS = 1e-9;

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool EQ(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return std::fabs(C(a) - C(b)) <= static_cast<C>(EPS);
  return C(a) == C(b);
}

template<typename T, typename U, typename C = std::common_type_t<T, U>>
bool LT(T a, U b) {
  if constexpr (std::is_floating_point_v<C>) return C(a) < C(b) - static_cast<C>(EPS);
  return C(a) < C(b);
}

template<typename T, typename U>
bool LE(T a, U b) {
  return !LT(b, a);
}

// Specialized version of seg_intersection() from 7.2.3, simplified for touch_is_intersect = false,
// i.e. we're detecting proper crossings of two non-parallel segments whose interiors intersect.
template<typename Pt>
bool proper_seg_intersection(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  auto cross = [](const Pt &o, const Pt &p, const Pt &q) {
    return (p.x - o.x) * (q.y - o.y) - (p.y - o.y) * (q.x - o.x);  // Overflow warning!
  };
  auto c1 = cross(a, b, c), c2 = cross(a, b, d), c3 = cross(c, d, a), c4 = cross(c, d, b);
  return ((LT(c1, 0) && LT(0, c2)) || (LT(c2, 0) && LT(0, c1))) &&
         ((LT(c3, 0) && LT(0, c4)) || (LT(c4, 0) && LT(0, c3)));
}

template<typename Pt>
std::vector<std::tuple<Pt, Pt, Pt>> delaunay_triangulation(const std::vector<Pt> &p) {
  int n = static_cast<int>(p.size());
  std::vector<decltype(Pt::x)> z;
  for (const auto &[px, py] : p) {
    z.emplace_back(px * px + py * py);  // Overflow warning!
  }
  std::vector<std::tuple<Pt, Pt, Pt>> res;
  std::vector<std::tuple<int, int, int>> res_idx;
  for (int i = 0; i < n - 2; i++) {
    for (int j = i + 1; j < n; j++) {
      for (int k = i + 1; k < n; k++) {
        if (j == k) {
          continue;
        }
        // Overflow warning!
        auto nx = (p[j].y - p[i].y) * (z[k] - z[i]) - (p[k].y - p[i].y) * (z[j] - z[i]);
        auto ny = (p[k].x - p[i].x) * (z[j] - z[i]) - (p[j].x - p[i].x) * (z[k] - z[i]);
        auto nz = (p[j].x - p[i].x) * (p[k].y - p[i].y) - (p[k].x - p[i].x) * (p[j].y - p[i].y);
        if (LE(0, nz)) {
          continue;
        }
        std::vector<Pt> s1{p[i], p[j], p[k], p[i]};
        for (int m = 0; m < n; m++) {
          if (LT(0, nx * (p[m].x - p[i].x) + ny * (p[m].y - p[i].y) + nz * (z[m] - z[i]))) {
            goto skip;
          }
        }
        // Reject candidates whose edges properly cross already accepted triangles.
        for (auto [t0, t1, t2] : res_idx) {
          std::vector<Pt> s2{p[t0], p[t1], p[t2], p[t0]};
          for (int u = 0; u < 3; u++) {
            for (int v = 0; v < 3; v++) {
              if (proper_seg_intersection(s1[u], s1[u + 1], s2[v], s2[v + 1])) {
                goto skip;
              }
            }
          }
        }
        res.emplace_back(p[i], p[j], p[k]);
        res_idx.emplace_back(i, j, k);
      skip:;
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
};

int main() {
  vector<Point> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<tuple<Point, Point, Point>> t{
      {Point{1, 3}, Point{1, 2}, Point{-1, 3}},
      {Point{1, 3}, Point{2, 1}, Point{1, 2}},
      {Point{1, 2}, Point{2, 1}, Point{0, 0}},
      {Point{1, 2}, Point{0, 0}, Point{-1, 3}}
  };
  assert(delaunay_triangulation(v) == t);

  // Integer-coordinate inputs are triangulated using exact integer arithmetic.
  vector<PointI> iv{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<tuple<PointI, PointI, PointI>> ti{
      {PointI{1, 3}, PointI{1, 2}, PointI{-1, 3}},
      {PointI{1, 3}, PointI{2, 1}, PointI{1, 2}},
      {PointI{1, 2}, PointI{2, 1}, PointI{0, 0}},
      {PointI{1, 2}, PointI{0, 0}, PointI{-1, 3}}
  };
  assert(delaunay_triangulation(iv) == ti);
  return 0;
}
