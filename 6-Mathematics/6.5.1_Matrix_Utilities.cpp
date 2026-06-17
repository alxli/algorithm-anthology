/*

Basic matrix operations defined on a two-dimensional vector of numeric values.

- `make_matrix<T>(m, n, v)` constructs and returns an $m$ by $n$ matrix with 0-based indices (row
  indices [0, `m`) and column indices [0, `n`)), where every value is initialized to `v`.
- `identity_matrix<T>(n)` returns the `n` by `n` identity matrix, that is, a matrix where each
  `a[i][j]` equals 1 if `i` = `j`, or 0 otherwise.
- `rows(a)` returns the number of rows in matrix `a`.
- `columns(a)` returns the number of columns in matrix `a`.
- `a[i][j]` may be used to access or modify the specified entry of `a`.
- Operators `<`, `>`, `<=`, `>=`, `==`, and `!=` define lexicographical comparison based on that of
  `std::vector`.
- Operators `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, and `/=` define scalar addition, subtraction,
  multiplication, and division involving a matrix and a numeric scalar value.
- Operators `*` and `*=` define vector and matrix multiplication.
- Operators `^` and `^=` define iterative binary matrix exponentiation of a square matrix `a` by an
  integer power `p`.
- `power_sum(a, p)` returns the power sum of a square matrix `a` up to an integer power `p`, that
  is, $a + a^2 + \ldots + a^p$.
- `transpose(a)` returns the transpose of an $m$ by $n$ matrix `a`, that is, a new $n$ by $m$ matrix
  $b$ such that `a[i][j]` = `b[j][i]` for every `i` $\in [0, m)$ and `j` $\in [0, n)$.
- `transpose_in_place(a)` assigns the square matrix `a` to its transpose, returning a reference to
  the modified argument itself.
- `rotate(a, d)` returns the matrix `a` rotated `d` degrees clockwise. A negative `d` specifies a
  counter-clockwise rotation, and `d` must be a multiple of 90.
- `rotate_in_place(a, d)` assigns the square matrix `a` to its rotation by `d` degrees clockwise,
  returning a reference to the modified argument itself. A negative `d` specifies a
  counter-clockwise rotation, and `d` must be a multiple of 90.

Exponentiation uses the usual iterative binary exponentiation pattern: keep an accumulated result,
square the base each round, and multiply the result when the current exponent bit is set. The power
sum uses a block matrix identity that $\begin{bmatrix} A & A \\ 0 & I \end{bmatrix}^p$ has
$A + A^2 + \dots + A^p$ in its upper-right block.

Time Complexity:
- O(m*n) for construction, output, comparison, and scalar arithmetic of $m$ by $n$ matrices.
- O(1) for `rows(a)` and `columns(a)`.
- O(m*n) for matrix-matrix addition and subtraction of $m$ by $n$ matrices.
- O(n^3 log p) for exponentiation of an $n$ by $n$ matrix to power $p$.
- O(n^3 log p) for power sum of an $n$ by $n$ matrix to power $p$.
- O(m*n*k) for multiplication of an $m$ by $n$ matrix by an $n$ by $k$ matrix.
- O(m*n) for `transpose()`, `transpose_in_place()`, `rotate()`, and `rotate_in_place()` of $m$ by
  $n$ matrices.

Space Complexity:
- O(1) auxiliary space for `rows()`, `columns()`, `a[i][j]` access, comparison operators, and
  in-place operations.
- O(n^2) auxiliary heap space for exponentiation of an $n$ by $n$ matrix to power $p$.
- O(n^2) auxiliary heap space for `power_sum()` of an $n$ by $n$ matrix up to power $p$.
- O(m*n) auxiliary heap space for all non-in-place operations returning an $m$ by $n$ matrix,
  `transpose()`, and `rotate()`.

*/

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <vector>

template<class T>
using Matrix = std::vector<std::vector<T>>;

template<class T = int>
Matrix<T> make_matrix(int m, int n, const T &v = T{}) {
  return Matrix<T>(m, std::vector<T>(n, v));
}

template<class T = int>
Matrix<T> identity_matrix(int n) {
  Matrix<T> res = make_matrix<T>(n, n);
  for (int i = 0; i < n; i++) {
    res[i][i] = 1;
  }
  return res;
}

template<class T>
int rows(const Matrix<T> &a) {
  return static_cast<int>(a.size());
}

template<class T>
int columns(const Matrix<T> &a) {
  return a.empty() ? 0 : static_cast<int>(a[0].size());
}

template<class T>
std::ostream &operator<<(std::ostream &out, const Matrix<T> &a) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      out << std::setw(10) << std::fixed << std::setprecision(5) << a[i][j];
    }
    out << std::endl;
  }
  return out;
}

