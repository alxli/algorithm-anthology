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
The following program assumes that a triangulation exists, and
produces one such valid result. The following is a C++ adaptation of
a FORTRAN90 program, which applies a divide and conquer algorithm
with complex linear-time merging. The original program can be found
via the following link. It contains more thorough documentation,
comments, and debugging messages associated with the current asserts().
http://people.sc.fsu.edu/~burkardt/f_src/table_delaunay/table_delaunay.html

Time Complexity: O(n log n) on the number of input points.

*/

#include <algorithm> /* std::min(), std::max() */
#include <cassert>
#include <cmath>     /* fabs(), sqrt() */
#include <utility>   /* std::pair */
#include <vector>

int wrap(int ival, int ilo, int ihi) {
  int jlo = std::min(ilo, ihi), jhi = std::max(ilo, ihi);
  int wide = jhi + 1 - jlo, res = jlo;
  if (wide != 1)  {
    assert(wide != 0);
    int tmp = (ival - jlo) % wide;
    if (tmp < 0) res += abs(wide);
    res += tmp;
  }
  return res;
}

double epsilon() {
  double r = 1;
  while (1 < (double)(r + 1)) r /= 2;
  return 2 * r;
}

void permute(int n, double a[][2], int p[]) {
  for (int istart = 1; istart <= n; istart++) {
    if (p[istart - 1] < 0) continue;
    if (p[istart - 1] == istart) {
      p[istart - 1] = -p[istart - 1];
      continue;
    }
    double tmp0 = a[istart - 1][0];
    double tmp1 = a[istart - 1][1];
    int iget = istart;
    for (;;) {
      int iput = iget;
      iget = p[iget - 1];
      p[iput - 1] = -p[iput - 1];
      assert(!(iget < 1 || n < iget));
      if (iget == istart) {
        a[iput - 1][0] = tmp0;
        a[iput - 1][1] = tmp1;
        break;
      }
      a[iput - 1][0] = a[iget - 1][0];
      a[iput - 1][1] = a[iget - 1][1];
    }
  }
  for (int i = 0; i < n; i++) p[i] = -p[i];
  return;
}

int * sort_heap(int n, double a[][2]) {
  double aval[2];
  int i, ir, j, l, idxt;
  int *idx;
  if (n < 1) return NULL;
  if (n == 1) {
    idx = new int[1];
    idx[0] = 1;
    return idx;
  }
  idx = new int[n];
  for (int i = 0; i < n; i++) idx[i] = i + 1;
  l = n / 2 + 1;
  ir = n;
  for (;;) {
    if (1 < l) {
      l--;
      idxt = idx[l - 1];
      aval[0] = a[idxt - 1][0];
      aval[1] = a[idxt - 1][1];
    } else {
      idxt = idx[ir - 1];
      aval[0] = a[idxt - 1][0];
      aval[1] = a[idxt - 1][1];
      idx[ir - 1] = idx[0];
      if (--ir == 1) {
        idx[0] = idxt;
        break;
      }
    }
    i = l;
    j = 2 * l;
    while (j <= ir) {
      if (j < ir && (a[idx[j - 1] - 1][0] <  a[idx[j] - 1][0] ||
                    (a[idx[j - 1] - 1][0] == a[idx[j] - 1][0] &&
                     a[idx[j - 1] - 1][1] <  a[idx[j] - 1][1]))) {
        j++;
      }
      if ( aval[0]  < a[idx[j - 1] - 1][0] ||
          (aval[0] == a[idx[j - 1] - 1][0] &&
           aval[1]  < a[idx[j - 1] - 1][1])) {
        idx[i - 1] = idx[j - 1];
        i = j;
        j *= 2;
      } else {
        j = ir + 1;
      }
    }
    idx[i - 1] = idxt;
  }
  return idx;
}

