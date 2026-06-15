/*

Converts a matrix to reduced row echelon form using Gaussian elimination to solve a system of linear
equations and compute rank. Each round finds a row with a nonzero entry in the current leading
column, swaps the largest-magnitude candidate into place for partial pivoting, normalizes that row,
and subtracts multiples of it from every other row to clear the column. In practice, this method can
still be prone to rounding error on certain matrices. For a more accurate algorithm for solving
systems of linear equations, LU decomposition with row partial pivoting should be used.

- `row_reduce(a)` assigns the matrix `a` to its reduced row echelon form, returning a reference to
  the modified argument itself.
- `matrix_rank(a)` returns the rank of matrix `a`, i.e. the number of nonzero rows after row
  reduction.
- `solve_system(a, b, &x)` solves the system of linear equations $ax = b$ given an $m$ by $n$ matrix
  `a` of real values, and a length $m$ vector `b`, returning 0 if there is one solution, $-1$
  if there are zero solutions, or $-2$ if there are infinite solutions. If there is exactly one
  solution, then the vector pointed to by `x` is populated with the solution vector of length $n$.

Time Complexity:
- O(m^2*n) per call to `row_reduce(a)`, `matrix_rank(a)`, and `solve_system(a)`, where $m$ and $n$
  are the number of rows and columns of `a`, respectively.

Space Complexity:
- O(1) auxiliary for `row_reduce(a)`.
- O(m*n) auxiliary heap space for `matrix_rank(a)` and `solve_system(a)`.

*/

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <vector>

const double EPS = 1e-9;

template<class Matrix>
Matrix &row_reduce(Matrix &a) {
  if (a.empty()) {
    return a;
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  for (int r = 0, lead = 0; r < rows && lead < cols; lead++) {
    int pivot = r;
    for (int i = r + 1; i < rows; i++) {
      if (fabs(a[i][lead]) > fabs(a[pivot][lead])) {
        pivot = i;
      }
    }
    if (fabs(a[pivot][lead]) < EPS) {
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

template<class Matrix>
int matrix_rank(Matrix a) {
  row_reduce(a);
  int rank = 0;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    for (int j = 0; j < static_cast<int>(a[i].size()); j++) {
      if (fabs(a[i][j]) > EPS) {
        rank++;
        break;
      }
    }
  }
  return rank;
}

template<class Matrix, class T>
int solve_system(const Matrix &a, const std::vector<T> &b, std::vector<T> *x) {
  if (x == nullptr || a.empty() || a.size() != b.size()) {
    return -1;
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  if (rows < cols) {
    return -2;
  }
  Matrix m(a);
  for (int i = 0; i < rows; i++) {
    m[i].push_back(b[i]);
  }
  row_reduce(m);
  int rank = 0;
  for (int i = 0; i < rows; i++) {
    int lead = -1;
    for (int j = 0; j < cols && lead < 0; j++) {
      if (fabs(m[i][j]) > EPS) {
        lead = j;
      }
    }
    if (lead < 0) {
      // A zero coefficient row with a nonzero constant means 0 = nonzero (no solution).
      if (fabs(m[i][cols]) > EPS) {
        return -1;
      }
    } else {
      rank++;
    }
  }
  // A unique solution requires a pivot in every column; fewer means free variables remain. This
  // also catches trailing free columns, which a per-row `lead > i` test alone would miss.
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

#include <cassert>
using namespace std;

int main() {
  vector<vector<double>> a{{-1, 2, 5}, {1, 0, -6}, {-4, 2, 2}};
  vector<double> b{3, 1, -2};
  vector<double> x;
  assert(solve_system(a, b, &x) == 0);
  assert(matrix_rank(a) == 3);
  assert(matrix_rank(vector<vector<double>>{{1, 2, 3}, {2, 4, 6}, {0, 1, 1}}) == 2);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    double sum = 0;
    for (int j = 0; j < static_cast<int>(a[i].size()); j++) {
      sum += a[i][j] * x[j];
    }
    assert(fabs(sum - b[i]) < EPS);
  }
  return 0;
}
