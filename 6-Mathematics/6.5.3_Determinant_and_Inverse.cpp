/*

Computes the determinant and inverse of a square matrix using Gaussian elimination. The inverse of a
matrix $a$ is another matrix $b$ such that $ab$ equals the identity matrix. The inverse of $a$
exists if and only if the determinant of $a$ is nonzero. In this case, $a$ is called invertible or
non-singular. The determinant falls out of elimination as the product of the pivots, negated once
per row swap. The inverse is found by appending the identity matrix and row-reducing the combined
matrix: the operations that turn $a$ into the identity turn the identity into the inverse.

Floating-point matrices use the largest-magnitude pivot in each column, although Gaussian
elimination can still suffer from rounding error; use LU decomposition for greater numerical
stability. Exact field types such as `Modular<MOD>` use any nonzero pivot and perform exact
arithmetic. These types must support construction from $0$ and $1$, equality, addition, subtraction,
multiplication, and division. For an integer matrix whose determinant is wanted exactly, the Bareiss
algorithm runs a fraction-free elimination: every intermediate value is itself a determinant of a
submatrix, so the divisions are always exact and the arithmetic stays in integers.

- `det_naive(a)` returns the determinant of an $n$ by $n$ matrix `a`, using the classic
  divide-and-conquer algorithm by Laplace expansions. It is division-free and computes in the
  matrix's element type, so an integer matrix yields an exact integer determinant; but at O(n!) it
  is only practical for tiny $n$, so prefer `det_bareiss` for exact integer determinants.
- `det(a, eps = 1e-10)` returns the determinant of an $n$ by $n$ floating-point or exact-field
  matrix `a` using Gaussian elimination. Floating-point pivots within `eps` of zero are treated as
  singular; exact fields use equality with zero.
- `det_bareiss(a)` returns the exact determinant of an integer matrix `a` using fraction-free
  elimination, with no rounding error. Stored entries are minors of `a`, but the products in each
  update can be larger. They use 128-bit intermediates when available; otherwise those products must
  fit in `int64_t`.
- `inverse(a, eps = 1e-10)` returns the inverse of square matrix `a` with floating-point or
  exact-field elements, or `std::nullopt` if the matrix is singular. Floating-point pivots within
  `eps` of zero are treated as singular.

Time Complexity:
- O(n!) per call to `det_naive()`, where $n$ is the dimension of the matrix.
- O(n^3) per call to `det()`, `det_bareiss()`, and `inverse()` where $n$ is the dimension of the
  matrix.

Space Complexity:
- O(n) auxiliary stack space and O(n^3) auxiliary heap space for `det_naive()`, where $n$ is the
  dimension of the matrix.
- O(n^2) auxiliary heap space for `det()`, `det_bareiss()`, and `inverse()`.

*/

#include <cmath>
#include <cstdint>
#include <optional>
#include <type_traits>
#include <vector>

template<typename SquareMatrix>
auto det_naive(const SquareMatrix &a) {
  using T = std::decay_t<decltype(a[0][0])>;
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return T(1);
  }
  if (n == 1) {
    return a[0][0];
  }
  if (n == 2) {
    return a[0][0] * a[1][1] - a[0][1] * a[1][0];
  }
  T res = 0;
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
int elimination_pivot(const SquareMatrix &a, int row, int col, double eps) {
  using T = std::decay_t<decltype(a[0][0])>;
  int n = static_cast<int>(a.size());
  if constexpr (std::is_floating_point_v<T>) {
    int pivot = row;
    for (int r = row + 1; r < n; r++) {
      if (fabs(a[r][col]) > fabs(a[pivot][col])) {
        pivot = r;
      }
    }
    return fabs(a[pivot][col]) < eps ? -1 : pivot;
  }
  for (int r = row; r < n; r++) {
    if (a[r][col] != T(0)) {
      return r;
    }
  }
  return -1;
}

template<typename SquareMatrix>
auto det(const SquareMatrix &a, double eps = 1e-10) {
  using T = std::decay_t<decltype(a[0][0])>;
  int n = static_cast<int>(a.size());
  SquareMatrix b(a);
  T res = 1;
  for (int i = 0; i < n; i++) {
    int p = elimination_pivot(b, i, i, eps);
    if (p == -1) {
      return T(0);
    }
    if (p != i) {
      std::swap(b[p], b[i]);
      res = T(0) - res;
    }
    res *= b[i][i];
    for (int j = i + 1; j < n; j++) {
      T z = b[j][i] / b[i][i];
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
#if defined(__SIZEOF_INT128__)
        __extension__ typedef __int128 int128_t;
        int128_t numerator =
            static_cast<int128_t>(a[i][j]) * a[k][k] - static_cast<int128_t>(a[i][k]) * a[k][j];
        a[i][j] = static_cast<int64_t>(numerator / prev);
#else
        a[i][j] = (a[i][j] * a[k][k] - a[i][k] * a[k][j]) / prev;  // Overflow warning.
#endif
      }
    }
    prev = a[k][k];
  }
  return n == 0 ? 1 : sign * a[n - 1][n - 1];
}

template<typename SquareMatrix>
std::optional<SquareMatrix> inverse(SquareMatrix a, double eps = 1e-10) {
  using T = std::decay_t<decltype(a[0][0])>;
  int n = static_cast<int>(a.size());
  for (int i = 0; i < n; i++) {
    a[i].resize(2 * n);
    for (int j = n; j < n * 2; j++) {
      a[i][j] = (i == j - n ? 1 : 0);
    }
  }
  for (int i = 0; i < n; i++) {
    int p = elimination_pivot(a, i, i, eps);
    if (p == -1) {
      return std::nullopt;
    }
    std::swap(a[p], a[i]);
    T pivot = a[i][i];
    for (int j = i; j < n * 2; j++) {
      a[i][j] /= pivot;
    }
    for (int j = 0; j < n; j++) {
      if (i != j) {
        T factor = a[j][i];
        for (int k = 0; k < n * 2; k++) {
          a[j][k] -= factor * a[i][k];
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
  assert(fabs(det(a) - det_naive(a)) < 1e-10);

  // Bareiss gives the determinant of an integer matrix exactly, with no rounding.
  vector<vector<int64_t>> ai{{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
  assert(det_naive(ai) == -306);  // Division-free, so also exact in the int64_t element type.
  assert(det_naive(vector<vector<int64_t>>{}) == 1);
  assert(det_bareiss(ai) == -306);
  assert(det_bareiss({{2, 0, 0}, {0, 3, 0}, {0, 0, 5}}) == 30);
  assert(det_bareiss({{1, 2}, {2, 4}}) == 0);  // Singular.
  auto inv = inverse(a);
  assert(inv);
  assert(!inverse(vector<vector<double>>{{1, 2}, {2, 4}}));
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        res[i][j] += a[i][k] * (*inv)[k][j];
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