int lrline(double xu, double yu, double xv1, double yv1,
           double xv2, double yv2, double dv) {
  double tol = 1e-7;
  double dx = xv2 - xv1, dy = yv2 - yv1;
  double dxu = xu - xv1, dyu = yu - yv1;
  double t = dy * dxu - dx * dyu + dv * sqrt(dx * dx + dy * dy);
  double tolabs = tol * std::max(std::max(fabs(dx), fabs(dy)),
                         std::max(fabs(dxu), std::max(fabs(dyu), fabs(dv))));
  if (tolabs < t) return 1;
  if (-tolabs <= t) return 0;
  return -1;
}

void vbedg(double x, double y, int point_num, double point_xy[][2],
           int tri_num, int tri_nodes[][3], int tri_neigh[][3],
           int *ltri, int *ledg, int *rtri, int *redg) {
  int a, b;
  double ax, ay, bx, by;
  bool done;
  int e, l, t;
  if (*ltri == 0) {
    done = false;
    *ltri = *rtri;
    *ledg = *redg;
  } else {
    done = true;
  }
  for (;;) {
    l = -tri_neigh[(*rtri) - 1][(*redg) - 1];
    t = l / 3;
    e = l % 3 + 1;
    a = tri_nodes[t - 1][e - 1];
    if (e <= 2) {
      b = tri_nodes[t - 1][e];
    } else {
      b = tri_nodes[t - 1][0];
    }
    ax = point_xy[a - 1][0];
    ay = point_xy[a - 1][1];
    bx = point_xy[b - 1][0];
    by = point_xy[b - 1][1];
    if (lrline(x, y, ax, ay, bx, by, 0.0) <= 0) break;
    *rtri = t;
    *redg = e;
  }
  if (done) return;
  t = *ltri;
  e = *ledg;
  for (;;) {
    b = tri_nodes[t - 1][e - 1];
    e = wrap(e - 1, 1, 3);
    while (0 < tri_neigh[t - 1][e - 1]) {
      t = tri_neigh[t - 1][e - 1];
      if (tri_nodes[t - 1][0] == b) {
        e = 3;
      } else if (tri_nodes[t - 1][1] == b) {
        e = 1;
      } else {
        e = 2;
      }
    }
    a = tri_nodes[t - 1][e - 1];
    ax = point_xy[a - 1][0];
    ay = point_xy[a - 1][1];
    bx = point_xy[b - 1][0];
    by = point_xy[b - 1][1];
    if (lrline(x, y, ax, ay, bx, by, 0.0) <= 0) break;
  }
  *ltri = t;
  *ledg = e;
  return;
}

int diaedg(double x0, double y0, double x1, double y1,
           double x2, double y2, double x3, double y3) {
  double ca, cb, s, tol, tola, tolb;
  int value;
  tol = 100.0 * epsilon();
  double dx10 = x1 - x0, dy10 = y1 - y0;
  double dx12 = x1 - x2, dy12 = y1 - y2;
  double dx30 = x3 - x0, dy30 = y3 - y0;
  double dx32 = x3 - x2, dy32 = y3 - y2;
  tola = tol * std::max(std::max(fabs(dx10), fabs(dy10)),
                        std::max(fabs(dx30), fabs(dy30)));
  tolb = tol * std::max(std::max(fabs(dx12), fabs(dy12)),
                        std::max(fabs(dx32), fabs(dy32)));
  ca = dx10 * dx30 + dy10 * dy30;
  cb = dx12 * dx32 + dy12 * dy32;
  if (tola < ca && tolb < cb) {
    value = -1;
  } else if (ca < -tola && cb < -tolb) {
    value = 1;
  } else {
    tola = std::max(tola, tolb);
    s = (dx10 * dy30 - dx30 * dy10) * cb + (dx32 * dy12 - dx12 * dy32) * ca;
    if (tola < s) {
      value = -1;
    } else if (s < -tola) {
      value = 1;
    } else {
      value = 0;
    }
  }
  return value;
}

