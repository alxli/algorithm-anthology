/*

4.5.3 - System Solver (Gaussian Elimination)

Given a system of m linear equations with n unknowns:

A(1,1)*x(1) + A(1,2)*x(2) + ... + A(1,n)*x(n) = B(1)
A(2,1)*x(1) + A(2,2)*x(2) + ... + A(2,n)*x(n) = B(2)
                        ...
A(m,1)*x(1) + A(m,2)*x(2) + ... + A(m,n)*x(n) = B(m)

For any system of linear equations, there will either
be no solution (in 2d, lines are parallel), a single
solution (in 2d, the lines intersect at a point), or
or infinite solutions (in 2d, lines are the same).

Using Gaussian elimination in O(n^3), this program
solves for the values of x(1) ... x(n) or determines
that no unique solution of x() exists. Note that
the implementation below uses 0-based indices.

*/

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <vector>

const double eps = 1e-9;
typedef std::vector<double> vd;
typedef std::vector<vd> vvd;

//note: A[i][n] stores B[i]
vd solve_system(vvd A) {
  int m = A.size(), n = A[0].size() - 1;
  vd x(n);
  if (n > m) goto fail;
  for (int k = 0; k < n; k++) {
    double mv = 0; int mi;
    for (int i = k; i < m; i++)
      if (mv < fabs(A[i][k]))
        mv = fabs(A[mi = i][k]);
    if (mv < eps) goto fail;
    for (int i = 0; i <= n; i++)
      std::swap(A[mi][i], A[k][i]);
    for (int i = k + 1; i < m; i++) {
      double v = A[i][k] / A[k][k];
      for (int j = k; j <= n; j++)
        A[i][j] -= v * A[k][j];
      A[i][k] = 0;
    }
  }
  for (int i = n; i < m; i++)
    if (fabs(A[i][n]) > eps) goto fail;
  for (int i = n - 1; i >= 0; i--) {
    if (fabs(A[i][i]) < eps) goto fail;
    double v = 0;
    for (int j = i + 1; j < n; j++)
      v += A[i][j] * x[j];
    v = A[i][n] - v;
    x[i] = v / A[i][i];
  }
  return x;
fail:;
  throw std::runtime_error("No unique solution");
  return vd();
}

/*** Example Usage (wcipeg.com/problem/syssolve) ***/

#include <cstdio>

int main() {
  int n, m;
  scanf("%d%d", &n, &m);
  vvd a(m, vd(n + 1));
  for (int i = 0; i < m; i++)
    for (int j = 0; j <= n; j++)
      scanf("%lf", &a[i][j]);
  try {
    vd x = solve_system(a);
    for (int i = 0; i < n; i++)
      printf("%.6lf\n", x[i]);
  } catch (...) {
    printf("NO UNIQUE SOLUTION\n");
  }
  return 0;
}
