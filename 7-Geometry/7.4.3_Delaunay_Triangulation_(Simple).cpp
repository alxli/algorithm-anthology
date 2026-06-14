/*

Given a set $P$ of distinct two-dimensional points, the Delaunay triangulation of $P$ is a set of
non-overlapping triangles that covers the entire convex hull of $P$ such that no point in $P$ lies
within the circumcircle of any of the resulting triangles.

The Delaunay triangulation is not necessarily unique when four or more points are cocircular.
This implementation produces one valid triangulation using a simple brute-force algorithm. Each
candidate triangle is tested against the empty-circumcircle condition, and candidates whose edges
would properly cross already accepted triangles are rejected.

- `delaunay_triangulation(lo, hi)` returns a Delaunay triangulation for the input range `[lo, hi)`
  of distinct points, where `lo` and `hi` must be random-access iterators, or an empty vector if a
  triangulation does not exist.

Time Complexity:
- O(n^6) per call to `delaunay_triangulation(lo, hi)`, where $n$ is the distance between `lo` and
  `hi`. The empty-circumcircle test contributes O(n^4); the remaining factor comes from checking
  candidate triangles against previously accepted triangles.

Space Complexity:
- O(n) auxiliary heap space, excluding the returned triangulation.

*/

#include <algorithm>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

// Specialized version of seg_intersection() from 7.2.3, simplified for touch_is_intersect = false,
// i.e. we're detecting proper crossings of two non-parallel segments whose interiors intersect.
template<class Pt>
bool proper_seg_intersection(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  auto cross = [](const Pt &o, const Pt &p, const Pt &q) {
    return (p.x - o.x) * (q.y - o.y) - (p.y - o.y) * (q.x - o.x);  // Overflow warning!
  };
  auto c1 = cross(a, b, c);
  auto c2 = cross(a, b, d);
  auto c3 = cross(c, d, a);
  auto c4 = cross(c, d, b);
  return ((LT(c1, 0) && LT(0, c2)) || (LT(c2, 0) && LT(0, c1))) &&
         ((LT(c3, 0) && LT(0, c4)) || (LT(c4, 0) && LT(0, c3)));
}

struct Triangle {
  double ax, ay, bx, by, cx, cy;

  Triangle(double ax, double ay, double bx, double by, double cx, double cy)
      : ax(ax), ay(ay), bx(bx), by(by), cx(cx), cy(cy) {}

  bool operator==(const Triangle &t) const {
    return EQ(ax, t.ax) && EQ(ay, t.ay) && EQ(bx, t.bx) && EQ(by, t.by) && EQ(cx, t.cx) &&
           EQ(cy, t.cy);
  }
};

// Accepts any point type with numeric .x/.y; coordinates are copied to double arrays.
template<class It>
std::vector<Triangle> delaunay_triangulation(It lo, It hi) {
  using Pt = typename std::iterator_traits<It>::value_type;
  int n = hi - lo;
  std::vector<Pt> pts;
  std::vector<double> x, y, z;
  for (It it = lo; it != hi; ++it) {
    pts.emplace_back(it->x, it->y);
    x.emplace_back(it->x);
    y.emplace_back(it->y);
    z.emplace_back(static_cast<double>(it->x) * it->x + static_cast<double>(it->y) * it->y);
  }
  std::vector<Triangle> res;
  std::vector<std::array<int, 3>> res_idx;
  for (int i = 0; i < n - 2; i++) {
    for (int j = i + 1; j < n; j++) {
      for (int k = i + 1; k < n; k++) {
        if (j == k) {
          continue;
        }
        double nx = (y[j] - y[i]) * (z[k] - z[i]) - (y[k] - y[i]) * (z[j] - z[i]);
        double ny = (x[k] - x[i]) * (z[j] - z[i]) - (x[j] - x[i]) * (z[k] - z[i]);
        double nz = (x[j] - x[i]) * (y[k] - y[i]) - (x[k] - x[i]) * (y[j] - y[i]);
        if (LE(0, nz)) {
          continue;
        }
        std::vector<Pt> s1{pts[i], pts[j], pts[k], pts[i]};
        for (int m = 0; m < n; m++) {
          if (LT(0, nx * (x[m] - x[i]) + ny * (y[m] - y[i]) + nz * (z[m] - z[i]))) {
            goto skip;
          }
        }
        // Reject candidates whose edges properly cross already accepted triangles.
        for (const auto &tri : res_idx) {
          std::vector<Pt> s2{pts[tri[0]], pts[tri[1]], pts[tri[2]], pts[tri[0]]};
          for (int u = 0; u < 3; u++) {
            for (int v = 0; v < 3; v++) {
              if (proper_seg_intersection(s1[u], s1[u + 1], s2[v], s2[v + 1])) {
                goto skip;
              }
            }
          }
        }
        res.emplace_back(pts[i].x, pts[i].y, pts[j].x, pts[j].y, pts[k].x, pts[k].y);
        res_idx.push_back({i, j, k});
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
  bool operator==(const Point &p) const { return EQ(x, p.x) && EQ(y, p.y); }
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
  vector<Triangle> t{
      Triangle(1, 3, 1, 2, -1, 3), Triangle(1, 3, 2, 1, 1, 2), Triangle(1, 2, 2, 1, 0, 0),
      Triangle(1, 2, 0, 0, -1, 3)
  };
  assert(delaunay_triangulation(v.begin(), v.end()) == t);

  // Integer-coordinate inputs are accepted (triangulation computed in double).
  vector<PointI> iv{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  assert(delaunay_triangulation(iv.begin(), iv.end()) == t);
  return 0;
}