int swapec(int i, int *top, int *btri, int *bedg,
           int point_num, double point_xy[][2],
           int tri_num, int tri_nodes[][3], int tri_neigh[][3], int stack[]) {
  int a, b, c, e, ee, em1, ep1, f, fm1, fp1, l, r, s, swap, t, tt, u;
  double x = point_xy[i - 1][0];
  double y = point_xy[i - 1][1];
  for (;;) {
    if (*top <= 0) break;
    t = stack[*top - 1];
    *top = *top - 1;
    if (tri_nodes[t - 1][0] == i) {
      e = 2;
      b = tri_nodes[t - 1][2];
    } else if (tri_nodes[t - 1][1] == i) {
      e = 3;
      b = tri_nodes[t - 1][0];
    } else {
      e = 1;
      b = tri_nodes[t - 1][1];
    }
    a = tri_nodes[t - 1][e - 1];
    u = tri_neigh[t - 1][e - 1];
    if (tri_neigh[u - 1][0] == t) {
      f = 1;
      c = tri_nodes[u - 1][2];
    } else if (tri_neigh[u - 1][1] == t) {
      f = 2;
      c = tri_nodes[u - 1][0];
    } else {
      f = 3;
      c = tri_nodes[u - 1][1];
    }
    swap = diaedg(x, y, point_xy[a - 1][0], point_xy[a - 1][1],
                        point_xy[c - 1][0], point_xy[c - 1][1],
                        point_xy[b - 1][0], point_xy[b - 1][1]);
    if (swap == 1) {
      em1 = wrap(e - 1, 1, 3);
      ep1 = wrap(e + 1, 1, 3);
      fm1 = wrap(f - 1, 1, 3);
      fp1 = wrap(f + 1, 1, 3);
      tri_nodes[t - 1][ep1 - 1] = c;
      tri_nodes[u - 1][fp1 - 1] = i;
      r = tri_neigh[t - 1][ep1 - 1];
      s = tri_neigh[u - 1][fp1 - 1];
      tri_neigh[t - 1][ep1 - 1] = u;
      tri_neigh[u - 1][fp1 - 1] = t;
      tri_neigh[t - 1][e - 1] = s;
      tri_neigh[u - 1][f - 1] = r;
      if (0 < tri_neigh[u - 1][fm1 - 1]) {
        *top = *top + 1;
        stack[*top - 1] = u;
      }
      if (0 < s) {
        if (tri_neigh[s - 1][0] == u) {
          tri_neigh[s - 1][0] = t;
        } else if (tri_neigh[s - 1][1] == u) {
          tri_neigh[s - 1][1] = t;
        } else {
          tri_neigh[s - 1][2] = t;
        }
        *top = *top + 1;
        if (point_num < *top) return 8;
        stack[*top - 1] = t;
      } else {
        if (u == *btri && fp1 == *bedg) {
          *btri = t;
          *bedg = e;
        }
        l = - (3 * t + e - 1);
        tt = t;
        ee = em1;
        while (0 < tri_neigh[tt - 1][ee - 1]) {
          tt = tri_neigh[tt - 1][ee - 1];
          if (tri_nodes[tt - 1][0] == a) {
            ee = 3;
          } else if (tri_nodes[tt - 1][1] == a) {
            ee = 1;
          } else {
            ee = 2;
          }
        }
        tri_neigh[tt - 1][ee - 1] = l;
      }
      if (0 < r) {
        if (tri_neigh[r - 1][0] == t) {
          tri_neigh[r - 1][0] = u;
        } else if (tri_neigh[r - 1][1] == t) {
          tri_neigh[r - 1][1] = u;
        } else {
          tri_neigh[r - 1][2] = u;
        }
      } else {
        if (t == *btri && ep1 == *bedg) {
          *btri = u;
          *bedg = f;
        }
        l = -(3 * u + f - 1);
        tt = u;
        ee = fm1;
        while (0 < tri_neigh[tt - 1][ee - 1]) {
          tt = tri_neigh[tt - 1][ee - 1];
          if (tri_nodes[tt - 1][0] == b) {
            ee = 3;
          } else if (tri_nodes[tt - 1][1] == b) {
            ee = 1;
          } else {
            ee = 2;
          }
        }
        tri_neigh[tt - 1][ee - 1] = l;
      }
    }
  }
  return 0;
}

