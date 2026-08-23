/*

The QR decomposition factors an $m$ by $n$ matrix $A$ with $m \geq n$ into $A = QR$, where $Q$ has
orthonormal columns and $R$ is upper triangular. Orthonormal columns are what make the factorization
useful: $Q$ preserves lengths, so multiplying by $Q^T$ rewrites a problem in a new basis without
distorting distances, and a triangular system is solved by back substitution.

This implementation uses Householder reflections. A Householder reflection is the mirror across the
hyperplane orthogonal to a chosen vector $v$, written $I - 2vv^T/(v^Tv)$, and $v$ is chosen so that
the reflection maps the part of a column below the diagonal onto the axis, zeroing it in one step.
Applying one reflection per column leaves $R$, and accumulating the reflections gives $Q$.
Reflecting is stable because it never scales anything: the alternative of orthogonalizing column by
column with Gram-Schmidt loses orthogonality as the columns approach linear dependence, whereas
reflections keep it to within rounding error.

The overdetermined system $Ax = b$ usually has no solution, so least squares asks for the $x$ whose
residual is shortest. Because $Q$ preserves lengths, $\|Ax - b\|$ equals $\|Rx - Q^Tb\|$, and the
entries of $Q^Tb$ below row $n$ cannot be changed by any $x$; minimizing therefore means solving the
square triangular system formed by the first $n$ rows, which back substitution does directly.

- `qr_decompose(a, &q, &r)` factors the $m$ by $n$ matrix `a` with $m \geq n$ into an $m$ by $m$
  orthogonal matrix `q` and an $m$ by $n$ upper triangular matrix `r`, whose product is `a`.
- `least_squares(a, b, eps = 1e-10)` returns the vector $x$ minimizing the Euclidean norm of
  $`a`x - `b`$, or `std::nullopt` if the columns of `a` are linearly dependent to within `eps`. The
  matrix `a` must have at least as many rows as columns, and `b` must have one entry per row.

Fitting any linear model reduces to this: one row per data point, one column per basis function.
Solving the normal equations $A^TAx = A^Tb$ by the row reduction of section 6.5.2 gives the same
answer but squares the condition number, losing roughly twice as many digits.

Time Complexity:
- O(m^2*n) per call to `qr_decompose()`, where $m$ and $n$ are the numbers of rows and columns.
- O(m*n^2) per call to `least_squares()`, since it never forms `q` explicitly.

Space Complexity:
- O(m^2 + m*n) for the matrices returned by `qr_decompose()`.
- O(m*n) auxiliary and O(n) for the vector returned by `least_squares()`.

*/

#include <cassert>
#include <cmath>
#include <optional>
#include <vector>

using Matrix = std::vector<std::vector<double>>;

// Reflects the trailing rows of the given column onto the axis, returning the reflection vector.
std::vector<double> householder_vector(const Matrix &a, int col) {
  int m = static_cast<int>(a.size());
  std::vector<double> v(m);
  double norm = 0;
  for (int i = col; i < m; i++) {
    v[i] = a[i][col];
    norm += v[i] * v[i];
  }
  norm = std::sqrt(norm);
  // Reflect away from a[col][col] so that the subtraction below never cancels.
  v[col] += a[col][col] >= 0 ? norm : -norm;
  return v;
}

void qr_decompose(const Matrix &a, Matrix *q, Matrix *r) {
  int m = static_cast<int>(a.size()), n = a.empty() ? 0 : static_cast<int>(a[0].size());
  assert(m >= n && q != nullptr && r != nullptr);
  *r = a;
  q->assign(m, std::vector<double>(m));
  for (int i = 0; i < m; i++) {
    (*q)[i][i] = 1;
  }
  for (int col = 0; col < n; col++) {
    std::vector<double> v = householder_vector(*r, col);
    double vv = 0;
    for (int i = col; i < m; i++) {
      vv += v[i] * v[i];
    }
    if (vv == 0) {
      continue;  // The column is already zero below the diagonal.
    }
    for (int j = 0; j < n; j++) {  // Apply the reflection to R from the left.
      double dot = 0;
      for (int i = col; i < m; i++) {
        dot += v[i] * (*r)[i][j];
      }
      for (int i = col; i < m; i++) {
        (*r)[i][j] -= 2 * dot / vv * v[i];
      }
    }
    for (int j = 0; j < m; j++) {  // Accumulate the reflection into Q from the right.
      double dot = 0;
      for (int i = col; i < m; i++) {
        dot += v[i] * (*q)[j][i];
      }
      for (int i = col; i < m; i++) {
        (*q)[j][i] -= 2 * dot / vv * v[i];
      }
    }
  }
}

