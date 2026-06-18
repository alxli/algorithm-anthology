/*

Computes the determinant and inverse of a square matrix using Gaussian elimination. The inverse of a
matrix $a$ is another matrix $b$ such that $ab$ equals the identity matrix. The inverse of $a$
exists if and only if the determinant of $a$ is nonzero. In this case, $a$ is called invertible or
non-singular. The determinant falls out of elimination as the product of the pivots, negated once
per row swap. The inverse is found by appending the identity matrix and row-reducing the combined
matrix: the operations that turn $a$ into the identity turn the identity into the inverse. In
practice, simple Gaussian elimination is prone to rounding error on certain matrices. For a more
accurate algorithm for solving systems of linear equations, use LU decomposition with row partial
pivoting. For an integer matrix whose determinant is wanted exactly, the Bareiss algorithm runs a
fraction-free elimination: every intermediate value is itself a determinant of a submatrix, so the
divisions are always exact and the arithmetic stays in integers.

- `det_naive(a)` returns the determinant of an $n$ by $n$ matrix `a`, using the classic
  divide-and-conquer algorithm by Laplace expansions. It is division-free and computes in the
  matrix's element type, so an integer matrix yields an exact integer determinant; but at O(n!) it
  is only practical for tiny `n`, so prefer `det_bareiss` for exact integer determinants.
- `det(a)` returns the determinant of an $n$ by $n$ matrix `a` using Gaussian elimination.
- `det_bareiss(a)` returns the exact determinant of an integer matrix `a` using fraction-free
  elimination, with no rounding error. Intermediate values are bounded by the magnitudes of the
  matrix's minors, so the entries must stay within `int64_t`; substitute a big-integer type for
  large matrices.
- `invert(a)` assigns the $n$ by $n$ matrix `a` to its inverse (if it exists), returning a reference
  to the modified argument itself. If `a` is not invertible, then its assigned values after the
  function call will be undefined (`+/-Inf` or `+/-NaN`).

Time Complexity:
- O(n!) per call to `det_naive()`, where $n$ is the dimension of the matrix.
- O(n^3) per call to `det()`, `det_bareiss()`, and `invert()` where $n$ is the dimension of the
  matrix.

Space Complexity:
- O(n) auxiliary stack space and O(n!*n) auxiliary heap space for `det_naive()`, where $n$ is the
  dimension of the matrix.
- O(n^2) auxiliary heap space for `det()`, `det_bareiss()`, and `invert()`.

*/

#include <cmath>
#include <cstdint>
#include <type_traits>
#include <vector>

template<typename SquareMatrix>
auto det_naive(const SquareMatrix &a) {
  int n = static_cast<int>(a.size());
  if (n == 1) {
    return a[0][0];
  }
  if (n == 2) {
    return a[0][0] * a[1][1] - a[0][1] * a[1][0];
  }
  std::decay_t<decltype(a[0][0])> res = 0;
  SquareMatrix temp(n - 1, typename SquareMatrix::value_type(n - 1));
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
    res += (p % 2 == 0 ? 1 : -1) * a[0][p] * det_naive(temp);
  }
  return res;
}

template<typename SquareMatrix>
double det(const SquareMatrix &a, double EPS = 1e-10) {
  int n = static_cast<int>(a.size());
  SquareMatrix b(a);
  double res = 1.0;
  for (int i = 0; i < n; i++) {
    // Partial pivoting: bring the largest-magnitude entry in column i to the diagonal. Each row
    // swap negates the determinant, so the swap sign must be tracked. Selecting pivot rows out of
    // order without this sign correction can return |det| with the wrong sign.
    int p = i;
    for (int r = i + 1; r < n; r++) {
      if (fabs(b[r][i]) > fabs(b[p][i])) {
        p = r;
      }
    }
    if (fabs(b[p][i]) < EPS) {
      return 0;
    }
    if (p != i) {
      std::swap(b[p], b[i]);
      res = -res;
    }
    res *= b[i][i];
    for (int j = i + 1; j < n; j++) {
      double z = b[j][i] / b[i][i];
      for (int k = i; k < n; k++) {
        b[j][k] -= z * b[i][k];
      }
    }
  }
  return res;
}

int64_t det_bareiss(std::vector<std::vector<int64_t>> a) {
  int n = static_cast<int>(a.size());
  int64_t prev = 1, sign = 1;
  for (int k = 0; k < n; k++) {
    if (a[k][k] == 0) {  // Swap in a nonzero pivot; each swap flips the sign.
      int p = -1;
      for (int r = k + 1; r < n; r++) {
        if (a[r][k] != 0) {
          p = r;
          break;
        }
      }
      if (p == -1) {
        return 0;
      }
      std::swap(a[k], a[p]);
      sign = -sign;
    }
    for (int i = k + 1; i < n; i++) {
      for (int j = k + 1; j < n; j++) {
        // The division by the previous pivot is always exact (Bareiss's theorem).
        a[i][j] = (a[i][j] * a[k][k] - a[i][k] * a[k][j]) / prev;
      }
    }
    prev = a[k][k];
  }
  return n == 0 ? 1 : sign * a[n - 1][n - 1];
}

template<typename SquareMatrix>
SquareMatrix &invert(SquareMatrix &a) {
  int n = static_cast<int>(a.size());
  for (int i = 0; i < n; i++) {
    a[i].resize(2 * n);
    for (int j = n; j < n * 2; j++) {
      a[i][j] = (i == j - n ? 1 : 0);
    }
  }
  for (int i = 0; i < n; i++) {
    // Partial pivoting: without it a zero on the diagonal divides by zero and fails even for an
    // invertible matrix (e.g. a permutation matrix). Picking the largest-magnitude pivot also
    // improves numerical stability.
    int p = i;
    for (int r = i + 1; r < n; r++) {
      if (fabs(a[r][i]) > fabs(a[p][i])) {
        p = r;
      }
    }
    std::swap(a[p], a[i]);
    double z = a[i][i];
    for (int j = i; j < n * 2; j++) {
      a[i][j] /= z;
    }
    for (int j = 0; j < n; j++) {
      if (i != j) {
        double z = a[j][i];
        for (int k = 0; k < n * 2; k++) {
          a[j][k] -= z * a[i][k];
        }
      }
    }
  }
  for (int i = 0; i < n; i++) {
    a[i].erase(a[i].begin(), a[i].begin() + n);
  }
  return a;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<double>> a{{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
  int n = static_cast<int>(a.size());
  vector<vector<double>> res(n, vector<double>(n, 0));
  double d = det(a);
  assert(fabs(d - det_naive(a)) < 1e-10);

  // Bareiss gives the determinant of an integer matrix exactly, with no rounding.
  vector<vector<int64_t>> ai{{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
  assert(det_naive(ai) == -306);  // Division-free, so also exact in the int64_t element type.
  assert(det_bareiss(ai) == -306);
  assert(det_bareiss({{2, 0, 0}, {0, 3, 0}, {0, 0, 5}}) == 30);
  assert(det_bareiss({{1, 2}, {2, 4}}) == 0);  // Singular.
  vector<vector<double>> inv = a;
  invert(inv);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        res[i][j] += a[i][k] * inv[k][j];
      }
    }
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      assert(fabs(res[i][j] - (i == j ? 1 : 0)) < 1e-10);
    }
  }
  return 0;
}