void perm_inv(int n, int p[]) {
  int i, i0, i1, i2;
  assert(n > 0);
  for (i = 1; i <= n; i++) {
    i1 = p[i - 1];
    while (i < i1) {
      i2 = p[i1 - 1];
      p[i1 - 1] = -i2;
      i1 = i2;
    }
    p[i - 1] = -p[i - 1];
  }
  for (i = 1; i <= n; i++) {
    i1 = -p[i - 1];
    if (0 <= i1) {
      i0 = i;
      for (;;) {
        i2 = p[i1 - 1];
        p[i1 - 1] = i0;
        if (i2 < 0) break;
        i0 = i1;
        i1 = i2;
      }
    }
  }
  return;
}

int dtris2(int point_num, double point_xy[][2],
           int tri_nodes[][3], int tri_neigh[][3]) {
  double cmax;
  int e, error;
  int i, j, k, l, m, m1, m2, n;
  int ledg, lr, ltri, redg, rtri, t, top;
  double tol;
  int *stack = new int[point_num];
  tol = 100.0 * epsilon();
  int *idx = sort_heap(point_num, point_xy);
  permute(point_num, point_xy, idx);
  m1 = 0;
  for (i = 1; i < point_num; i++) {
    m = m1;
    m1 = i;
    k = -1;
    for (j = 0; j <= 1; j++) {
      cmax = std::max(fabs(point_xy[m][j]), fabs(point_xy[m1][j]));
      if (tol * (cmax + 1.0) < fabs(point_xy[m][j] - point_xy[m1][j])) {
        k = j;
        break;
      }
    }
    assert(k != -1);
  }
  m1 = 1;
  m2 = 2;
  j = 3;
  for (;;) {
    assert(point_num >= j);
    m = j;
    lr = lrline(point_xy[m - 1][0], point_xy[m - 1][1],
                point_xy[m1 - 1][0], point_xy[m1 - 1][1],
                point_xy[m2 - 1][0], point_xy[m2 - 1][1], 0.0);
    if (lr != 0) break;
    j++;
  }
  int tri_num = j - 2;
  if (lr == -1) {
    tri_nodes[0][0] = m1;
    tri_nodes[0][1] = m2;
    tri_nodes[0][2] = m;
    tri_neigh[0][2] = -3;
    for (i = 2; i <= tri_num; i++) {
      m1 = m2;
      m2 = i + 1;
      tri_nodes[i - 1][0] = m1;
      tri_nodes[i - 1][1] = m2;
      tri_nodes[i - 1][2] = m;
      tri_neigh[i - 1][0] = -3 * i;
      tri_neigh[i - 1][1] = i;
      tri_neigh[i - 1][2] = i - 1;
    }
    tri_neigh[tri_num - 1][0] = -3 * tri_num - 1;
    tri_neigh[tri_num - 1][1] = -5;
    ledg = 2;
    ltri = tri_num;
  } else {
    tri_nodes[0][0] = m2;
    tri_nodes[0][1] = m1;
    tri_nodes[0][2] = m;
    tri_neigh[0][0] = -4;
    for (i = 2; i <= tri_num; i++) {
      m1 = m2;
      m2 = i+1;
      tri_nodes[i - 1][0] = m2;
      tri_nodes[i - 1][1] = m1;
      tri_nodes[i - 1][2] = m;
      tri_neigh[i - 2][2] = i;
      tri_neigh[i - 1][0] = -3 * i - 3;
      tri_neigh[i - 1][1] = i - 1;
    }
    tri_neigh[tri_num - 1][2] = -3 * (tri_num);
    tri_neigh[0][1] = -3 * (tri_num) - 2;
    ledg = 2;
    ltri = 1;
  }
  top = 0;
  for (i = j + 1; i <= point_num; i++) {
    m = i;
    m1 = tri_nodes[ltri - 1][ledg - 1];
    if (ledg <= 2) {
      m2 = tri_nodes[ltri - 1][ledg];
    } else {
      m2 = tri_nodes[ltri - 1][0];
    }
    lr = lrline(point_xy[m - 1][0], point_xy[m - 1][1],
                point_xy[m1 - 1][0], point_xy[m1 - 1][1],
                point_xy[m2 - 1][0], point_xy[m2 - 1][1], 0.0);
    if (0 < lr) {
      rtri = ltri;
      redg = ledg;
      ltri = 0;
    } else {
      l = -tri_neigh[ltri - 1][ledg - 1];
      rtri = l / 3;
      redg = (l % 3) + 1;
    }
    vbedg(point_xy[m - 1][0], point_xy[m - 1][1],
          point_num, point_xy, tri_num, tri_nodes, tri_neigh,
          &ltri, &ledg, &rtri, &redg);
    n = tri_num + 1;
    l = -tri_neigh[ltri - 1][ledg - 1];
    for (;;) {
      t = l / 3;
      e = (l % 3) + 1;
      l = -tri_neigh[t - 1][e - 1];
      m2 = tri_nodes[t - 1][e - 1];
      if (e <= 2) {
        m1 = tri_nodes[t - 1][e];
      } else {
        m1 = tri_nodes[t - 1][0];
      }
      tri_num++;
      tri_neigh[t - 1][e - 1] = tri_num;
      tri_nodes[tri_num - 1][0] = m1;
      tri_nodes[tri_num - 1][1] = m2;
      tri_nodes[tri_num - 1][2] = m;
      tri_neigh[tri_num - 1][0] = t;
      tri_neigh[tri_num - 1][1] = tri_num - 1;
      tri_neigh[tri_num - 1][2] = tri_num + 1;
      top++;
      assert(point_num >= top);
      stack[top - 1] = tri_num;
      if (t == rtri && e == redg) break;
    }
    tri_neigh[ltri - 1][ledg - 1] = -3 * n - 1;
    tri_neigh[n - 1][1] = -3 * tri_num - 2;
    tri_neigh[tri_num - 1][2] = -l;
    ltri = n;
    ledg = 2;
    error = swapec(m, &top, &ltri, &ledg, point_num, point_xy,
                   tri_num, tri_nodes, tri_neigh, stack);
    assert(error == 0);
  }
  for (i = 0; i < 3; i++)
    for (j = 0; j < tri_num; j++)
      tri_nodes[j][i] = idx[tri_nodes[j][i] - 1];
  perm_inv(point_num, idx);
  permute(point_num, point_xy, idx);
  delete[] idx;
  delete[] stack;
  return tri_num;
}

/*** C++ Wrapper ***/

typedef std::pair<double, double> point;
#define x first
#define y second

struct triangle { point a, b, c; };

template<class It>
std::vector<triangle> delaunay_triangulation(It lo, It hi) {
  int n = hi - lo;
  double points[n][2];
  int tri_nodes[3 * n][3], tri_neigh[3 * n][3];
  int curr = 0;
  for (It it = lo; it != hi; ++curr, ++it) {
    points[curr][0] = it->x;
    points[curr][1] = it->y;
  }
  int m = dtris2(n, points, tri_nodes, tri_neigh);
  std::vector<triangle> res;
  for (int i = 0; i < m; i++)
    res.push_back((triangle){*(lo + (tri_nodes[i][0] - 1)),
                             *(lo + (tri_nodes[i][1] - 1)),
                             *(lo + (tri_nodes[i][2] - 1))});
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
