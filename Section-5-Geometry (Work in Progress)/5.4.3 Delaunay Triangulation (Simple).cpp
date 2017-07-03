/*

Given a range of points P on the Cartesian plane, the Delaunay
Triangulation of said points is a set of non-overlapping triangles
covering the entire convex hull of P, such that no point in P lies
within the circumcircle of any of the resulting triangles. The
triangulation maximizes the minimum angle of all the angles of the
triangles in the triangulation. In addition, for any point p in the
convex hull (not necessarily in P), the nearest point is guaranteed
to be a vertex of the enclosing triangle from the triangulation.
See: https://en.wikipedia.org/wiki/Delaunay_triangulation

The triangulation may not exist (e.g. for a set of collinear points)
or it may not be unique (multiple possible triangulations may exist).
The triangulation may not exist (e.g. for a set of collinear points)
or it may not be unique (multiple possible triangulations may exist).
The following program assumes that a triangulation exists, and
produces one such valid result using one of the simplest algorithms
to solve this problem. It involves encasing the simplex in a circle
and rejecting the simplex if another point in the tessellation is
within the generalized circle.

Time Complexity: O(n^4) on the number of input points.

*/

#include <algorithm> /* std::sort() */
#include <cmath>     /* fabs(), sqrt() */
#include <utility>   /* std::pair */
#include <vector>

const double eps = 1e-9;

#define EQ(a, b) (fabs((a) - (b)) <= eps) /* equal to */
#define LT(a, b) ((a) < (b) - eps)        /* less than */
#define GT(a, b) ((a) > (b) + eps)        /* greater than */
#define LE(a, b) ((a) <= (b) + eps)       /* less than or equal to */
#define GE(a, b) ((a) >= (b) - eps)       /* greater than or equal to */

typedef std::pair<double, double> point;
#define x first
#define y second

double norm(const point & a) { return a.x * a.x + a.y * a.y; }
double dot(const point & a, const point & b) { return a.x * b.x + a.y * b.y; }
double cross(const point & a, const point & b) { return a.x * b.y - a.y * b.x; }

const bool TOUCH_IS_INTERSECT = false;

bool contain(const double & l, const double & m, const double & h) {
  if (TOUCH_IS_INTERSECT) return LE(l, m) && LE(m, h);
  return LT(l, m) && LT(m, h);
}

bool overlap(const double & l1, const double & h1,
             const double & l2, const double & h2) {
  if (TOUCH_IS_INTERSECT) return LE(l1, h2) && LE(l2, h1);
  return LT(l1, h2) && LT(l2, h1);
}

int seg_intersection(const point & a, const point & b,
                     const point & c, const point & d) {
  point ab(b.x - a.x, b.y - a.y);
  point ac(c.x - a.x, c.y - a.y);
  point cd(d.x - c.x, d.y - c.y);
  double c1 = cross(ab, cd), c2 = cross(ac, ab);
  if (EQ(c1, 0) && EQ(c2, 0)) {
    double t0 = dot(ac, ab) / norm(ab);
    double t1 = t0 + dot(cd, ab) / norm(ab);
    if (overlap(std::min(t0, t1), std::max(t0, t1), 0, 1)) {
      point res1 = std::max(std::min(a, b), std::min(c, d));
      point res2 = std::min(std::max(a, b), std::max(c, d));
      return (res1 == res2) ? 0 : 1;
    }
    return -1;
  }
  if (EQ(c1, 0)) return -1;
  double t = cross(ac, cd) / c1, u = c2 / c1;
  if (contain(0, t, 1) && contain(0, u, 1)) return 0;
  return -1;
}

struct triangle { point a, b, c; };

template<class It>
std::vector<triangle> delaunay_triangulation(It lo, It hi) {
  int n = hi - lo;
  std::vector<double> x, y, z;
  for (It it = lo; it != hi; ++it) {
    x.push_back(it->x);
    y.push_back(it->y);
    z.push_back((it->x) * (it->x) + (it->y) * (it->y));
  }
  std::vector<triangle> res;
  for (int i = 0; i < n - 2; i++) {
    for (int j = i + 1; j < n; j++) {
      for (int k = i + 1; k < n; k++) {
        if (j == k) continue;
        double nx = (y[j] - y[i]) * (z[k] - z[i]) - (y[k] - y[i]) * (z[j] - z[i]);
        double ny = (x[k] - x[i]) * (z[j] - z[i]) - (x[j] - x[i]) * (z[k] - z[i]);
        double nz = (x[j] - x[i]) * (y[k] - y[i]) - (x[k] - x[i]) * (y[j] - y[i]);
        if (GE(nz, 0)) continue;
        bool done = false;
        for (int m = 0; m < n; m++)
          if (x[m] - x[i]) * nx + (y[m] - y[i]) * ny + (z[m] - z[i]) * nz > 0) {
            done = true;
            break;
          }
        if (!done) { //handle 4 points on a circle
          point s1[] = { *(lo + i), *(lo + j), *(lo + k), *(lo + i) };
          for (int t = 0; t < (int)res.size(); t++) {
            point s2[] = { res[t].a, res[t].b, res[t].c, res[t].a };
            for (int u = 0; u < 3; u++)
              for (int v = 0; v < 3; v++)
                if (seg_intersection(s1[u], s1[u + 1], s2[v], s2[v + 1]) == 0)
                  goto skip;
          }
          res.push_back((triangle){*(lo + i), *(lo + j), *(lo + k)});
        }
skip:;
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  vector<point> v;
  v.push_back(point(1, 3));
  v.push_back(point(1, 2));
  v.push_back(point(2, 1));
  v.push_back(point(0, 0));
  v.push_back(point(-1, 3));
  vector<triangle> dt = delaunay_triangulation(v.begin(), v.end());
  for (int i = 0; i < (int)dt.size(); i++) {
    cout << "Triangle: ";
    cout << "(" << dt[i].a.x << "," << dt[i].a.y << ") ";
    cout << "(" << dt[i].b.x << "," << dt[i].b.y << ") ";
    cout << "(" << dt[i].c.x << "," << dt[i].c.y << ")\n";
  }
  return 0;
}