template<class T, class U>
Matrix<T> &operator+=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] += v;
    }
  }
  return a;
}

template<class T, class U>
Matrix<T> &operator-=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] -= v;
    }
  }
  return a;
}

template<class T, class U>
Matrix<T> &operator*=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] *= v;
    }
  }
  return a;
}

template<class T, class U>
Matrix<T> &operator/=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] /= v;
    }
  }
  return a;
}

template<class T>
Matrix<T> &operator+=(Matrix<T> &a, const Matrix<T> &b) {
  if (rows(a) != rows(b) || columns(a) != columns(b)) {
    throw std::runtime_error("Invalid dimensions for matrix addition.");
  }
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] += b[i][j];
    }
  }
  return a;
}

template<class T>
Matrix<T> &operator-=(Matrix<T> &a, const Matrix<T> &b) {
  if (rows(a) != rows(b) || columns(a) != columns(b)) {
    throw std::runtime_error("Invalid dimensions for matrix subtraction.");
  }
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] -= b[i][j];
    }
  }
  return a;
}

template<class T>
Matrix<T> operator+(const Matrix<T> &a, const Matrix<T> &b) {
  Matrix<T> c(a);
  return c += b;
}

template<class T>
Matrix<T> operator-(const Matrix<T> &a, const Matrix<T> &b) {
  Matrix<T> c(a);
  return c -= b;
}

template<class T>
Matrix<T> &operator*=(Matrix<T> &a, const std::vector<T> &v) {
  if (columns(a) != static_cast<int>(v.size()) || v.empty()) {
    throw std::runtime_error("Invalid dimensions for matrix-vector multiplication.");
  }
  int cols = columns(a);
  for (int i = 0; i < rows(a); i++) {
    T sum = 0;
    for (int j = 0; j < cols; j++) {
      sum += a[i][j] * v[j];
    }
    a[i].assign(1, sum);
  }
  return a;
}

template<class T>
Matrix<T> operator*(const Matrix<T> &a, const Matrix<T> &b) {
  if (columns(a) != rows(b)) {
    throw std::runtime_error("Invalid dimensions for matrix multiplication.");
  }
  Matrix<T> res = make_matrix<T>(rows(a), columns(b));
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(b); j++) {
      for (int k = 0; k < rows(b); k++) {
        res[i][j] += a[i][k] * b[k][j];
      }
    }
  }
  return res;
}

// clang-format off
template<class T>
Matrix<T> &operator*=(Matrix<T> &a, const Matrix<T> &b) { return a = a * b; }

template<class T, class U>
Matrix<T> operator+(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m += v; }

template<class T, class U>
Matrix<T> operator-(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m -= v; }

template<class T, class U>
Matrix<T> operator*(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m *= v; }

template<class T, class U>
Matrix<T> operator/(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m /= v; }

template<class T, class U>
Matrix<T> operator+(const U &v, const Matrix<T> &a) { return a + v; }

template<class T, class U>
Matrix<T> operator*(const U &v, const Matrix<T> &a) { return a * v; }
// clang-format on

template<class T, class U>
Matrix<T> operator-(const U &v, const Matrix<T> &a) {
  Matrix<T> m = make_matrix<T>(rows(a), columns(a), v);
  return m -= a;
}

template<class T>
Matrix<T> operator^(Matrix<T> a, unsigned int p) {
  if (rows(a) != columns(a)) {
    throw std::runtime_error("Matrix must be square for exponentiation.");
  }
  Matrix<T> res = identity_matrix<T>(rows(a));
  while (p > 0) {
    if (p & 1) {
      res *= a;
    }
    a *= a;
    p >>= 1;
  }
  return res;
}

template<class T>
Matrix<T> operator^=(Matrix<T> &a, unsigned int p) {
  return a = a ^ p;
}

template<class T>
Matrix<T> power_sum(const Matrix<T> &a, unsigned int p) {
  if (rows(a) != columns(a)) {
    throw std::runtime_error("Matrix must be square for power_sum.");
  }
  int n = rows(a);
  if (p == 0) {
    return make_matrix<T>(n, n);
  }
  // [A A]^p = [A^p  A + A^2 + ... + A^p]
  // [0 I]     [ 0                 I          ]
  Matrix<T> block = make_matrix<T>(2 * n, 2 * n);
  for (int i = 0; i < n; i++) {
    block[i][i] = a[i][i];
    block[i + n][i + n] = 1;
    for (int j = 0; j < n; j++) {
      block[i][j] = a[i][j];
      block[i][j + n] = a[i][j];
    }
  }
  block = block ^ p;
  Matrix<T> res = make_matrix<T>(n, n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      res[i][j] = block[i][j + n];
    }
  }
  return res;
}

