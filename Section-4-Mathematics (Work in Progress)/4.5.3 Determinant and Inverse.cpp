/*

Computes the determinant and inverse of a square matrix using Gaussian
elimination. The inverse of a matrix a is another matrix b such that a*b equals
the identity matrix. The inverse of a exists if and only if the determinant of a
is zero. In this case, a is called invertible or non-singular. In practice,
simple Gaussian elimination is prone to rounding error on certain matrices. For
a more accurate algorithm for solving systems of linear equations, see LU
decomposition with row partial pivoting should be.

- det_naive(a) returns the determinant of an n by n matrix a, using the classic
  divide-and-conquer algorithm by Laplace expansions.
- det(a) returns the determinant of an n by n matrix a using Gaussian
  elimination.
- invert(a) assigns the n by n matrix a to its inverse (if it exists), returning
  a reference to the modified argument itself. If a is not invertible, then its
  assigned values after the function call will be undefined (+/-Inf or +/-NaN).

Time Complexity:
- O(n!) per call to det_naive(), where n is the dimension of the matrix.
- O(n^3) per call to det() and invert() where n is the dimension of the matrix.

Space Complexity:
- O(n) auxiliary stack space and O(n!*n) auxiliary heap space for det_naive(),
  where n is the dimension of the matrix.
- O(n^2) auxiliary heap space for det() and invert().

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
    res += (p % 2 == 0 ? 1 : -1)*a[0][p]*det_naive(temp);
  }
  return res;
}

template<class SquareMatrix>
double det(const SquareMatrix &a, double EPS = 1e-10) {
  SquareMatrix b(a);
  int n = a.size();
  double res = 1.0;
  std::vector<bool> used(n, false);
  for (int i = 0; i < n; i++) {
    int p;
    for (p = 0; p < n; p++) {
      if (!used[p] && fabs(b[p][i]) > EPS) {
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
SquareMatrix& invert(SquareMatrix &a) {
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

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  const int n = 3, a[n][n] = {{6, 1, 1}, {4, -2, 5}, {2, 8, 7}};
  vector<vector<double> > m(n), inv, res(n, vector<double>(n, 0));
  for (int i = 0; i < n; i++) {
    m[i] = vector<double>(a[i], a[i] + n);
  }
  double d = det(m);
  assert(fabs(d - det_naive(m)) < 1e-10);
  invert(inv = m);
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
  return 0;
}
