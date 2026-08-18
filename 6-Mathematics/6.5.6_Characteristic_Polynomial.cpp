/*

Computes the characteristic polynomial of a square matrix over a prime field: the coefficients of
$\det(xI - A)$, a monic degree-$n$ polynomial whose roots are the eigenvalues of $A$. Its constant
term recovers the determinant (up to sign), its second-highest coefficient is the negated trace, and
evaluating it at a scalar gives $\det(xI - A)$ for that $x$. It is the standard tool behind
eigenvalue counting, matrix-power recurrences via Cayley-Hamilton, and problems that reduce to a
determinant in an unknown.

A direct expansion would cost a determinant per coefficient. Instead the matrix is first reduced to
upper Hessenberg form, where every entry more than one place below the diagonal is zero. This
reduction uses similarity transforms (a row operation paired with the inverse column operation),
which leave the characteristic polynomial unchanged. The characteristic polynomial of a Hessenberg
matrix then follows from a short recurrence over its leading principal submatrices, adding one row
and column at a time, for an overall cost of O(n^3). The arithmetic is exact modulo a prime, since
the Hessenberg reduction divides by pivots and is not numerically stable over the reals.

- `characteristic_polynomial(a)` returns a vector $c$ of length $n + 1$ with
  $\det(xI - `a`) = c_0 + c_1 x + ... + c_n x^n$ modulo `MOD`, where $c_n = 1$. The matrix `a` must
  be square; its entries are reduced modulo `MOD` internally.

Time Complexity:
- O(n^3) per call, where $n$ is the dimension of the matrix.

Space Complexity:
- O(n^2) auxiliary.

*/

#include <cstdint>
#include <utility>
#include <vector>

const int64_t MOD = 998244353;

int64_t powmod(int64_t x, int64_t n, int64_t m) {
  int64_t res = 1 % m;
  for (x %= m; n > 0; n >>= 1) {
    if (n & 1) {
      res = res * x % m;
    }
    x = x * x % m;
  }
  return res;
}

void to_hessenberg(std::vector<std::vector<int64_t>> &m) {
  int n = static_cast<int>(m.size());
  for (int j = 0; j + 2 < n; j++) {
    int pivot = -1;
    for (int i = j + 1; i < n; i++) {
      if (m[i][j] != 0) {
        pivot = i;
        break;
      }
    }
    if (pivot == -1) {
      continue;
    }
    if (pivot != j + 1) {  // Move the pivot to the subdiagonal via a similarity swap.
      std::swap(m[pivot], m[j + 1]);
      for (int r = 0; r < n; r++) {
        std::swap(m[r][pivot], m[r][j + 1]);
      }
    }
    int64_t pinv = powmod(m[j + 1][j], MOD - 2, MOD);
    for (int k = j + 2; k < n; k++) {
      int64_t u = m[k][j] * pinv % MOD;
      if (u == 0) {
        continue;
      }
      for (int c = 0; c < n; c++) {  // Row k -= u * row (j + 1).
        m[k][c] = (m[k][c] - u * m[j + 1][c] % MOD + MOD) % MOD;
      }
      for (int r = 0; r < n; r++) {  // Inverse column operation keeps it a similarity.
        m[r][j + 1] = (m[r][j + 1] + u * m[r][k]) % MOD;
      }
    }
  }
}

std::vector<int64_t> characteristic_polynomial(std::vector<std::vector<int64_t>> a) {
  int n = static_cast<int>(a.size());
  for (auto &row : a) {
    for (int64_t &x : row) {
      x = (x % MOD + MOD) % MOD;
    }
  }
  to_hessenberg(a);
  std::vector<std::vector<int64_t>> p(n + 1);
  p[0] = {1};
  for (int k = 0; k < n; k++) {
    p[k + 1].assign(k + 2, 0);
    for (int i = 0; i < static_cast<int>(p[k].size()); i++) {
      p[k + 1][i + 1] = (p[k + 1][i + 1] + p[k][i]) % MOD;                // x * p[k].
      p[k + 1][i] = (p[k + 1][i] - a[k][k] * p[k][i] % MOD + MOD) % MOD;  // -a[k][k] * p[k].
    }
    int64_t t = 1;
    for (int i = 0; i < k; i++) {
      t = t * a[k - i][k - i - 1] % MOD;  // Running product of subdiagonal entries.
      int64_t coeff = t * a[k - i - 1][k] % MOD;
      for (int j = 0; j < static_cast<int>(p[k - i - 1].size()); j++) {
        p[k + 1][j] = (p[k + 1][j] - coeff * p[k - i - 1][j] % MOD + MOD) % MOD;
      }
    }
  }
  return p[n];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // [[1, 2], [3, 4]]: det(xI - A) = x^2 - 5x - 2.
  vector<vector<int64_t>> a{{1, 2}, {3, 4}};
  assert((characteristic_polynomial(a) == vector<int64_t>{MOD - 2, MOD - 5, 1}));  // -2, -5, 1.

  // det(xI - B) = (x - 3)(x^2 - 7x + 6) = x^3 - 10x^2 + 27x - 18.
  vector<vector<int64_t>> b{{2, 0, 1}, {0, 3, 0}, {4, 1, 5}};
  assert((characteristic_polynomial(b) == vector<int64_t>{MOD - 18, 27, MOD - 10, 1}));
  return 0;
}
