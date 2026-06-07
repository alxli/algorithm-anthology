/*

Solves a linear programming problem using Dantzig's simplex algorithm. The canonical form of a
linear programming problem is to maximize (or minimize) the dot product $cx$, subject to $ax \leq b$
and $x \geq 0$, where $x$ is a vector of unknowns to be solved, $c$ is a vector of coefficients, $a$
is a matrix of linear equation coefficients, and $b$ is a vector of boundary coefficients.

- `simplex_solve(a, b, c, &x)` solves the linear programming problem for an $m$ by $n$ matrix `a` of
  real values, a length $m$ vector `b`, and a length $n$ vector `c`, returning 0 if an optimum was
  found, $-1$ if there are no feasible solutions, or 1 if the objective is unbounded. If an optimum
  is found, then the vector pointed to by `x` is populated with a dense solution vector.

Time Complexity:
- Polynomial (average) on the number of equations and unknowns, but exponential in the worst case.

Space Complexity:
- O(m*n) auxiliary heap space.

*/

#include <cfloat>
#include <cmath>
#include <vector>

template<class Matrix>
int simplex_solve(
    const Matrix &a, const std::vector<double> &b, const std::vector<double> &c,
    std::vector<double> *x, const bool MAXIMIZE = true, const double EPS = 1e-10
) {
  int m = a.size(), n = c.size();
  Matrix t(m + 2, std::vector<double>(n + 2));
  t[1][1] = 0;
  for (int j = 1; j <= n; j++) {
    t[1][j + 1] = MAXIMIZE ? c[j - 1] : -c[j - 1];
  }
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++) {
      t[i + 1][j + 1] = -a[i - 1][j - 1];
    }
    t[i + 1][1] = b[i - 1];
  }
  for (int j = 1; j <= n; j++) {
    t[0][j + 1] = j;
  }
  for (int i = n + 1; i <= m + n; i++) {
    t[i - n + 1][0] = i;
  }
  for (;;) {
    int p1 = 0, p2 = 0;
    double mn = DBL_MAX, xmax = EPS;
    for (int j = 2; j <= n + 1; j++) {
      if (t[1][j] > xmax) {
        p2 = j;
        xmax = t[1][j];
      }
    }
    if (p2 == 0) {
      break;
    }
    for (int i = 2; i <= m + 1; i++) {
      if (t[i][p2] < -EPS) {
        double v = fabs(t[i][1] / t[i][p2]);
        if (mn <= v) {
          continue;
        }
        mn = v;
        p1 = i;
      }
    }
    if (p1 == 0) {
      return 1;
    }
    std::swap(t[p1][0], t[0][p2]);
    for (int i = 1; i <= m + 1; i++) {
      if (i != p1) {
        for (int j = 1; j <= n + 1; j++) {
          if (j != p2) {
            t[i][j] -= t[p1][j] * t[i][p2] / t[p1][p2];
          }
        }
      }
    }
    t[p1][p2] = 1.0 / t[p1][p2];
    for (int j = 1; j <= n + 1; j++) {
      if (j != p2) {
        t[p1][j] *= fabs(t[p1][p2]);
      }
    }
    for (int i = 1; i <= m + 1; i++) {
      if (i != p1) {
        t[i][p2] *= t[p1][p2];
      }
    }
    for (int i = 2; i <= m + 1; i++) {
      if (t[i][1] < 0) {
        return -1;
      }
    }
  }
  x->assign(n, 0);
  for (int j = 1; j <= n; j++) {
    for (int i = 2; i <= m + 1; i++) {
      if (fabs(t[i][0] - j) < EPS) {
        (*x)[j - 1] = t[i][1];
      }
    }
  }
  return 0;
}

/*** Example Usage and Output:

Solution = 33.3043 at (5.30435, 4.34783).

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  // Solve [x, y] that maximizes 3x + 4y, subject to x, y >= 0 and:
  //  -2x +    1y <=  0
  //   1x + 0.85y <=  9
  //   1x +    2y <= 14
  vector<vector<double>> va{{-2, 1}, {1, 0.85}, {1, 2}};
  vector<double> vb{0, 9, 14};
  vector<double> vc{3, 4};
  vector<double> x;
  assert(simplex_solve(va, vb, vc, &x) == 0);
  double maxval = 0;
  for (int i = 0; i < static_cast<int>(x.size()); i++) {
    maxval += vc[i] * x[i];
  }
  cout << "Solution = " << maxval << " at (" << x[0];
  for (int i = 1; i < static_cast<int>(x.size()); i++) {
    cout << ", " << x[i];
  }
  cout << ")." << endl;
  return 0;
}
