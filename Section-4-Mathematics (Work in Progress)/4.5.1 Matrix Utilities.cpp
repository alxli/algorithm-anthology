/*

Basic matrix operations defined on a two-dimensional vector of numeric values.

- make_matrix(r, c, v) constructs and returns a matrix with r rows and c columns
  where the value at every index is initialized to v.
- make_matrix(a) returns a matrix constructed from the two dimensional array a.
- identity_matrix(n) returns the n by n identity matrix, that is, a matrix where
  a[i][j] equals 1 (if i == j), or 0 otherwise, for every i and j in [0, n).
- rows(a) returns the number of rows r in an r by c matrix a.
- columns(a) returns the number of columns c in an r by c matrix a.
- a[i][j] may be used to access or modify the entry at row i, column j of an r
  by c matrix a, for every i in [0, r) and j in [0, c).
- operators <, >, <=, >=, ==, and != defines lexicographical comparison based on
  that of std::vector.
- operators +, -, *, /, +=, -=, *=, and /= defines scalar addition, subtraction,
  multiplication, and division involving a matrix a numeric scalar value v.
- operators * and *= defines vector and matrix multiplication.
- operators ^ and ^= defines matrix exponentiation of a square matrix a by an
  integer power p.
- power_sum(a, p) returns the power sum of a square matrix a up to an integer
  power p, that is, a + a^2 + ... + a^p.
- transpose(a) returns the transpose of an r by c matrix a, that is, a new c by
  r matrix b such that a[i][j] == b[j][i] for every i in [0, r) and j in [0, c).
- transpose_in_place(a) assigns the square matrix a to its transpose, returning
  a reference to the modified argument itself.
- rotate(a, d) returns the matrix a rotated d degrees clockwise. A negative d
  specifies a counter-clockwise rotation, and d must be a multiple of 90.
- rotate_in_place(a, d) assigns the square matrix a to its rotation by d degrees
  clockwise, returning a reference to the modified argument itself. A negative d
  specifies a counter-clockwise rotation, and d must be a multiple of 90.

Time Complexity:
- O(n*m) for construction, output, comparison, and scalar arithmetic of n by m
  matrices.
- O(1) for rows(a) and columns(a).
- O(n*m) for matrix-matrix addition and subtraction of n by m matrices.
- O(n*m*log(p)) for exponentiation of an n by m matrix to power p.
- O(n*m*log^2(p)) for power sum of an n by m matrix to power p.
- O(n*m*k) for multiplication of an n by m matrix by an m by k matrix.
- O(n*m) for transpose(), transpose_in_place(), rotate(), and rotate_in_place()
  of n by m matrices.

Space Complexity:
- O(1) auxiliary space for rows(), columns(), a[i][j] access, comparison
  operators, and in-place operations.
- O(n*m*log(p)) auxiliary stack and heap space for exponentiation of an n by m
  matrix to power p, as well as the power sum of an n by m matrix up to power p.
- O(n*m) auxiliary heap space for all non-in-place operations returning an n by
  m matrix, transpose(), and rotate().

*/

#include <algorithm>
#include <cstddef>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <vector>

typedef std::vector<std::vector<int> > matrix;

matrix make_matrix(int r, int c) {
  return matrix(r, matrix::value_type(c));
}

template<class T>
matrix make_matrix(int r, int c, const T &v) {
  return matrix(r, matrix::value_type(c, v));
}

template<class T, size_t r, size_t c>
matrix make_matrix(T (&a)[r][c]) {
  matrix res(r, matrix::value_type(c));
  for (size_t i = 0; i < r; i++) {
    for (size_t j = 0; j < c; j++) {
      res[i][j] = a[i][j];
    }
  }
  return res;
}

matrix identity_matrix(int n) {
  matrix res(n, matrix::value_type(n, 0));
  for (int i = 0; i < n; i++) {
    res[i][i] = 1;
  }
  return res;
}

int rows(const matrix &a) { return a.size(); }
int columns(const matrix &a) { return a.empty() ? 0 : a[0].size(); }

std::ostream& operator<<(std::ostream &out, const matrix &a) {
  static const int W = 10, P = 5;
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      out << std::setw(W) << std::fixed << std::setprecision(P) << a[i][j];
    }
    out << std::endl;
  }
  return out;
}

template<class T>
matrix& operator+=(matrix &a, const T &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] += v;
    }
  }
  return a;
}

template<class T>
matrix& operator-=(matrix &a, const T &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] -= v;
    }
  }
  return a;
}

template<class T>
matrix& operator*=(matrix &a, const T &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] *= v;
    }
  }
  return a;
}

template<class T>
matrix& operator/=(matrix &a, const T &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] /= v;
    }
  }
  return a;
}

matrix& operator+=(matrix &a, const matrix &b) {
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

matrix& operator-=(matrix &a, const matrix &b) {
  if (rows(a) != rows(b) || columns(a) != columns(b)) {
    throw std::runtime_error("Invalid dimensions for matrix addition.");
  }
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(a); j++) {
      a[i][j] -= b[i][j];
    }
  }
  return a;
}

matrix operator+(const matrix &a, const matrix &b) {
  matrix c(a);
  return c += b;
}

matrix operator-(const matrix &a, const matrix &b) {
  matrix c(a);
  return c -= b;
}

template<class T>
matrix& operator*=(matrix &a, const std::vector<T> &v) {
  if (columns(a) != (int)v.size() || v.empty()) {
    throw std::runtime_error("Invalid dimensions for matrix multiplication.");
  }
  for (int i = 0; i < rows(a); i++) {
    a[i][0] *= v[0];
    for (int j = 1; j < columns(a); j++) {
      a[i][0] += a[i][j]*v[j];
    }
  }
  for (int i = 0; i < rows(a); i++) {
    a[i].resize(1);
  }
  return a;
}

