/*

Description: The canonical form of a linear programming
problem is to maximize c^T*x, subject to Ax <= b, and x >= 0.
where x is the vector of variables (to be solved), c and b
are vectors of (known) coefficients, A is a (known) matrix of
coefficients, and (.)^T is the matrix transpose. The following
implementation solves n variables in a system of m constraints.

Precondition: ab has dimensions m by n+1 and c has length n+1.

Complexity: The simplex method is remarkably efficient in
practice, usually taking 2m or 3m iterations, converging in
expected polynomial time for certain distributions of random
inputs. However, its worst-case complexity is exponential,
and can be demonstrated with carefully constructed examples.

*/

#include <algorithm> /* std::swap() */
#include <cfloat>    /* DBL_MAX */
#include <cmath>     /* fabs() */
#include <vector>

typedef std::vector<double> vd;
typedef std::vector<vd> vvd;

//ab[i][0..n-1] stores A and ab[i][n] stores B
vd simplex(const vvd & ab, const vd & c, bool max = true) {
  const double eps = 1e-10;
  int n = c.size() - 1, m = ab.size();
  vvd ts(m + 2, vd(n + 2));
  ts[1][1] = max ? c[n] : -c[n];
  for (int j = 1; j <= n; j++)
    ts[1][j + 1] = max ? c[j - 1] : -c[j - 1];
  for (int i = 1; i <= m; i++) {
    for (int j = 1; j <= n; j++)
      ts[i + 1][j + 1] = -ab[i - 1][j - 1];
    ts[i + 1][1] = ab[i - 1][n];
  }
  for (int j = 1; j <= n; j++)
    ts[0][j + 1] = j;
  for (int i = n + 1; i <= n + m; i++)
    ts[i - n + 1][0] = i;
  double p1 = 0.0, p2 = 0.0;
  bool done = true;
  do {
    double mn = DBL_MAX, xmax = 0.0, v;
    for (int j = 2; j <= n + 1; j++)
      if (ts[1][j] > 0.0 && ts[1][j] > xmax) {
        p2 = j;
        xmax = ts[1][j];
      }
    for (int i = 2; i <= m + 1; i++) {
      v = fabs(ts[i][1] / ts[i][p2]);
      if (ts[i][p2] < 0.0 && mn > v) {
        mn = v;
        p1 = i;
      }
    }
    std::swap(ts[p1][0], ts[0][p2]);
    for (int i = 1; i <= m + 1; i++) {
      if (i == p1) continue;
      for (int j = 1; j <= n + 1; j++)
        if (j != p2)
          ts[i][j] -= ts[p1][j] * ts[i][p2] / ts[p1][p2];
    }
    ts[p1][p2] = 1.0 / ts[p1][p2];
    for (int j = 1; j <= n + 1; j++) {
      if (j != p2)
        ts[p1][j] *= fabs(ts[p1][p2]);
    }
    for (int i = 1; i <= m + 1; i++) {
      if (i != p1)
        ts[i][p2] *= ts[p1][p2];
    }
    for (int i = 2; i <= m + 1; i++)
      if (ts[i][1] < 0.0) return vd(); //no solution
    done = true;
    for (int j = 2; j <= n + 1; j++)
      if (ts[1][j] > 0) done = false;
  } while (!done);
  vd res;
  for (int i = 1; i <= n; i++)
    for (int j = 2; j <= m + 1; j++)
      if (fabs(ts[j][0] - i) <= eps)
        res.push_back(ts[j][1]);
  //the solution is stored in ts[1][1]
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

/*
  Maximize 3x + 4y + 5, subject to x, y >= 0 and:
      -2x +    1y <=  0
       1x + 0.85y <=  9
       1x +    2y <= 14

  Note: The solution is 38.3043 at (5.30435, 4.34783).
*/

int main() {
  const int n = 2, m = 3;
  double ab[m][n + 1] = {{-2, 1, 0}, {1, 0.85, 9}, {1, 2, 14}};
  double c[n + 1] = {3, 4, 5};
  vvd vab(m, vd(n + 1));
  vd vc(c, c + n + 1);
  for (int i = 0; i < m; i++) {
    for (int j = 0; j <= n; j++)
      vab[i][j] = ab[i][j];
  }
  vd x = simplex(vab, vc);
  if (x.empty()) {
    cout << "No solution.\n";
  } else {
    double solval = c[n];
    for (int i = 0; i < (int)x.size(); i++)
      solval += c[i] * x[i];
    cout << "Solution = " << solval;
    cout << " at (" << x[0];
    for (int i = 1; i < (int)x.size(); i++)
      cout << ", " << x[i];
    cout << ").\n";
  }
  return 0;
}
