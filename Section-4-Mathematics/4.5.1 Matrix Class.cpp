/*

4.5.1 - Matrix Class

Basic matrix class with support for arithmetic operations
as well as matrix multiplication and exponentiation.

*/

#include <ostream>
#include <stdexcept>
#include <vector>

template<class val_t> class matrix {

  int r, c;
  std::vector< std::vector<val_t> > mat;

 public:
  matrix(int rows, int cols, val_t init = val_t()) {
    r = rows;
    c = cols;
    mat.resize(r, std::vector<val_t>(c, init));
  }

  template<size_t rows, size_t cols>
  matrix(val_t (&init)[rows][cols]) {
    r = rows;
    c = cols;
    mat.resize(r, std::vector<val_t>(c));
    for (int i = 0; i < r; i++)
      for (int j = 0; j < c; j++)
        mat[i][j] = init[i][j];
  }

  val_t & operator() (int r, int c) { return mat[r][c]; }
  std::vector<val_t> & operator[] (int r) { return mat[r]; }
  val_t at(int r, int c) const { return mat[r][c]; }

  friend bool operator < (const matrix & a, const matrix & b) {
    return a.mat < b.mat;
  }

  friend bool operator > (const matrix & a, const matrix & b) {
    return a.mat > b.mat;
  }

  friend bool operator <= (const matrix & a, const matrix & b) {
    return a.mat <= b.mat;
  }

  friend bool operator >= (const matrix & a, const matrix & b) {
    return a.mat >= b.mat;
  }

  friend bool operator == (const matrix & a, const matrix & b) {
    return a.mat == b.mat;
  }

  friend bool operator != (const matrix & a, const matrix & b) {
    return a.mat == b.mat;
  }

  friend matrix operator + (const matrix & a, const matrix & b) {
    if (a.r != b.r || a.c != b.c)
      throw std::runtime_error("Matrix dimensions don't match.");
    matrix res(a);
    for (int i = 0; i < res.r; i++)
      for (int j = 0; j < res.c; j++)
        res.mat[i][j] += b.mat[i][j];
    return res;
  }

  friend matrix operator - (const matrix & a, const matrix & b) {
    if (a.r != b.r || a.c != b.c)
      throw std::runtime_error("Matrix dimensions don't match.");
    matrix res(a);
    for (int i = 0; i < a.r; i++)
      for (int j = 0; j < a.c; j++)
        res.mat[i][j] -= b.mat[i][j];
    return res;
  }

  friend matrix operator * (const matrix & a, const matrix & b) {
    if (a.c != b.r)
      throw std::runtime_error("# of a cols must equal # of b rows.");
    matrix res(a.r, b.c, 0);
    for (int i = 0; i < a.r; i++)
      for (int j = 0; j < b.c; j++)
        for (int k = 0; k < a.c; k++)
          res.mat[i][j] += a.mat[i][k] * b.mat[k][j];
    return res;
  }

  friend matrix operator + (const matrix & a, const val_t & v) {
    matrix res(a);
    for (int i = 0; i < a.r; i++)
      for (int j = 0; j < a.c; j++) res.mat[i][j] += v;
    return res;
  }

  friend matrix operator - (const matrix & a, const val_t & v) {
    matrix res(a);
    for (int i = 0; i < a.r; i++)
      for (int j = 0; j < a.c; j++) res.mat[i][j] -= v;
    return res;
  }

  friend matrix operator * (const matrix & a, const val_t & v) {
    matrix res(a);
    for (int i = 0; i < a.r; i++)
      for (int j = 0; j < a.c; j++) res.mat[i][j] *= v;
    return res;
  }

  friend matrix operator / (const matrix & a, const val_t & v) {
    matrix res(a);
    for (int i = 0; i < a.r; i++)
      for (int j = 0; j < a.c; j++)
        res.mat[i][j] /= v;
    return res;
  }

  //raise matrix to the n-th power. precondition: a must be a square matrix
  friend matrix operator ^ (const matrix & a, unsigned int n) {
    if (a.r != a.c)
      throw std::runtime_error("Matrix must be square for exponentiation.");
    if (n == 0) return identity_matrix(a.r);
    if (n % 2 == 0) return (a * a) ^ (n / 2);
    return a * (a ^ (n - 1));
  }

  //returns a^1 + a^2 + ... + a^n
  friend matrix powsum(const matrix & a, unsigned int n) {
    if (n == 0) return matrix(a.r, a.r);
    if (n % 2 == 0)
      return powsum(a, n / 2) * (identity_matrix(a.r) + (a ^ (n / 2)));
    return a + a * powsum(a, n - 1);
  }

  matrix & operator += (const matrix & m) { *this = *this + m; return *this; }
  matrix & operator -= (const matrix & m) { *this = *this - m; return *this; }
  matrix & operator *= (const matrix & m) { *this = *this * m; return *this; }
  matrix & operator += (const val_t & v)  { *this = *this + v; return *this; }
  matrix & operator -= (const val_t & v)  { *this = *this - v; return *this; }
  matrix & operator *= (const val_t & v)  { *this = *this * v; return *this; }
  matrix & operator /= (const val_t & v)  { *this = *this / v; return *this; }
  matrix & operator ^= (unsigned int n)   { *this = *this ^ n; return *this; }

  static matrix identity_matrix(int n) {
    matrix res(n, n);
    for (int i = 0; i < n; i++) res[i][i] = 1;
    return res;
  }

  friend std::ostream & operator << (std::ostream & out, const matrix & m) {
    out << "[";
    for (int i = 0; i < m.r; i++) {
      out << (i > 0 ? ",[" : "[");
      for (int j = 0; j < m.c; j++)
        out << (j > 0 ? "," : "") << m.mat[i][j];
      out << "]";
    }
    out << "]";
    return out;
  }
};


/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  int a[2][2] = {{1,8}, {5,9}};
  matrix<int> m(5, 5, 10), m2(a);
  m += 10;
  m[0][0] += 10;
  assert(m[0][0] == 30 && m[1][1] == 20);
  assert(powsum(m2, 3) == m2 + m2*m2 + (m2^3));
  return 0;
}