matrix operator*(const matrix &a, const matrix &b) {
  if (columns(a) != rows(b)) {
    throw std::runtime_error("Invalid dimensions for matrix multiplication.");
  }
  matrix res = make_matrix(rows(a), columns(b), 0);
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < columns(b); j++) {
      for (int k = 0; k < rows(b); k++) {
        res[i][j] += a[i][k]*b[k][j];
      }
    }
  }
  return res;
}

matrix& operator*=(matrix &a, const matrix &b) {
  return a = a*b;
}

template<class T>
matrix operator+(const matrix &a, const T &v) { matrix m(a); return m += v; }

template<class T>
matrix operator-(const matrix &a, const T &v) { matrix m(a); return m -= v; }

template<class T>
matrix operator*(const matrix &a, const T &v) { matrix m(a); return m *= v; }

template<class T>
matrix operator/(const matrix &a, const T &v) { matrix m(a); return m /= v; }

template<class T>
matrix operator+(const T &v, const matrix &a) { return a + v; }

template<class T>
matrix operator-(const T &v, const matrix &a) { return a - v; }

template<class T>
matrix operator*(const T &v, const matrix &a) { return a * v; }

template<class T>
matrix operator/(const T &v, const matrix &a) { return a / v; }

matrix operator^(const matrix &a, unsigned int p) {
  if (rows(a) != columns(a)) {
    throw std::runtime_error("Matrix must be square for exponentiation.");
  }
  if (p == 0) {
    return identity_matrix(rows(a));
  }
  return (p % 2 == 0) ? (a*a)^(p/2) : a*(a^(p - 1));
}

matrix operator^=(matrix &a, unsigned int p) {
  return a = a ^ p;
}

matrix power_sum(const matrix &a, unsigned int p) {
  if (rows(a) != columns(a)) {
    throw std::runtime_error("Matrix must be square for power_sum.");
  }
  if (p == 0) {
    return make_matrix(rows(a), rows(a));
  }
  return (p % 2 == 0) ? power_sum(a, p/2)*(identity_matrix(rows(a)) + (a^(p/2)))
                      : (a + a*power_sum(a, p - 1));
}

matrix transpose(const matrix &a) {
  matrix res = make_matrix(columns(a), rows(a));
  for (int i = 0; i < rows(res); i++) {
    for (int j = 0; j < columns(res); j++) {
      res[i][j] = a[j][i];
    }
  }
  return res;
}

matrix& transpose_in_place(matrix &a) {
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

matrix rotate(const matrix &a, int degrees = 90) {
  if (degrees % 90 != 0) {
    throw std::runtime_error("Rotation must be by a multiple of 90 degrees.");
  }
  if (degrees < 0) {
    degrees = 360 - ((-degrees) % 360);
  }
  matrix res;
  switch (degrees % 360) {
    case 90: {
      res = make_matrix(columns(a), rows(a));
      for (int i = 0; i < columns(a); i++) {
        for (int j = 0; j < rows(a); j++) {
          res[i][j] = a[rows(a) - j - 1][i];
        }
      }
      break;
    }
    case 180: {
      res = make_matrix(rows(a), columns(a));
      for (int i = 0; i < rows(a); i++) {
        for (int j = 0; j < columns(a); j++) {
          res[i][j] = a[rows(a) - i - 1][columns(a) - j - 1];
        }
      }
      break;
    }
    case 270: {
      res = make_matrix(columns(a), rows(a));
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

matrix& rotate_in_place(matrix &a, int degrees = 90) {
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
      for (int i = 0; i < n; i++) {
        std::reverse(a[i].begin(), a[i].end());
      }
      break;
    }
    case 180: {
      for (int i = 0; i < columns(a); i++) {
        for (int j = 0, k = n - 1; j < k; j++, k--) {
          std::swap(a[i][j], a[i][k]);
        }
      }
      for (int j = 0; j < n; j++) {
        for (int i = 0, k = columns(a) - 1; i < k; i++, k--) {
          std::swap(a[i][j], a[k][j]);
        }
      }
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
  int a[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int a90[3][2] = {{4, 1}, {5, 2}, {6, 3}};
  int a180[2][3] = {{6, 5, 4}, {3, 2, 1}};
  int a270[3][2] = {{3, 6}, {2, 5}, {1, 4}};
  cout << make_matrix(a) << endl;
  assert(rotate(make_matrix(a), -270) == make_matrix(a90));
  assert(rotate(make_matrix(a), -180) == make_matrix(a180));
  assert(rotate(make_matrix(a), -90) == make_matrix(a270));
  assert(rotate(make_matrix(a), 0) == make_matrix(a));
  assert(rotate(make_matrix(a), 90) == make_matrix(a90));
  assert(rotate(make_matrix(a), 180) == make_matrix(a180));
  assert(rotate(make_matrix(a), 270) == make_matrix(a270));
  assert(rotate(make_matrix(a), 360) == make_matrix(a));

  int b[3][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  for (int d = -360; d <= 360; d += 90) {
    matrix m = make_matrix(b);
    assert(rotate_in_place(m, d) == rotate(make_matrix(b), d));
  }

  matrix m = make_matrix(5, 5, 10) + 10;
  int v[] = {1, 2, 3, 4, 5}, mv[5][1] = {{300}, {300}, {300}, {300}, {300}};
  assert(m*vector<int>(v, v + 5) == make_matrix(mv));

  m[0][0] += 5;
  assert(m[0][0] == 25 && m[1][1] == 20);
  assert(power_sum(m, 3) == m + m*m + (m^3));
  return 0;
}
