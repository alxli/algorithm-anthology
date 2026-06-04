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
#include <cmath>
#include <utility>
#include <vector>

const double EPS = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= EPS)
#define LT(a, b) ((a) < (b) - EPS)
#define LE(a, b) ((a) <= (b) + EPS)

struct Point {
  double x, y;
  Point(double x = 0, double y = 0) : x(x), y(y) {}
  bool operator==(const Point &p) const { return x == p.x && y == p.y; }
  bool operator!=(const Point &p) const { return !(*this == p); }
  bool operator<(const Point &p) const { return x != p.x ? x < p.x : y < p.y; }
  bool operator>(const Point &p) const { return p < *this; }
};

double sqnorm(const Point &a) {
  return a.x * a.x + a.y * a.y;
}

double dot(const Point &a, const Point &b) {
  return a.x * b.x + a.y * b.y;
}

double cross(const Point &a, const Point &b, const Point &o = Point(0, 0)) {
  return (a.x - o.x) * (b.y - o.y) - (a.y - o.y) * (b.x - o.x);
}

int seg_intersection(
    const Point &a, const Point &b, const Point &c, const Point &d, Point *p = nullptr,
    Point *q = nullptr
) {
  static const bool TOUCH_IS_INTERSECT = false;  // false is important!
  Point ab(b.x - a.x, b.y - a.y);
  Point ac(c.x - a.x, c.y - a.y);
  Point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    double t0 = dot(ac, ab) / sqnorm(ab);
    double t1 = t0 + dot(cd, ab) / sqnorm(ab);
    double mint = std::min(t0, t1), maxt = std::max(t0, t1);
    bool overlap = TOUCH_IS_INTERSECT ? (LE(mint, 1) && LE(0, maxt)) : (LT(mint, 1) && LT(0, maxt));
    if (overlap) {
      Point res1 = std::max(std::min(a, b), std::min(c, d));
      Point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != nullptr) {
          *p = res1;
        }
        return 0;  // Collinear and meeting at an endpoint.
      }
      if (p != nullptr && q != nullptr) {
        *p = res1;
        *q = res2;
      }
      return 1;  // Collinear and overlapping.
    } else {
      return -1;  // Collinear and disjoint.
    }
  }
  if (EQ(c1, 0)) {
    return -1;  // Parallel and disjoint.
  }
  double t = cross(ac, cd) / c1, u = c2 / c1;
  bool t_between_01 = TOUCH_IS_INTERSECT ? (LE(0, t) && LE(t, 1)) : (LT(0, t) && LT(t, 1));
  bool u_between_01 = TOUCH_IS_INTERSECT ? (LE(0, u) && LE(u, 1)) : (LT(0, u) && LT(u, 1));
  if (t_between_01 && u_between_01) {
    if (p != nullptr) {
      *p = Point(a.x + t * ab.x, a.y + t * ab.y);
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

struct Triangle {
  Point a, b, c;

  Triangle(const Point &a, const Point &b, const Point &c) : a(a), b(b), c(c) {}

  bool operator==(const Triangle &t) const {
    return EQ(a.x, t.a.x) && EQ(a.y, t.a.y) && EQ(b.x, t.b.x) && EQ(b.y, t.b.y) && EQ(c.x, t.c.x) &&
           EQ(c.y, t.c.y);
  }
};

template<class It>
std::vector<Triangle> delaunay_triangulation(It lo, It hi) {
  int n = hi - lo;
  std::vector<double> x, y, z;
  for (It it = lo; it != hi; ++it) {
    x.emplace_back(it->x);
    y.emplace_back(it->y);
    z.emplace_back(sqnorm(*it));
  }
  std::vector<Triangle> res;
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
        Point s1[] = {lo[i], lo[j], lo[k], lo[i]};
        for (int m = 0; m < n; m++) {
          if (nx * (x[m] - x[i]) + ny * (y[m] - y[i]) + nz * (z[m] - z[i]) > 0) {
            goto skip;
          }
        }
        // Handle four points on a circle.
        for (const auto &tri : res) {
          Point s2[] = {tri.a, tri.b, tri.c, tri.a};
          for (int u = 0; u < 3; u++) {
            for (int v = 0; v < 3; v++) {
              if (seg_intersection(s1[u], s1[u + 1], s2[v], s2[v + 1]) == 0) {
                goto skip;
              }
            }
          }
        }
        res.emplace_back(lo[i], lo[j], lo[k]);
      skip:;
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Point> v{{1, 3}, {1, 2}, {2, 1}, {0, 0}, {-1, 3}};
  vector<Triangle> t{
      Triangle(Point(1, 3), Point(1, 2), Point(-1, 3)),
      Triangle(Point(1, 3), Point(2, 1), Point(1, 2)),
      Triangle(Point(1, 2), Point(2, 1), Point(0, 0)),
      Triangle(Point(1, 2), Point(0, 0), Point(-1, 3))
  };
  assert(delaunay_triangulation(v.begin(), v.end()) == t);
  return 0;
}