template<class T>
Matrix<T> transpose(const Matrix<T> &a) {
  Matrix<T> res = make_matrix<T>(columns(a), rows(a));
  for (int i = 0; i < rows(res); i++) {
    for (int j = 0; j < columns(res); j++) {
      res[i][j] = a[j][i];
    }
  }
  return res;
}

template<class T>
Matrix<T> &transpose_in_place(Matrix<T> &a) {
  if (rows(a) != columns(a)) {
    throw std::runtime_error("Matrix must be square for transpose_in_place.");
  }
  for (int i = 0; i < rows(a); i++) {
    for (int j = i + 1; j < columns(a); j++) {
      std::swap(a[i][j], a[j][i]);
    }
  }
  return a;
}

template<class T>
Matrix<T> rotate(const Matrix<T> &a, int degrees = 90) {
  if (degrees % 90 != 0) {
    throw std::runtime_error("Rotation must be by a multiple of 90 degrees.");
  }
  if (degrees < 0) {
    degrees = 360 - ((-degrees) % 360);
  }
  Matrix<T> res;
  switch (degrees % 360) {
    case 90: {
      res = make_matrix<T>(columns(a), rows(a));
      for (int i = 0; i < columns(a); i++) {
        for (int j = 0; j < rows(a); j++) {
          res[i][j] = a[rows(a) - j - 1][i];
        }
      }
      break;
    }
    case 180: {
      res = make_matrix<T>(rows(a), columns(a));
      for (int i = 0; i < rows(a); i++) {
        for (int j = 0; j < columns(a); j++) {
          res[i][j] = a[rows(a) - i - 1][columns(a) - j - 1];
        }
      }
      break;
    }
    case 270: {
      res = make_matrix<T>(columns(a), rows(a));
      for (int i = 0; i < columns(a); i++) {
        for (int j = 0; j < rows(a); j++) {
          res[i][j] = a[j][columns(a) - i - 1];
        }
      }
      break;
    }
    default: {
      res = a;
    }
  }
  return res;
}

template<class T>
Matrix<T> &rotate_in_place(Matrix<T> &a, int degrees = 90) {
  if (degrees % 90 != 0) {
    throw std::runtime_error("Rotation must be by a multiple of 90 degrees.");
  }
  if (degrees % 180 != 0 && rows(a) != columns(a)) {
    throw std::runtime_error("Matrix must be square for rotate_in_place.");
  }
  if (degrees < 0) {
    degrees = 360 - ((-degrees) % 360);
  }
  int n = rows(a);
  switch (degrees % 360) {
    case 90: {
      transpose_in_place(a);
      for (auto &row : a) {
        std::reverse(row.begin(), row.end());
      }
      break;
    }
    case 180: {
      // A 180-degree rotation is a horizontal flip (reverse each row) followed by a vertical flip
      // (reverse the row order). This works for non-square matrices too, unlike a row/column index
      // scheme keyed on a single dimension.
      for (auto &row : a) {
        std::reverse(row.begin(), row.end());
      }
      std::reverse(a.begin(), a.end());
      break;
    }
    case 270: {
      transpose_in_place(a);
      for (int j = 0; j < n; j++) {
        for (int i = 0, k = columns(a) - 1; i < k; i++, k--) {
          std::swap(a[i][j], a[k][j]);
        }
      }
      break;
    }
  }
  return a;
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  using matrix = Matrix<int>;
  matrix a{{1, 2, 3}, {4, 5, 6}};
  matrix a90{{4, 1}, {5, 2}, {6, 3}};
  matrix a180{{6, 5, 4}, {3, 2, 1}};
  matrix a270{{3, 6}, {2, 5}, {1, 4}};
  cout << a << endl;
  assert(rotate(a, -270) == a90);
  assert(rotate(a, -180) == a180);
  assert(rotate(a, -90) == a270);
  assert(rotate(a, 0) == a);
  assert(rotate(a, 90) == a90);
  assert(rotate(a, 180) == a180);
  assert(rotate(a, 270) == a270);
  assert(rotate(a, 360) == a);

  matrix b{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  for (int d = -360; d <= 360; d += 90) {
    matrix m = b;
    assert(rotate_in_place(m, d) == rotate(b, d));
  }

  matrix m = make_matrix(5, 5, 10) + 10;
  vector<int> v{1, 2, 3, 4, 5};
  matrix mv{{300}, {300}, {300}, {300}, {300}};
  assert(m * v == mv);

  m[0][0] += 5;
  assert(m[0][0] == 25 && m[1][1] == 20);
  assert((m ^ 0) == identity_matrix(5));
  assert(power_sum(m, 3) == m + m * m + (m ^ 3));

  Matrix<double> d = make_matrix<double>(2, 2, 0.5);
  assert(rows(d) == 2 && columns(d) == 2);
  assert((d + 0.25)[0][0] == 0.75);
  return 0;
}
