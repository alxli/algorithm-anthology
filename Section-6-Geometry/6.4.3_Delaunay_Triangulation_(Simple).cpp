/*

Given a set P of two dimensional points, the Delaunay triangulation of P is a
set of non-overlapping triangles that covers the entire convex hull of P such
that no point in P lies within the circumcircle of any of the resulting
triangles. For any point p in the convex hull of P (but not necessarily in P),
the nearest point is guaranteed to be a vertex of the enclosing triangle from
the triangulation.

The triangulation may not exist (e.g. for a set of collinear points), or may not
be unique if it does exists. The following program assumes its existence and
produces one such valid result using a simple algorithm which encases each
triangle in a circle and rejecting the triangle if another point in the
tessellation is within the generalized circle.

- delaunay_triangulation(lo, hi) returns a Delaunay triangulation for the input
  range [lo, hi) of points, where lo and hi must be random-access iterators, or
  an empty vector if a triangulation does not exist.

Time Complexity:
- O(n^4) per call to delaunay_triangulation(lo, hi), where n is the distance
  between lo and hi.

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

typedef std::pair<double, double> point;
#define x first
#define y second

double sqnorm(const point &a) { return a.x*a.x + a.y*a.y; }
double dot(const point &a, const point &b) { return a.x*b.x + a.y*b.y; }
double cross(const point &a, const point &b, const point &o = point(0, 0)) {
  return (a.x - o.x)*(b.y - o.y) - (a.y - o.y)*(b.x - o.x);
}

int seg_intersection(const point &a, const point &b, const point &c,
                     const point &d, point *p = NULL, point *q = NULL) {
  static const bool TOUCH_IS_INTERSECT = false;  // false is important!
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {  // Collinear.
    double t0 = dot(ac, ab) / sqnorm(ab);
    double t1 = t0 + dot(cd, ab) / sqnorm(ab);
    double mint = std::min(t0, t1), maxt = std::max(t0, t1);
    bool overlap = TOUCH_IS_INTERSECT ? (LE(mint, 1) && LE(0, maxt))
                                      : (LT(mint, 1) && LT(0, maxt));
    if (overlap) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      if (res1 == res2) {
        if (p != NULL) {
          *p = res1;
        }
        return 0;  // Collinear and meeting at an endpoint.
      }
      if (p != NULL && q != NULL) {
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
  double t = cross(ac, cd)/c1, u = c2/c1;
  bool t_between_01 = TOUCH_IS_INTERSECT ? (LE(0, t) && LE(t, 1))
                                         : (LT(0, t) && LT(t, 1));
  bool u_between_01 = TOUCH_IS_INTERSECT ? (LE(0, u) && LE(u, 1))
                                         : (LT(0, u) && LT(u, 1));
  if (t_between_01 && u_between_01) {
    if (p != NULL) {
      *p = point(a.x + t*ab.x, a.y + t*ab.y);
    }
    return 0;  // Non-parallel with one intersection.
  }
  return -1;  // Non-parallel with no intersections.
}

struct triangle {
  point a, b, c;

  triangle(const point &a, const point &b, const point &c) : a(a), b(b), c(c) {}

  bool operator==(const triangle &t) const {
    return EQ(a.x, t.a.x) && EQ(a.y, t.a.y) &&
           EQ(b.x, t.b.x) && EQ(b.y, t.b.y) &&
           EQ(c.x, t.c.x) && EQ(c.y, t.c.y);
  }
};

template<class It>
std::vector<triangle> delaunay_triangulation(It lo, It hi) {
  int n = hi - lo;
  std::vector<double> x, y, z;
  for (It it = lo; it != hi; ++it) {
    x.push_back(it->x);
    y.push_back(it->y);
    z.push_back(sqnorm(*it));
  }
  std::vector<triangle> res;
  for (int i = 0; i < n - 2; i++) {
    for (int j = i + 1; j < n; j++) {
      for (int k = i + 1; k < n; k++) {
        if (j == k) {
          continue;
        }
        double nx = (y[j] - y[i])*(z[k] - z[i]) - (y[k] - y[i])*(z[j] - z[i]);
        double ny = (x[k] - x[i])*(z[j] - z[i]) - (x[j] - x[i])*(z[k] - z[i]);
        double nz = (x[j] - x[i])*(y[k] - y[i]) - (x[k] - x[i])*(y[j] - y[i]);
        if (LE(0, nz)) {
          continue;
        }
        point s1[] = {lo[i], lo[j], lo[k], lo[i]};
        for (int m = 0; m < n; m++) {
          if (nx*(x[m] - x[i]) + ny*(y[m] - y[i]) + nz*(z[m] - z[i]) > 0) {
            goto skip;
          }
        }
        // Handle four points on a circle.
        for (int t = 0; t < (int)res.size(); t++) {
          point s2[] = {res[t].a, res[t].b, res[t].c, res[t].a};
          for (int u = 0; u < 3; u++) {
            for (int v = 0; v < 3; v++) {
              if (seg_intersection(s1[u], s1[u + 1], s2[v], s2[v + 1]) == 0) {
                goto skip;
              }
            }
          }
        }
        res.push_back(triangle(lo[i], lo[j], lo[k]));
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
  vector<point> v;
  v.push_back(point(1, 3));
  v.push_back(point(1, 2));
  v.push_back(point(2, 1));
  v.push_back(point(0, 0));
  v.push_back(point(-1, 3));
  vector<triangle> t;
  t.push_back(triangle(point(1, 3), point(1, 2), point(-1, 3)));
  t.push_back(triangle(point(1, 3), point(2, 1), point(1, 2)));
  t.push_back(triangle(point(1, 2), point(2, 1), point(0, 0)));
  t.push_back(triangle(point(1, 2), point(0, 0), point(-1, 3)));
  assert(delaunay_triangulation(v.begin(), v.end()) == t);
  return 0;
}
