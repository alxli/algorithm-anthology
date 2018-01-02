/*

The LU decomposition of a matrix a with row-partial pivoting is a factorization
of a (after some rows are possibly permuted by a permutation matrix p) as a
product of a lower triangular matrix l and an upper triangular matrix u. This
factorization can be used to tackle many common problems in linear algebra such
as solving systems of linear equations and computing determinants. An
improvement on basic row reduction, LU decomposition by row-partial pivoting
keeps the relative magnitude of matrix values small, thus reducing the relative
error due to rounding in computed solutions.

- lu_decompose(a, &p1col) assigns the r by c matrix a to merged LU decomposition
  matrix lu, returning either 0 or 1 denoting the "sign" of the permutation
  parity (0 if the number of overall row swaps performed is even, or 1 if it is
  odd), or -1 denoting a degenerate matrix (i.e. singular for square matrices).
  The merged matrix lu has lu[i][j] = l[i][j] for i > j and lu[i][j] = u[i][j]
  for i <= j. Note that the algorithm always yields an atomic lower triangular
  matrix for which the diagonal entries l[i][i] are always equal to 1, so this
  is not explicitly stored in the resulting merged matrix. For general i and j,
  the values of the lower and upper triangular matrices should be accessed via
  the getl(lu, i, j) and getu(lu, i, j) functions. Optionally, a vector<int>
  pointer p1col may be passed to return the permutation vector p1col where
  p1col[i] stores the only column that is equal to 1 in row i of the permutation
  matrix p (all other columns in row i of p are implicitly 0). The resulting
  permutation matrix p corresponding to p1col will satisfy p*a = l*u.
- solve_system(a, b, &x) solves the system of linear equations a*x = b given an
  r by c matrix a of real values, and a length r vector b, returning 0 if there
  is one solution or -1 if there is zero/infinite solutions. If there is exactly
  one solution, then the vector pointed to by x is populated with the solution
  vector of length c.
- det(a) returns the determinant of an n by n matrix a using LU decomposition.
- invert(a) assigns the n by n matrix a to its inverse (if it exists), returning
  0 if the inversion was successful or -1 if a has no inverse.

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
#include <limits>
#include <vector>

template<class Matrix>
int lu_decompose(Matrix &a, std::vector<int> *p1col = NULL,
                 const double EPS = 1e-10) {
  int r = a.size(), c = a[0].size(), parity = 0;
  if (p1col != NULL) {
    p1col->resize(r);
    for (int i = 0; i < r; i++) {
      (*p1col)[i] = i;
    }
  }
  for (int i = 0; i < r && i < c; i++) {
    int pi = i;
    for (int k = i + 1; k < r; k++) {
      if (fabs(a[k][i]) > fabs(a[pi][i])) {
        pi = k;
      }
    }
    if (fabs(a[pi][i]) < EPS) {
      return -1;
    }
    if (pi != i) {
      if (p1col != NULL) {
        std::iter_swap(p1col->begin() + i, p1col->begin() + pi);
      }
      std::iter_swap(a.begin() + i, a.begin() + pi);
      parity = 1 - parity;
    }
    for (int j = i + 1; j < r; j++) {
      a[j][i] /= a[i][i];
      for (int k = i + 1; k < c; k++) {
        a[j][k] -= a[j][i]*a[i][k];
      }
    }
  }
  return parity;
}

template<class Matrix>
double getl(const Matrix &lu, int i, int j) {
  return i > j ? lu[i][j] : (i < j ? 0 : 1);
}

template<class Matrix>
double getu(const Matrix &lu, int i, int j) {
  return i <= j ? lu[i][j] : 0;
}

template<class Matrix, class T>
int solve_system(const Matrix &a, const std::vector<T> &b, std::vector<T> *x,
                 const double EPS = 1e-10) {
  int r = a.size(), c = a[0].size();
  if (x == NULL || a.empty() || a.size() != b.size() || r < c) {
    return -1;
  }
  x->resize(c);
  std::vector<int> p1col;
  Matrix lu;
  int status = lu_decompose(lu = a, &p1col, EPS);
  if (status < 0) {
    return status;
  }
  for (int i = 0; i < c; i++) {
    (*x)[i] = b[p1col[i]];
    for (int k = 0; k < i; k++) {
      (*x)[i] -= getl(lu, i, k)*(*x)[k];
    }
  }
  for (int i = c - 1; i >= 0; i--) {
    for (int k = i + 1; k < c; k++) {
      (*x)[i] -= getu(lu, i, k)*(*x)[k];
    }
    (*x)[i] /= getu(lu, i, i);
  }
  for (int i = 0; i < r; i++) {
    double val = 0;
    for (int j = 0; j < c; j++) {
      val += a[i][j]*(*x)[j];
    }
    if (fabs(val - b[i])/b[i] > EPS) {
      return -1;
    }
  }
  return 0;
}

template<class SquareMatrix>
double det(const SquareMatrix &a) {
  int n = a.size();
  SquareMatrix lu;
  int status = lu_decompose(lu = a);
  if (status < 0) {
    return 0;
  }
  double res = 1;
  for (int i = 0; i < n; i++) {
    res *= lu[i][i];
  }
  return status == 0 ? res : -res;
}

template<class SquareMatrix>
int invert(SquareMatrix &a) {
  int n = a.size();
  std::vector<int> p1col;
  int status = lu_decompose(a, &p1col);
  if (status < 0) {
    return status;
  }
  SquareMatrix ia(n, typename SquareMatrix::value_type(n, 0));
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < n; i++) {
      if (p1col[i] == j) {
        ia[i][j] = 1.0;
      } else {
        ia[i][j] = 0.0;
      }
      for (int k = 0; k < i; k++) {
        ia[i][j] -= getl(a, i, k)*ia[k][j];
      }
    }
    for (int i = n - 1; i >= 0; i--) {
      for (int k = i + 1; k < n; k++) {
        ia[i][j] -= getu(a, i, k)*ia[k][j];
      }
      ia[i][j] /= getu(a, i, i);
    }
  }
  a.swap(ia);
  return 0;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  { // Solve a system.
    const int equations = 3, unknowns = 3;
    const int a[equations][unknowns] = {{-1, 2, 5}, {1, 0, -6}, {-4, 2, 2}};
    const int b[equations] = {3, 1, -2};
    vector<vector<double> > m(equations);
    for (int i = 0; i < equations; i++) {
      m[i].assign(a[i], a[i] + unknowns);
    }
    vector<double> x;
    assert(solve_system(m, vector<double>(b, b + equations), &x) == 0);
    for (int i = 0; i < equations; i++) {
      double sum = 0;
      for (int j = 0; j < unknowns; j++) {
        sum += a[i][j]*x[j];
      }
      assert(fabs(sum - b[i]) < 1e-10);
    }
  }
  { // Find the determinant.
    const int n = 3, a[n][n] = {{1, 3, 5}, {2, 4, 7}, {1, 1, 0}};
    vector<vector<double> > m(n);
    for (int i = 0; i < n; i++) {
      m[i] = vector<double>(a[i], a[i] + n);
    }
    assert(fabs(det(m) - 4) < 1e-10);
  }
  { // Find the inverse.
    const int n = 3, a[n][n] = {{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
    vector<vector<double> > m(n), res(n, vector<double>(n, 0));
    for (int i = 0; i < n; i++) {
      m[i] = vector<double>(a[i], a[i] + n);
    }
    assert(invert(m) == 0);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
          res[i][j] += a[i][k]*m[k][j];
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
