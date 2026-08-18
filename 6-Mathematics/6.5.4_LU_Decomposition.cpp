/*

The LU decomposition of a matrix $A$ with row-partial pivoting is a factorization of $A$ (after some
rows are possibly permuted by a permutation matrix $p$) as a product of a lower triangular matrix
$L$ and an upper triangular matrix $U$. This factorization can be used to tackle many common
problems in linear algebra such as solving systems of linear equations and computing determinants.
An improvement on basic row reduction, LU decomposition by row-partial pivoting keeps the relative
magnitude of matrix values small, thus reducing the relative error due to rounding in computed
solutions. These routines require floating-point matrix elements; use 6.5.3 for exact-field or
integer determinant calculations.

- `lu_decompose(a, &perm, eps = 1e-10)` assigns floating-point matrix `a` to merged LU decomposition
  matrix `lu`; it returns $0$ for even row-swap parity, $1$ for odd parity, or $-1$ for a degenerate
  matrix (i.e. singular for square matrices). The merged matrix stores `l[i][j]` in `lu[i][j]` when
  `i > j`, and `u[i][j]` otherwise. The diagonal entries `l[i][i]` are always $1$, so they are not
  explicitly stored. Access general entries of the lower and upper triangular matrices via
  `getl(lu, i, j)` and `getu(lu, i, j)`. Optional output vector `perm` represents $p$: `perm[i]` is
  the only column containing $1$ in row `i`. Left-multiplying `a` by this permutation matrix gives
  the product of the separate lower and upper triangular matrices, not the merged storage matrix
  `lu` itself.
- `solve_system(a, b, &x, eps = 1e-10)` solves the system of linear equations $Ax = b$ given an $m$
  by $n$ floating-point matrix `a` and a length $m$ vector `b`, returning $0$ if there is one
  solution or $-1$ if there are zero or infinite solutions. If there is exactly one solution, then
  the output vector `x` is populated with the solution of length $n$; otherwise, `x` is unchanged.
- `det(a)` returns the determinant of an $n$ by $n$ floating-point matrix `a` using LU
  decomposition.
- `inverse(a)` returns the inverse of the $n$ by $n$ floating-point matrix `a`, or `std::nullopt` if
  `a` is singular.

Time Complexity:
- O(m*n*min(m, n)) per call to `lu_decompose(a)`, where $m$ and $n$ are the numbers of rows and
  columns of `a`, respectively.
- O(m*n^2) per call to `solve_system()`, which requires $m \geq n$.
- O(n^3) per call to `det(a)` and `inverse(a)`, where $n$ is the length of square matrix `a`.

Space Complexity:
- O(1) auxiliary for `lu_decompose()`.
- O(n^2) auxiliary for `det()` and `inverse()`.
- O(m*n) auxiliary for `solve_system()`.

*/

#include <algorithm>
#include <cmath>
#include <numeric>
#include <optional>
#include <type_traits>
#include <vector>

template<typename Matrix>
int lu_decompose(Matrix &a, std::vector<int> *perm = nullptr, double eps = 1e-10) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  int parity = 0;
  if (perm != nullptr) {
    perm->resize(rows);
    std::iota(perm->begin(), perm->end(), 0);
  }
  for (int i = 0; i < rows && i < cols; i++) {
    int pi = i;
    for (int k = i + 1; k < rows; k++) {
      if (std::fabs(a[k][i]) > std::fabs(a[pi][i])) {
        pi = k;
      }
    }
    if (std::fabs(a[pi][i]) < eps) {
      return -1;
    }
    if (pi != i) {
      if (perm != nullptr) {
        std::iter_swap(perm->begin() + i, perm->begin() + pi);
      }
      std::iter_swap(a.begin() + i, a.begin() + pi);
      parity = 1 - parity;
    }
    for (int j = i + 1; j < rows; j++) {
      a[j][i] /= a[i][i];
      for (int k = i + 1; k < cols; k++) {
        a[j][k] -= a[j][i] * a[i][k];
      }
    }
  }
  return parity;
}

template<typename Matrix>
auto getl(const Matrix &lu, int i, int j) {
  using T = std::decay_t<decltype(lu[0][0])>;
  return i > j ? lu[i][j] : T(i == j);
}

template<typename Matrix>
auto getu(const Matrix &lu, int i, int j) {
  using T = std::decay_t<decltype(lu[0][0])>;
  return i <= j ? lu[i][j] : T{0};
}

