/*

Solves a linear programming problem using Dantzig's simplex algorithm. The
canonical form of a linear programming problem is to maximize (or minimize) the
dot product c*x, subject to a*x <= b and x >= 0, where x is a vector of unknowns
to be solved, c is a vector of coefficients, a is a matrix of linear equation
coefficients, and b is a vector of boundary coefficients.

- simplex_solve(a, b, c, &x) solves the linear programming problem for an m by n
  matrix a of real values, a length m vector b, a length n vector c, returning 0
  if a solution was found or -1 if there is no solution. If a solution is found,
  then the vector pointed to by x is populated with the solution vector of
  length n.

Time Complexity:
- Polynomial (average) on the number of equations and unknowns, but exponential
  in the worst case.

Space Complexity:
- O(m*n) auxiliary heap space.

*/

#include <cmath>
#include <limits>
#include <vector>

template<class Matrix>
int simplex_solve(const Matrix &a, const std::vector<double> &b,
                  const std::vector<double> &c, std::vector<double> *x,
                  const bool MAXIMIZE = true, const double EPS = 1e-10) {
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
  double p1 = 0, p2 = 0;
  bool done = true;
  do {
    double mn = std::numeric_limits<double>::max(), xmax = 0, v;
    for (int j = 2; j <= n + 1; j++) {
      if (t[1][j] > 0 && t[1][j] > xmax) {
        p2 = j;
        xmax = t[1][j];
      }
    }
    for (int i = 2; i <= m + 1; i++) {
      v = fabs(t[i][1] / t[i][p2]);
      if (t[i][p2] < 0 && mn > v) {
        mn = v;
        p1 = i;
      }
    }
    std::swap(t[p1][0], t[0][p2]);
    for (int i = 1; i <= m + 1; i++) {
      if (i != p1) {
        for (int j = 1; j <= n + 1; j++) {
          if (j != p2) {
            t[i][j] -= t[p1][j]*t[i][p2] / t[p1][p2];
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
    done = true;
    for (int j = 2; j <= n + 1; j++) {
      if (t[1][j] > 0) {
        done = false;
      }
    }
  } while (!done);
  x->clear();
  for (int j = 1; j <= n; j++) {
    for (int i = 2; i <= m + 1; i++) {
      if (fabs(t[i][0] - j) < EPS) {
        x->push_back(t[i][1]);
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
  const int equations = 3, unknowns = 2;
  double a[equations][unknowns] = {{-2, 1}, {1, 0.85}, {1, 2}};
  double b[equations] = {0, 9, 14};
  double c[unknowns] = {3, 4};
  vector<vector<double>> va(equations, vector<double>(unknowns));
  vector<double> vb(b, b + equations), vc(c, c + unknowns), x;
  for (int i = 0; i < equations; i++) {
    for (int j = 0; j < unknowns; j++) {
      va[i][j] = a[i][j];
    }
  }
  assert(simplex_solve(va, vb, vc, &x) == 0);
  double maxval = 0;
  for (int i = 0; i < (int)x.size(); i++) {
    maxval += c[i]*x[i];
  }
  cout << "Solution = " << maxval << " at (" << x[0];
  for (int i = 1; i < (int)x.size(); i++) {
    cout << ", " << x[i];
  }
  cout << ")." << endl;
  return 0;
}
