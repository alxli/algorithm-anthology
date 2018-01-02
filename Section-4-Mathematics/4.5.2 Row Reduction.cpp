/*

Converts a matrix to reduced row echelon form using Gaussian elimination to
solve a system of linear equations as well as compute the determinant. In
practice, this method is prone to rounding error on certain matrices. For a more
accurate algorithm for solving systems of linear equations, LU decomposition
with row partial pivoting should be used.

- row_reduce(a) assigns the matrix a to its reduced row echelon form, returning
  a reference to the modified argument itself.
- solve_system(a, b, &x) solves the system of linear equations a*x = b given an
  r by c matrix a of real values, and a length r vector b, returning 0 if there
  is one solution, -1 if there is zero solutions, or -2 if there are infinite
  solutions. If there is exactly one solution, then the vector pointed to by
  x is populated with the solution vector of length c.

Time Complexity:
- O(r^2*c) per call to row_reduce(a) and solve_system(a), where r and c are the
  number of rows and columns of a respectively.

Space Complexity:
- O(1) auxiliary for row_reduce(a).
- O(r*c) auxiliary heap space for solve_system(a).

*/

#include <cmath>
#include <cstdlib>
#include <stdexcept>
#include <vector>

const double EPS = 1e-9;

template<class Matrix>
Matrix& row_reduce(Matrix &a) {
  if (a.empty()) {
    return a;
  }
  int r = a.size(), c = a[0].size(), lead = 0;
  for (int row = 0; row < r && lead < c; row++) {
    int i = row;
    while (fabs(a[i][lead]) < EPS) {
      if (++i == r) {
        i = row;
        if (++lead == c) {
          return a;
        }
      }
    }
    std::swap(a[i], a[row]);
    typename Matrix::value_type::value_type lv = a[row][lead];
    for (int j = 0; j < c; j++) {
      a[row][j] /= lv;
    }
    for (int i = 0; i < r; i++) {
      if (i != row) {
        lv = a[i][lead];
        for (int j = 0; j < c; j++) {
          a[i][j] -= lv*a[row][j];
        }
      }
    }
    for (int j = 0; j < lead; j++) {
      a[row][j] = 0;
    }
    a[row][lead++] = 1;
  }
  return a;
}

template<class Matrix, class T>
int solve_system(const Matrix &a, const std::vector<T> &b, std::vector<T> *x) {
  if (x == NULL || a.empty() || a.size() != b.size()) {
    return -1;
  }
  int r = a.size(), c = a[0].size();
  if (r < c) {
    return -2;
  }
  Matrix m(a);
  for (int i = 0; i < r; i++) {
    m[i].push_back(b[i]);
  }
  row_reduce(m);
  for (int i = 0; i < r; i++) {
    int lead = -1;
    for (int j = 0; j < c && lead < 0; j++) {
      if (fabs(m[i][j]) > EPS) {
        lead = j;
      }
    }
    if (lead < 0 && fabs(m[i][c]) > EPS) {
      return -1;
    }
    if (lead > i) {
      return -2;
    }
  }
  x->resize(c);
  for (int i = 0; i < c; i++) {
    (*x)[i] = m[i][c];
  }
  return 0;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
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
    assert(fabs(sum - b[i]) < EPS);
  }
  return 0;
}