template<typename Matrix, typename T>
int solve_system(const Matrix &a, const std::vector<T> &b, std::vector<T> *x, double eps = 1e-10) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  if (x == nullptr || a.empty() || a.size() != b.size() || rows < cols) {
    return -1;
  }
  Matrix lu(a);
  std::vector<int> perm;
  int status = lu_decompose(lu, &perm, eps);
  if (status < 0) {
    return status;
  }
  std::vector<T> solution(cols);
  for (int i = 0; i < cols; i++) {
    solution[i] = b[perm[i]];
    for (int k = 0; k < i; k++) {
      solution[i] -= getl(lu, i, k) * solution[k];
    }
  }
  for (int i = cols - 1; i >= 0; i--) {
    for (int k = i + 1; k < cols; k++) {
      solution[i] -= getu(lu, i, k) * solution[k];
    }
    solution[i] /= getu(lu, i, i);
  }
  for (int i = 0; i < rows; i++) {
    T val = 0;
    for (int j = 0; j < cols; j++) {
      val += a[i][j] * solution[j];
    }
    // Mixed absolute/relative tolerance: dividing by b[i] alone would skip the check for negative
    // b[i] (ratio goes negative) and divide by zero when b[i] == 0.
    if (std::fabs(val - b[i]) > eps * (1.0 + std::fabs(b[i]))) {
      return -1;
    }
  }
  x->swap(solution);
  return 0;
}

template<typename Matrix>
auto det(const Matrix &a) {
  using T = std::decay_t<decltype(a[0][0])>;
  int n = static_cast<int>(a.size());
  Matrix lu(a);
  int status = lu_decompose(lu);
  if (status < 0) {
    return T{0};
  }
  T res = 1;
  for (int i = 0; i < n; i++) {
    res *= lu[i][i];
  }
  return status == 0 ? res : -res;
}

template<typename SquareMatrix>
std::optional<SquareMatrix> inverse(SquareMatrix a) {
  int n = static_cast<int>(a.size());
  std::vector<int> perm;
  int status = lu_decompose(a, &perm);
  if (status < 0) {
    return std::nullopt;
  }
  SquareMatrix ia(n, typename SquareMatrix::value_type(n, 0));
  for (int j = 0; j < n; j++) {
    for (int i = 0; i < n; i++) {
      if (perm[i] == j) {
        ia[i][j] = 1.0;
      } else {
        ia[i][j] = 0.0;
      }
      for (int k = 0; k < i; k++) {
        ia[i][j] -= getl(a, i, k) * ia[k][j];
      }
    }
    for (int i = n - 1; i >= 0; i--) {
      for (int k = i + 1; k < n; k++) {
        ia[i][j] -= getu(a, i, k) * ia[k][j];
      }
      ia[i][j] /= getu(a, i, i);
    }
  }
  return ia;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  {  // Solve a system.
    vector<vector<double>> a{{-1, 2, 5}, {1, 0, -6}, {-4, 2, 2}};
    vector<double> b{3, 1, -2};
    vector<double> x;
    assert(solve_system(a, b, &x) == 0);
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      double sum = 0;
      for (int j = 0; j < static_cast<int>(a[0].size()); j++) {
        sum += a[i][j] * x[j];
      }
      assert(EQ(sum, b[i]));
    }
  }
  {  // Find the determinant.
    vector<vector<double>> a{{1, 3, 5}, {2, 4, 7}, {1, 1, 0}};
    assert(EQ(det(a), 4));
  }
  {  // Find the inverse.
    vector<vector<double>> a{{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
    auto inv = inverse(a);
    int n = static_cast<int>(a.size());
    vector<vector<double>> res(n, vector<double>(n));
    assert(inv);
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        for (int k = 0; k < n; k++) {
          res[i][j] += a[i][k] * (*inv)[k][j];
        }
      }
    }
    for (int i = 0; i < n; i++) {
      for (int j = 0; j < n; j++) {
        assert(EQ(res[i][j], i == j ? 1 : 0));
      }
    }
  }
  {  // Singular matrices have no inverse.
    vector<vector<double>> singular{{1, 2}, {2, 4}};
    assert(!inverse(singular));
    // Failed operations leave their output arguments unchanged.
    vector<vector<double>> inconsistent{{1}, {1}};
    vector<double> b{1, 2}, x{99, 100};
    assert(solve_system(inconsistent, b, &x) == -1);
    assert((x == vector<double>{99, 100}));
  }
  return 0;
}
