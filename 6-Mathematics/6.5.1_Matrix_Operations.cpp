/*

Basic matrix operations defined on a two-dimensional vector of numeric values. Matrix arithmetic
also supports modular value types such as `Modular<MOD>` from 6.3.2. The element type must be
constructible from $0$ and $1$ and support the arithmetic operators used by the requested operation.

- `make_matrix<T>(m, n, v)` constructs and returns an $m$ by $n$ matrix with 0-based indices (row
  indices $[0, `m`)$ and column indices $[0, `n`)$), where every value is initialized to `v`.
- `identity_matrix<T>(n)` returns the `n` by `n` identity matrix, that is, a matrix where each
  `a[i][j]` equals $1$ if `i` = `j`, or $0$ otherwise.
- `rows(a)` returns the number of rows in matrix `a`.
- `cols(a)` returns the number of columns in matrix `a`.
- `a[i][j]` may be used to access or modify the specified entry of `a`.
- Operators `<`, `>`, `<=`, `>=`, `==`, and `!=` define lexicographical comparison based on that of
  `std::vector`.
- Operators `+`, `-`, `*`, `/`, `+=`, `-=`, `*=`, and `/=` define scalar addition, subtraction,
  multiplication, and division involving a matrix and a numeric scalar value.
- `a * v` returns the matrix-vector product as a vector.
- Operators `*` and `*=` define matrix multiplication.

Exponentiation uses iterative binary exponentiation: keep an accumulated result, square the base
each round, and multiply it when the current exponent bit is set. The power sum uses the block
identity that $\begin{bmatrix} A & A \\ 0 & I \end{bmatrix}^p$ has $A + A^2 + \dots + A^p$ in its
upper-right block.

- Operators `^` and `^=` define iterative binary matrix exponentiation of a square matrix `a` by a
  `uint64_t` power `p`.
- `power_sum(a, p)` returns the power sum of a square matrix `a` up to a `uint64_t` power `p`, that
  is, $a + a^2 + \ldots + a^p$.

Time Complexity:
- O(m*n) per construction, output, comparison, or scalar-arithmetic operation on $m$ by $n$
  matrices.
- O(1) per call to `rows()` and `cols()`.
- O(m*n) per matrix-matrix addition or subtraction of $m$ by $n$ matrices.
- O(n^3 log p) per exponentiation of an $n$ by $n$ matrix to power $p$.
- O(n^3 log p) per call to `power_sum()` for an $n$ by $n$ matrix and power $p$.
- O(m*n) per multiplication of an $m$ by $n$ matrix by a vector of length $n$.
- O(m*n*k) per multiplication of an $m$ by $n$ matrix by an $n$ by $k$ matrix.

Space Complexity:
- O(1) auxiliary for `rows()`, `cols()`, `a[i][j]` access, comparison and scalar compound operators.
- O(n^2) auxiliary for exponentiation of an $n$ by $n$ matrix to power $p$.
- O(n^2) auxiliary for `power_sum()` of an $n$ by $n$ matrix up to power $p$.
- O(m) for the vector returned by matrix-vector multiplication.
- O(m*n) auxiliary for all non-in-place operations returning an $m$ by $n$ matrix.

*/

#include <cassert>
#include <cstdint>
#include <iomanip>
#include <ostream>
#include <vector>

template<typename T>
using Matrix = std::vector<std::vector<T>>;

template<typename T = int>
Matrix<T> make_matrix(int m, int n, const T &v = T{}) {
  return Matrix<T>(m, std::vector<T>(n, v));
}

template<typename T = int>
Matrix<T> identity_matrix(int n) {
  Matrix<T> res = make_matrix<T>(n, n);
  for (int i = 0; i < n; i++) {
    res[i][i] = 1;
  }
  return res;
}

template<typename T>
int rows(const Matrix<T> &a) {
  return static_cast<int>(a.size());
}

template<typename T>
int cols(const Matrix<T> &a) {
  return a.empty() ? 0 : static_cast<int>(a[0].size());
}

template<typename T>
std::ostream &operator<<(std::ostream &out, const Matrix<T> &a) {
  auto flags = out.flags();
  auto precision = out.precision();
  out << std::fixed << std::setprecision(5);
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      out << std::setw(10) << a[i][j];
    }
    out << std::endl;
  }
  out.flags(flags);
  out.precision(precision);
  return out;
}

template<typename T, typename U>
Matrix<T> &operator+=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      a[i][j] += v;
    }
  }
  return a;
}

template<typename T, typename U>
Matrix<T> &operator-=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      a[i][j] -= v;
    }
  }
  return a;
}

template<typename T, typename U>
Matrix<T> &operator*=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      a[i][j] *= v;
    }
  }
  return a;
}

