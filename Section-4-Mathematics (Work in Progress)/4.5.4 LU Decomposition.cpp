/*

The LU (lower/upper) decomposition of a matrix a is a factorization of a as the
product of a lower triangular matrix l and an upper triangular matrix u. This
factorization can be used to easily solve many common problems in linear algebra
such as solving a system of linear equations or computing the determinant.

- lu_decompose(a, &detsign, &perm) assigns the matrix a to its LU decomposition,
  returning a reference to the modified argument itself. Optionally, an int
  pointer detsign may be passed to store the sign of the determinant of a.
  Optionally, an vector<int> pointer may be passed to store the permutation
  vector perm. The resulting merged LU matrix m will have m[i][j] = l[i][j] for
  i > j and m[i][j] = u[i][j] for i <= j. Note that the result will have l[i][i]
  equal to 1 for every i from 1 to n, but this is not stored in m. The values of
  the actual lower and upper triangular matrices can be accessed via the
  getl(lu, i, j) and getu(lu, i, j) functions respectively.
- solve_system(a, b) returns an r by c matrix x such that a*x = b for a given
  r by r matrix a and r by c matrix b. Note that the number of columns m can be
  equal to 1 for b and x, making them column vectors (though they will still be
  represented as the Matrix template type).
- det(a) returns the determinant of an n by n matrix a.
- inverse(a) returns the inverse of the n by n matrix a (if it exists). If a is
  not invertible, then the values of the returned matrix will be undefined
  (+/-Inf or NaN).

Time Complexity:
- O(r^2*c) per call to lu_decompose(a) and solve_system(a, b), where r and c are
  the number of rows and columns respectively, in accordance to the functions'
  descriptions above.
- O(n^3) per call to det(a) and inverse(a), where n is the dimension of a.

Space Complexity:
- O(1) auxiliary for lu_decompose().
- O(n^2) for det(a) and inverse(a).
- O(r*c) auxiliary heap space for solve_system(a, b).

*/

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <vector>

template<class Matrix>
Matrix& lu_decompose(Matrix &a, int *detsign = NULL,
                     std::vector<int> *perm = NULL,
                     const double EPS = 1e-10) {
  int r = a.size(), c = a[0].size(), sign = 1;
  if (perm != NULL) {
    perm->resize(r);
    for (int i = 0; i < r; i++) {
      (*perm)[i] = i;
    }
  }
  for (int i = 0, j = 0; i < r && j < c; i++, j++) {
    int pi = i;
    for (int k = i + 1; k < r; k++) {
      if (fabs(a[k][j]) > fabs(a[pi][j])) {
        pi = k;
      }
    }
    if (fabs(a[pi][j]) < EPS) {
      i--;
      continue;
    }
    if (pi != i) {
      if (perm != NULL) {
        std::iter_swap(perm->begin() + i, perm->begin() + pi);
      }
      sign = -sign;
      for (int k = 0; k < c; k++) {
        std::swap(a[i][k], a[pi][k]);
      }
    }
    for (int s = i + 1; s < r; s++) {
      a[s][j] /= a[i][j];
      for (int d = j + 1; d < c; d++) {
        a[s][d] -= a[s][j]*a[i][d];
      }
    }
  }
  if (detsign != NULL) {
    *detsign = sign;
  }
  return a;
}

template<class Matrix>
double getl(const Matrix &lu, int i, int j) {
  return i > j ? lu[i][j] : (i < j ? 0 : 1);
}

template<class Matrix>
double getu(const Matrix &lu, int i, int j) {
  return i <= j ? lu[i][j] : 0;
}

template<class Matrix>
Matrix solve_system(const Matrix &a, const Matrix &b) {
  int r = b.size(), c = b[0].size();
  std::vector<int> perm;
  Matrix lu;
  lu_decompose(lu = a, NULL, &perm);
  Matrix y(r, std::vector<double>(c)), x(r, std::vector<double>(c));
  for (int j = 0; j < c; j++) {
    y[0][j] = b[perm[0]][j]/getl(lu, 0, 0);
    for (int i = 1; i < r; i++) {
      double s = 0;
      for (int k = 0; k < i; k++) {
        s += getl(lu, i, k)*y[k][j];
      }
      y[i][j] = (b[perm[i]][j] - s)/getl(lu, i, i);
    }
  }
  for (int j = 0; j < c; j++) {
    x[r - 1][j] = y[r - 1][j]/getu(lu, r - 1, r - 1);
    for (int i = r - 2; i >= 0; i--) {
      double s = 0;
      for (int k = i + 1; k < r; k++) {
        s += getu(lu, i, k)*x[k][j];
      }
      x[i][j] = (y[i][j] - s)/getu(lu, i, i);
    }
  }
  return x;
}

template<class SquareMatrix>
double det(const SquareMatrix &a) {
  int n = a.size(), sign;
  SquareMatrix lu;
  lu_decompose(lu = a, &sign);
  double res = 1;
  for (int i = 0; i < n; i++) {
    res *= lu[i][i];
  }
  return sign < 0 ? -res : res;
}

template<class SquareMatrix>
SquareMatrix inverse(const SquareMatrix &a) {
  int n = a.size();
  SquareMatrix I(n, typename SquareMatrix::value_type(n, 0));
  for (int i = 0; i < n; i++) {
    I[i][i] = 1;
  }
  return solve_system(a, I);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  { // Find the determinant.
    const int n = 3, a[n][n] = {{1, 3, 5}, {2, 4, 7}, {1, 1, 0}};
    vector<vector<double> > m(n);
    for (int i = 0; i < n; i++) {
      m[i] = vector<double>(a[i], a[i] + n);
    }
    assert(fabs(det(m) - 4) < 1e-10);
  }
  { // Solve for [x, y] in x + 3y = 4 && 2x + 3y = 6
    const int n = 2, a[n][n] = {{1, 3}, {2, 3}}, b[n] = {4, 6};
    vector<vector<double> > ma(n), mb(n), x;
    for (int i = 0; i < n; i++) {
      ma[i] = vector<double>(a[i], a[i] + n);
      mb[i] = vector<double>(1, b[i]);
    }
    x = solve_system(ma, mb);
    for (int i = 0; i < n; i++) {
      assert(fabs(a[i][0]*x[0][0] + a[i][1]*x[1][0] - b[i]) < 1e-10);
    }
  }
  { // Find the inverse by solving a system.
    const int n = 3, a[n][n] = {{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
    vector<vector<double> > m(n), inv, res(n, vector<double>(n, 0));
    for (int i = 0; i < n; i++) {
      m[i] = vector<double>(a[i], a[i] + n);
    }
    inv = inverse(m);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
          res[i][j] += a[i][k]*inv[k][j];
        }
      }
    }
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        assert(fabs(res[i][j] - (i == j ? 1 : 0)) < 1e-10);
      }
    }
  }
  return 0;
}