std::optional<std::vector<double>> least_squares(
    const Matrix &a, const std::vector<double> &b, double eps = 1e-10
) {
  int m = static_cast<int>(a.size()), n = a.empty() ? 0 : static_cast<int>(a[0].size());
  assert(m >= n && static_cast<int>(b.size()) == m);
  Matrix r = a;
  std::vector<double> rhs = b;
  for (int col = 0; col < n; col++) {
    std::vector<double> v = householder_vector(r, col);
    double vv = 0;
    for (int i = col; i < m; i++) {
      vv += v[i] * v[i];
    }
    if (vv == 0) {
      continue;
    }
    for (int j = col; j < n; j++) {
      double dot = 0;
      for (int i = col; i < m; i++) {
        dot += v[i] * r[i][j];
      }
      for (int i = col; i < m; i++) {
        r[i][j] -= 2 * dot / vv * v[i];
      }
    }
    double dot = 0;  // The same reflection applied to the right-hand side.
    for (int i = col; i < m; i++) {
      dot += v[i] * rhs[i];
    }
    for (int i = col; i < m; i++) {
      rhs[i] -= 2 * dot / vv * v[i];
    }
  }
  std::vector<double> x(n);
  for (int i = n - 1; i >= 0; i--) {  // Back substitution over the triangular top block.
    if (std::fabs(r[i][i]) <= eps) {
      return std::nullopt;
    }
    double sum = rhs[i];
    for (int j = i + 1; j < n; j++) {
      sum -= r[i][j] * x[j];
    }
    x[i] = sum / r[i][i];
  }
  return x;
}

/*** Example Usage ***/

#include <cmath>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  Matrix a{{12, -51, 4}, {6, 167, -68}, {-4, 24, -41}};
  Matrix q, r;
  qr_decompose(a, &q, &r);
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      double product = 0, orthogonal = 0;
      for (int k = 0; k < 3; k++) {
        product += q[i][k] * r[k][j];
        orthogonal += q[k][i] * q[k][j];
      }
      assert(EQ(product, a[i][j]));                // The factors multiply back to a.
      assert(EQ(orthogonal, i == j ? 1.0 : 0.0));  // The columns of q are orthonormal.
      assert(i <= j || EQ(r[i][j], 0.0));          // r is upper triangular.
    }
  }

  // An exactly solvable square system.
  auto exact = least_squares(Matrix{{2, 1}, {1, 3}}, vector<double>{5, 10});
  assert(exact.has_value() && EQ((*exact)[0], 1.0) && EQ((*exact)[1], 3.0));

  // Fit y = c0 + c1*x to four collinear points, then to points with one outlier.
  Matrix design{{1, 0}, {1, 1}, {1, 2}, {1, 3}};
  auto line = least_squares(design, vector<double>{1, 3, 5, 7});
  assert(line.has_value() && EQ((*line)[0], 1.0) && EQ((*line)[1], 2.0));
  auto noisy = least_squares(design, vector<double>{1, 3, 5, 8});
  assert(noisy.has_value() && EQ((*noisy)[0], 0.8) && EQ((*noisy)[1], 2.3));

  // Dependent columns have no unique least-squares solution.
  assert(!least_squares(Matrix{{1, 2}, {2, 4}, {3, 6}}, vector<double>{1, 2, 3}).has_value());
  return 0;
}