template<typename T, typename U>
Matrix<T> &operator/=(Matrix<T> &a, const U &v) {
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      a[i][j] /= v;
    }
  }
  return a;
}

template<typename T>
Matrix<T> &operator+=(Matrix<T> &a, const Matrix<T> &b) {
  assert(rows(a) == rows(b) && cols(a) == cols(b));
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      a[i][j] += b[i][j];
    }
  }
  return a;
}

template<typename T>
Matrix<T> &operator-=(Matrix<T> &a, const Matrix<T> &b) {
  assert(rows(a) == rows(b) && cols(a) == cols(b));
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      a[i][j] -= b[i][j];
    }
  }
  return a;
}

template<typename T>
Matrix<T> operator+(const Matrix<T> &a, const Matrix<T> &b) {
  Matrix<T> c(a);
  return c += b;
}

template<typename T>
Matrix<T> operator-(const Matrix<T> &a, const Matrix<T> &b) {
  Matrix<T> c(a);
  return c -= b;
}

template<typename T>
std::vector<T> operator*(const Matrix<T> &a, const std::vector<T> &v) {
  assert(cols(a) == static_cast<int>(v.size()));
  std::vector<T> res(rows(a));
  for (int i = 0; i < rows(a); i++) {
    for (int j = 0; j < cols(a); j++) {
      res[i] += a[i][j] * v[j];
    }
  }
  return res;
}

template<typename T>
Matrix<T> operator*(const Matrix<T> &a, const Matrix<T> &b) {
  assert(cols(a) == rows(b));
  Matrix<T> res = make_matrix<T>(rows(a), cols(b));
  // The k loop sits outside j so the inner loop walks res[i] and b[k] contiguously instead of
  // striding down a column of b. Each res[i][j] still accumulates in increasing k, so results are
  // unchanged, including for floating-point types.
  for (int i = 0; i < rows(a); i++) {
    for (int k = 0; k < rows(b); k++) {
      for (int j = 0; j < cols(b); j++) {
        res[i][j] += a[i][k] * b[k][j];
      }
    }
  }
  return res;
}

// clang-format off
template<typename T>
Matrix<T> &operator*=(Matrix<T> &a, const Matrix<T> &b) { return a = a * b; }

template<typename T, typename U>
Matrix<T> operator+(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m += v; }

template<typename T, typename U>
Matrix<T> operator-(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m -= v; }

template<typename T, typename U>
Matrix<T> operator*(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m *= v; }

template<typename T, typename U>
Matrix<T> operator/(const Matrix<T> &a, const U &v) { Matrix<T> m(a); return m /= v; }

template<typename T, typename U>
Matrix<T> operator+(const U &v, const Matrix<T> &a) { return a + v; }

template<typename T, typename U>
Matrix<T> operator*(const U &v, const Matrix<T> &a) { return a * v; }
// clang-format on

template<typename T, typename U>
Matrix<T> operator-(const U &v, const Matrix<T> &a) {
  Matrix<T> m = make_matrix<T>(rows(a), cols(a), v);
  return m -= a;
}

template<typename T>
Matrix<T> operator^(Matrix<T> a, uint64_t p) {
  assert(rows(a) == cols(a));
  Matrix<T> res = identity_matrix<T>(rows(a));
  while (p > 0) {
    if (p & 1) {
      res *= a;
    }
    p >>= 1;
    if (p > 0) {
      a *= a;
    }
  }
  return res;
}

template<typename T>
Matrix<T> &operator^=(Matrix<T> &a, uint64_t p) {
  return a = a ^ p;
}

template<typename T>
Matrix<T> power_sum(const Matrix<T> &a, uint64_t p) {
  assert(rows(a) == cols(a));
  int n = rows(a);
  if (p == 0) {
    return make_matrix<T>(n, n);
  }
  // The upper-right block of [[A, A], [0, I]]^p is A + A^2 + ... + A^p.
  Matrix<T> block = make_matrix<T>(2 * n, 2 * n);
  for (int i = 0; i < n; i++) {
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

/*** Example Usage ***/

using namespace std;

int main() {
  using matrix = Matrix<int>;
  matrix m = make_matrix(5, 5, 10) + 10;
  vector<int> v{1, 2, 3, 4, 5};
  assert((m * v == vector<int>{300, 300, 300, 300, 300}));

  m[0][0] += 5;
  assert(m[0][0] == 25 && m[1][1] == 20);
  assert((m ^ 0) == identity_matrix(5));
  assert((identity_matrix<int>(3) ^ 5) == identity_matrix<int>(3));
  assert(power_sum(m, 3) == m + m * m + (m ^ 3));

  Matrix<double> d = make_matrix<double>(2, 2, 0.5);
  assert(rows(d) == 2 && cols(d) == 2);
  assert((d + 0.25)[0][0] == 0.75);
  return 0;
}
