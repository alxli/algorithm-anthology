/*

Computes the determinant, inverse, and adjugate (adjoint) of a square matrix.

- det_naive(a) returns the determinant of an n by n matrix a, using the classic
  divide-and-conquer algorithm by Laplace expansions.
- det(a) returns the determinant of an n by n matrix a using Gaussian
  elimination.
- inverse(a) assigns the n by n matrix a to its inverse (if it exists),
  returning a reference to the modified argument itself. Note that the inverse
  exists (that is, a is "invertible" or "non-singular") if and only if its
  determinant is non-zero, which should be checked prior to calling this
  function. Otherwise if a is not invertible, then its assigned values after the
  function call will be undefined (likely +/-Inf or NaN).
- adjugate(a) assigns the n by n matrix a to its adjugate (if it exists),
  returning a reference to the modified argument itself. Note that the inverse
  exists (that is, a is "invertible" or "non-singular") if and only if its
  determinant is non-zero, which should be checked prior to calling this
  function. Otherwise if a is not invertible, then its assigned values after the
  function call will be undefined (likely +/-Inf or NaN).

Time Complexity:
- O(n!) per call to det_naive(), where n is the dimension of the matrix.
- O(n^3) per call to det(), inverse(), and adjugate() where n is the dimension
  of the matrix.

Space Complexity:
- O(n) auxiliary stack space and O(n!*n) auxiliary heap space for det_naive(),
  where n is the dimension of the matrix.
- O(n^2) auxiliary for det(), inverse(), and adjugate().

*/

#include <cmath>
#include <map>
#include <vector>

template<class SquareMatrix>
double det_naive(const SquareMatrix &a) {
  int n = a.size();
  if (n == 1) {
    return a[0][0];
  }
  if (n == 2) {
    return a[0][0]*a[1][1] - a[0][1]*a[1][0];
  }
  double res = 0;
  std::vector<std::vector<double> > temp(n - 1, std::vector<double>(n - 1));
  for (int p = 0; p < n; p++) {
    int h = 0, k = 0;
    for (int i = 1; i < n; i++) {
      for (int j = 0; j < n; j++) {
        if (j == p) {
          continue;
        }
        temp[h][k++] = a[i][j];
        if (k == n - 1) {
          h++;
          k = 0;
        }
      }
    }
    res += (p % 2 == 0 ? 1 : -1)*a[0][p]*det_naive(temp);
  }
  return res;
}

template<class SquareMatrix>
double det(const SquareMatrix &a, double eps = 1e-10) {
  SquareMatrix b(a);
  int n = a.size();
  double res = 1.0;
  std::vector<bool> used(n, false);
  for (int i = 0; i < n; i++) {
    int p;
    for (p = 0; p < n; p++) {
      if (!used[p] && fabs(b[p][i]) > eps) {
        break;
      }
    }
    if (p >= n) {
      return 0;
    }
    res *= b[p][i];
    used[p] = true;
    double z = 1.0/b[p][i];
    for (int j = 0; j < n; j++) {
      b[p][j] *= z;
    }
    for (int j = 0; j < n; j++) {
      if (j != p) {
        z = b[j][i];
        for (int k = 0; k < n; k++) {
          b[j][k] -= z*b[p][k];
        }
      }
    }
  }
  return res;
}

template<class SquareMatrix>
SquareMatrix& inverse(SquareMatrix &a) {
  int n = a.size();
  for (int i = 0; i < n; i++) {
    a[i].resize(2*n);
    for (int j = n; j < n*2; j++) {
      a[i][j] = (i == j - n ? 1 : 0);
    }
  }
  for (int i = 0; i < n; i++) {
    double z = a[i][i];
    for (int j = i; j < n*2; j++) {
      a[i][j] /= z;
    }
    for (int j = 0; j < n; j++) {
      if (i != j) {
        double z = a[j][i];
        for (int k = 0; k < n*2; k++) {
          a[j][k] -= z*a[i][k];
        }
      }
    }
  }
  for (int i = 0; i < n; i++) {
    a[i].erase(a[i].begin(), a[i].begin() + n);
  }
  return a;
}

template<class SquareMatrix>
SquareMatrix& adjugate(SquareMatrix &a) {
  int n = a.size();
  double d = det(a);
  inverse(a);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      a[i][j] *= d;
    }
  }
  return a;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

template<class M>
M mul(const M &a, const M &b) {
  int n = a.size();
  M res(n, vector<double>(n, 0));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        res[i][j] += a[i][k]*b[k][j];
      }
    }
  }
  return res;
}

int main() {
  const int n = 3, a[n][n] = {{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
  vector<vector<double> > m(n), inv, adj, res;
  for (int i = 0; i < n; i++) {
    m[i] = vector<double>(a[i], a[i] + n);
  }
  double d = det(m);
  assert(fabs(d - det_naive(m)) < 1e-10);
  res = mul(m, inverse(inv = m));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      assert(fabs(res[i][j] - (i == j ? 1 : 0)) < 1e-10);
    }
  }
  res = mul(m, adjugate(adj = m));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      assert(fabs(res[i][j] - (i == j ? d : 0)) < 1e-10);
    }
  }
  return 0;
}
