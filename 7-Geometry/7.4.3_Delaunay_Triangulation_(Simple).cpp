/*

Given a set $P$ of two dimensional points, the Delaunay triangulation of $P$ is a set of
non-overlapping triangles that covers the entire convex hull of $P$ such that no point in $P$ lies
within the circumcircle of any of the resulting triangles. For any point $p$ in the convex hull of
$P$ (but not necessarily in $P$), the nearest point is guaranteed to be a vertex of the enclosing
triangle from the triangulation.

The triangulation may not exist (e.g. for a set of collinear points), or may not be unique if it
does exist. The following program assumes its existence and produces one such valid result using a
simple algorithm which encases each triangle in a circle and rejects the triangle if another point
in the tessellation is within the generalized circle.

- `delaunay_triangulation(lo, hi)` returns a Delaunay triangulation for the input range `[lo, hi)`
  of points, where `lo` and `hi` must be random-access iterators, or an empty vector if a
  triangulation does not exist.

Time Complexity:
- O(n^4) per call to `delaunay_triangulation(lo, hi)`, where $n$ is the distance between `lo` and
  `hi`.

Space Complexity:
- O(n) auxiliary heap space for storage of the Delaunay triangulation.

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

template<class Pt>
int seg_intersection(const Pt &a, const Pt &b, const Pt &c, const Pt &d) {
  static const bool TOUCH_IS_INTERSECT = false;  // false is important!
  double ab_x = b.x - a.x, ab_y = b.y - a.y;
  double ac_x = c.x - a.x, ac_y = c.y - a.y;
  double cd_x = d.x - c.x, cd_y = d.y - c.y;
  double c1 = ab_x * cd_y - ab_y * cd_x;
  double c2 = ac_x * ab_y - ac_y * ab_x;
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    Pt res1 = std::max(std::min(a, b), std::min(c, d));
    Pt res2 = std::min(std::max(a, b), std::max(c, d));
    if (TOUCH_IS_INTERSECT ? !(res2 < res1) : (res1 < res2)) {
      return 1;  // Collinear and overlapping.
    }
    return -1;  // Collinear and disjoint.
  }
  if (EQ(c1, 0)) {
    return -1;  // Parallel and disjoint.
  }
  double t_num = ac_x * cd_y - ac_y * cd_x;
  bool c1_pos = c1 > 0;
  bool t_between_01 = c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, t_num) && LE(t_num, c1))
                                                   : (LT(0, t_num) && LT(t_num, c1)))
                             : (TOUCH_IS_INTERSECT ? (LE(t_num, 0) && LE(c1, t_num))
                                                   : (LT(t_num, 0) && LT(c1, t_num)));
  bool u_between_01 =
      c1_pos ? (TOUCH_IS_INTERSECT ? (LE(0, c2) && LE(c2, c1)) : (LT(0, c2) && LT(c2, c1)))
             : (TOUCH_IS_INTERSECT ? (LE(c2, 0) && LE(c1, c2)) : (LT(c2, 0) && LT(c1, c2)));
  if (t_between_01 && u_between_01) {
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
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
    z.emplace_back((double)it->x * it->x + (double)it->y * it->y);
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
          if (nx * (x[m] - x[i]) + ny * (y[m] - y[i]) + nz * (z[m] - z[i]) > 0) {
            goto skip;
          }
        }
        // Handle four points on a circle.
        for (const auto &tri : res_idx) {
          std::vector<Pt> s2{pts[tri[0]], pts[tri[1]], pts[tri[2]], pts[tri[0]]};
          for (int u = 0; u < 3; u++) {
            for (int v = 0; v < 3; v++) {
              if (seg_intersection(s1[u], s1[u + 1], s2[v], s2[v + 1]) == 0) {
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

struct PointD {
  double x, y;
  PointD(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const PointD &p) const { return EQ(x, p.x) && EQ(y, p.y); }
  bool operator<(const PointD &p) const { return x != p.x ? x < p.x : y < p.y; }
};

struct PointI {
  int x, y;
  PointI(int x = 0, int y = 0) : x(x), y(y) {}
  bool operator==(const PointI &p) const { return x == p.x && y == p.y; }
  bool operator<(const PointI &p) const { return x != p.x ? x < p.x : y < p.y; }
};

int main() {
  vector<PointD> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
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
