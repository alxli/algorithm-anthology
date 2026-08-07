/*

Converts a matrix to reduced row echelon form using Gaussian elimination to solve a system of linear
equations and compute rank. Each round finds a row with a nonzero entry in the current leading
column, normalizes that row, and subtracts multiples of it from every other row to clear the column.
Floating-point matrices use the largest-magnitude candidate for partial pivoting, though elimination
can still be prone to rounding error; use LU decomposition for greater numerical stability. Exact
field types such as `Modular<MOD>` instead use any nonzero pivot and perform exact arithmetic. Such
types must support construction from $0$ and $1$, equality, addition, subtraction, multiplication,
and division.

- `row_reduce(a)` assigns the matrix `a` to its reduced row echelon form, returning a reference to
  the modified argument itself.
- `matrix_rank(a)` returns the rank of matrix `a`, i.e. the number of nonzero rows after row
  reduction.
- `solve_system(a, b, &x)` solves the system of linear equations $ax = b$ given an $m$ by $n$ matrix
  `a` over the real numbers or an exact field, and a length $m$ vector `b`, returning $0$ if there
  is one solution, $-1$ if there are zero solutions, or $-2$ if there are infinite solutions. If
  there is exactly one solution, then the output vector `x` is populated with the solution of length
  $n$.

Time Complexity:
- O(m*n*min(m, n)) per call to `row_reduce()` and `matrix_rank()`, where $m$ and $n$ are the numbers
  of rows and columns of `a`, respectively.
- O(m*n*min(m, n)) per call to `solve_system()`.

Space Complexity:
- O(1) auxiliary for `row_reduce()`.
- O(m*n) auxiliary for `matrix_rank()` and `solve_system()`.

*/

#include <cassert>
#include <cmath>
#include <type_traits>
#include <utility>
#include <vector>

const double EPS = 1e-9;

template<typename T>
bool rref_is_zero(const T &v) {
  if constexpr (std::is_floating_point_v<T>) {
    return std::fabs(v) < EPS;
  }
  return v == T(0);
}

template<typename Matrix>
Matrix &row_reduce(Matrix &a) {
  if (a.empty()) {
    return a;
  }
  using T = std::decay_t<decltype(a[0][0])>;
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  for (int r = 0, lead = 0; r < rows && lead < cols; lead++) {
    int pivot = r;
    if constexpr (std::is_floating_point_v<T>) {
      for (int i = r + 1; i < rows; i++) {
        if (std::fabs(a[i][lead]) > std::fabs(a[pivot][lead])) {
          pivot = i;
        }
      }
    } else {
      while (pivot < rows && rref_is_zero(a[pivot][lead])) {
        pivot++;
      }
    }
    if (pivot == rows || rref_is_zero(a[pivot][lead])) {
      continue;
    }
    std::swap(a[pivot], a[r]);
    auto lv = a[r][lead];
    for (int j = 0; j < cols; j++) {
      a[r][j] /= lv;
    }
    for (int i = 0; i < rows; i++) {
      if (i != r) {
        lv = a[i][lead];
        for (int j = 0; j < cols; j++) {
          a[i][j] -= lv * a[r][j];
        }
      }
    }
    for (int j = 0; j < lead; j++) {
      a[r][j] = 0;
    }
    a[r][lead] = 1;
    r++;
  }
  return a;
}

template<typename Matrix>
int matrix_rank(Matrix a) {
  row_reduce(a);
  int rank = 0;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    for (int j = 0; j < static_cast<int>(a[i].size()); j++) {
      if (!rref_is_zero(a[i][j])) {
        rank++;
        break;
      }
    }
  }
  return rank;
}

template<typename Matrix, typename T>
int solve_system(const Matrix &a, const std::vector<T> &b, std::vector<T> *x) {
  if (x == nullptr || a.empty() || a.size() != b.size()) {
    return -1;
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  Matrix m(a);
  for (int i = 0; i < rows; i++) {
    m[i].push_back(b[i]);
  }
  row_reduce(m);
  int rank = 0;
  for (int i = 0; i < rows; i++) {
    int lead = -1;
    for (int j = 0; j < cols && lead < 0; j++) {
      if (!rref_is_zero(m[i][j])) {
        lead = j;
      }
    }
    if (lead < 0) {
      // A zero coefficient row with a nonzero constant means 0 = nonzero (no solution).
      if (!rref_is_zero(m[i][cols])) {
        return -1;
      }
    } else {
      rank++;
    }
  }
  // A unique solution requires a pivot in every column; fewer means free variables remain. This
  // also catches trailing free columns, which a per-row "lead > i" test alone would miss.
  if (rank < cols) {
    return -2;
  }
  x->resize(cols);
  for (int i = 0; i < cols; i++) {
    (*x)[i] = m[i][cols];
  }
  return 0;
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<vector<double>> a{{-1, 2, 5}, {1, 0, -6}, {-4, 2, 2}};
  vector<double> b{3, 1, -2};
  vector<double> x;
  assert(solve_system(a, b, &x) == 0);
  assert(matrix_rank(a) == 3);
  assert(matrix_rank(vector<vector<double>>{{1, 2, 3}, {2, 4, 6}, {0, 1, 1}}) == 2);
  assert(solve_system(vector<vector<double>>{{0, 0}}, vector<double>{1}, &x) == -1);
  assert(solve_system(vector<vector<double>>{{1, 0}}, vector<double>{1}, &x) == -2);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    double sum = 0;
    for (int j = 0; j < static_cast<int>(a[i].size()); j++) {
      sum += a[i][j] * x[j];
    }
    assert(fabs(sum - b[i]) < EPS);
  }
  return 0;
}
