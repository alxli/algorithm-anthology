/*

Solves a linear programming problem using Dantzig's simplex algorithm. The canonical form of a
linear programming problem is to maximize (or minimize) the dot product $cx$, subject to $ax \leq b$
and $x \geq 0$, where $x$ is a vector of unknowns to be solved, $c$ is a vector of coefficients, $a$
is a matrix of linear equation coefficients, and $b$ is a vector of boundary coefficients.

The simplex algorithm walks between vertices of the feasible polytope, at each step pivoting to an
adjacent vertex that improves the objective, until no improving move remains (an optimum is reached)
or an unbounded improving direction is found.

The implementation uses a two-phase simplex tableau. Phase 1 introduces an artificial variable to
find a feasible starting basis, which is essential when some constraint bounds are negative. Phase 2
then optimizes the original objective. The `basis` and `nonbasis` arrays track which variable each
tableau row or column currently represents, making solution recovery independent of pivot order.

- `simplex_solve(a, b, c, &x)` solves the linear programming problem for an $m$ by $n$ matrix `a` of
  real values, a length $m$ vector `b`, and a length $n$ vector `c`, returning 0 if an optimum was
  found, $-1$ if there are no feasible solutions, or 1 if the objective is unbounded. If an optimum
  is found, then the vector pointed to by `x` is populated with a dense solution vector.

Time Complexity:
- Polynomial (average) on the number of equations and unknowns, but exponential in the worst case.

Space Complexity:
- O(m*n) auxiliary heap space.

*/

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

int simplex_solve(
    const std::vector<std::vector<double>> &a, const std::vector<double> &b,
    const std::vector<double> &c, std::vector<double> *x, const bool MAXIMIZE = true,
    const double EPS = 1e-10
) {
  int m = static_cast<int>(a.size()), n = static_cast<int>(c.size());
  assert(x != nullptr && b.size() == a.size());
  for (const auto &row : a) {
    assert(row.size() == c.size());
  }
  std::vector<int> basis(m), nonbasis(n + 1);
  std::vector<std::vector<double>> tab(m + 2, std::vector<double>(n + 2));
  for (int i = 0; i < m; i++) {
    for (int j = 0; j < n; j++) {
      tab[i][j] = a[i][j];
    }
    basis[i] = n + i;
    tab[i][n] = -1;
    tab[i][n + 1] = b[i];
  }
  for (int j = 0; j < n; j++) {
    nonbasis[j] = j;
    tab[m][j] = MAXIMIZE ? -c[j] : c[j];
  }
  nonbasis[n] = -1;
  tab[m + 1][n] = 1;
  auto pivot = [&](int row, int col) {
    double inv = 1.0 / tab[row][col];
    for (int i = 0; i < m + 2; i++) {
      if (i == row || fabs(tab[i][col]) <= EPS) {
        continue;
      }
      double factor = tab[i][col] * inv;
      for (int j = 0; j < n + 2; j++) {
        if (j != col) {
          tab[i][j] -= factor * tab[row][j];
        }
      }
      tab[i][col] = -factor;
    }
    for (int j = 0; j < n + 2; j++) {
      if (j != col) {
        tab[row][j] *= inv;
      }
    }
    tab[row][col] = inv;
    std::swap(basis[row], nonbasis[col]);
  };
  auto simplex = [&](int phase) {
    int objective = (phase == 1 ? m + 1 : m);
    while (true) {
      int col = -1;
      for (int j = 0; j <= n; j++) {
        if (phase == 2 && nonbasis[j] == -1) {
          continue;
        }
        if (col == -1 || tab[objective][j] < tab[objective][col] - EPS ||
            (fabs(tab[objective][j] - tab[objective][col]) <= EPS && nonbasis[j] < nonbasis[col])) {
          col = j;
        }
      }
      if (tab[objective][col] >= -EPS) {
        return true;
      }
      int row = -1;
      for (int i = 0; i < m; i++) {
        if (tab[i][col] <= EPS) {
          continue;
        }
        double cur = tab[i][n + 1] / tab[i][col];
        double best = row == -1 ? 0 : tab[row][n + 1] / tab[row][col];
        if (row == -1 || cur < best - EPS || (fabs(cur - best) <= EPS && basis[i] < basis[row])) {
          row = i;
        }
      }
      if (row == -1) {
        return false;
      }
      pivot(row, col);
    }
  };
  int row = 0;
  for (int i = 1; i < m; i++) {
    if (tab[i][n + 1] < tab[row][n + 1]) {
      row = i;
    }
  }
  if (m > 0 && tab[row][n + 1] < -EPS) {
    pivot(row, n);
    if (!simplex(1) || fabs(tab[m + 1][n + 1]) > EPS) {
      return -1;
    }
    for (int i = 0; i < m; i++) {
      if (basis[i] != -1) {
        continue;
      }
      int col = -1;
      for (int j = 0; j <= n; j++) {
        if (fabs(tab[i][j]) > EPS && (col == -1 || nonbasis[j] < nonbasis[col])) {
          col = j;
        }
      }
      if (col != -1) {
        pivot(i, col);
      }
    }
  }
  if (!simplex(2)) {
    return 1;
  }
  x->assign(n, 0);
  for (int i = 0; i < m; i++) {
    if (basis[i] < n) {
      (*x)[basis[i]] = tab[i][n + 1];
    }
  }
  return 0;
}

/*** Example Usage and Output:

Solution = 33.3043 at (5.30435, 4.34783).

***/

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

  // Feasible even though the first RHS is negative: x >= 1 and x <= 3.
  vector<vector<double>> negative_rhs{{-1}, {1}};
  vector<double> negative_bounds{-1, 3};
  vector<double> one_var{1};
  assert(simplex_solve(negative_rhs, negative_bounds, one_var, &x) == 0);
  assert(fabs(x[0] - 3) < 1e-9);

  // Infeasible: x <= -1 contradicts x >= 0.
  vector<vector<double>> infeasible{{1}};
  vector<double> bad_bounds{-1};
  assert(simplex_solve(infeasible, bad_bounds, one_var, &x) == -1);

  // Unbounded: maximize x subject only to x >= 0.
  vector<vector<double>> unbounded{{-1}};
  vector<double> lower_bound{0};
  assert(simplex_solve(unbounded, lower_bound, one_var, &x) == 1);
  return 0;
}
