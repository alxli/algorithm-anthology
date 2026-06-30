/*

Polynomial operations treat a vector `a` as coefficients of $a_0 + a_1 x + \dots + a_{n-1} x^{n-1}$
over a field. This section implements the core algebra needed for formal power series and polynomial
division modulo the prime $998244353$. Multiplication uses the number theoretic transform, so
inverse and division are fast enough for large polynomials while still keeping the API close to the
underlying coefficient vectors.

The code aliases one modular field element as `Coeff` and one coefficient vector as `Poly`.

- `trim(a)` removes trailing zero coefficients from `a`.
- `add(a, b)` returns `a + b`.
- `subtract(a, b)` returns `a - b`.
- `multiply(a, b)` returns `a * b`.
- `derivative(a)` returns the formal derivative of `a`.
- `integral(a)` returns the formal antiderivative of `a` with constant term zero.
- `inverse(a, n)` returns the first `n` coefficients of `1 / a`, requiring `a[0]` to be nonzero.
- `divide(a, b)` returns the polynomial quotient of `a / b`, requiring `b` to be nonzero.
- `modulo(a, b)` returns the polynomial remainder of `a / b`, requiring `b` to be nonzero.

The inverse is a formal power series inverse modulo $x^n$: it finds $b$ such that
$a \cdot b \equiv 1 \pmod{x^n}$. Division uses the standard reversal trick: reverse both
polynomials, compute a truncated series inverse of the reversed divisor, multiply, truncate, and
reverse back.

Time Complexity:
- O(n) for `trim()`, `add()`, `subtract()`, `derivative()`, and `integral()`.
- O(|a||b|) for the small-input branch of `multiply()`, otherwise O(n log n), where $n$ is the
  padded transform length.
- O(n log^2 n) for `inverse(a, n)`, where $n$ is the requested length.
- O(n log n + m log^2 m) for `divide()` and `modulo()`, where $n$ is the output quotient length and
  $m$ is the divisor length.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

using Coeff = int64_t;
using Poly = std::vector<Coeff>;

const Coeff MOD = 998244353;
const Coeff ROOT = 3;
const int MAX_POWER_OF_TWO = 23;
const int NAIVE_CUTOFF = 150;

Coeff powmod(Coeff b, Coeff e, Coeff m = MOD) {
  Coeff res = 1;
  for (b %= m; e > 0; e >>= 1) {
    if (e & 1) {
      res = res * b % m;
    }
    b = b * b % m;
  }
  return res;
}

void ntt(Poly &a, bool invert) {
  int n = static_cast<int>(a.size());
  assert((n & (n - 1)) == 0 && n <= (1 << MAX_POWER_OF_TWO));
  for (int i = 1, j = 0; i < n; i++) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) {
      j ^= bit;
    }
    j ^= bit;
    if (i < j) {
      std::swap(a[i], a[j]);
    }
  }
  for (int len = 2; len <= n; len <<= 1) {
    Coeff root = powmod(ROOT, (MOD - 1) / len);
    if (invert) {
      root = powmod(root, MOD - 2);
    }
    for (int i = 0; i < n; i += len) {
      Coeff w = 1;
      for (int k = 0; k < len / 2; k++) {
        Coeff u = a[i + k], v = a[i + k + len / 2] * w % MOD;
        a[i + k] = (u + v) % MOD;
        a[i + k + len / 2] = (u - v + MOD) % MOD;
        w = w * root % MOD;
      }
    }
  }
  if (invert) {
    Coeff n_inv = powmod(n, MOD - 2);
    for (Coeff &x : a) {
      x = x * n_inv % MOD;
    }
  }
}

void trim(Poly &a) {
  while (!a.empty() && a.back() == 0) {
    a.pop_back();
  }
}

Poly add(const Poly &a, const Poly &b) {
  Poly res(std::max(a.size(), b.size()));
  for (int i = 0; i < static_cast<int>(res.size()); i++) {
    Coeff x = i < static_cast<int>(a.size()) ? a[i] : 0;
    Coeff y = i < static_cast<int>(b.size()) ? b[i] : 0;
    res[i] = (x + y) % MOD;
  }
  trim(res);
  return res;
}

Poly subtract(const Poly &a, const Poly &b) {
  Poly res(std::max(a.size(), b.size()));
  for (int i = 0; i < static_cast<int>(res.size()); i++) {
    Coeff x = i < static_cast<int>(a.size()) ? a[i] : 0;
    Coeff y = i < static_cast<int>(b.size()) ? b[i] : 0;
    res[i] = (x - y + MOD) % MOD;
  }
  trim(res);
  return res;
}

Poly multiply(Poly a, Poly b) {
  if (a.empty() || b.empty()) {
    return {};
  }
  int result_size = a.size() + b.size() - 1;
  if (std::min(a.size(), b.size()) < NAIVE_CUTOFF) {
    Poly res(result_size);
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      for (int j = 0; j < static_cast<int>(b.size()); j++) {
        res[i + j] = (res[i + j] + a[i] * b[j]) % MOD;
      }
    }
    trim(res);
    return res;
  }
  int n = 1;
  while (n < result_size) {
    n <<= 1;
  }
  assert(n <= (1 << MAX_POWER_OF_TWO));
  a.resize(n);
  b.resize(n);
  ntt(a, false);
  ntt(b, false);
  for (int i = 0; i < n; i++) {
    a[i] = a[i] * b[i] % MOD;
  }
  ntt(a, true);
  a.resize(result_size);
  trim(a);
  return a;
}

Poly derivative(const Poly &a) {
  if (a.size() <= 1) {
    return {};
  }
  Poly res(a.size() - 1);
  for (int i = 1; i < static_cast<int>(a.size()); i++) {
    res[i - 1] = a[i] * i % MOD;
  }
  trim(res);
  return res;
}

Poly integral(const Poly &a) {
  Poly res(a.size() + 1);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    res[i + 1] = a[i] * powmod(i + 1, MOD - 2) % MOD;
  }
  return res;
}

Poly inverse(const Poly &a, int n) {
  assert(n >= 0 && !a.empty() && a[0] != 0);
  if (n == 0) {
    return {};
  }
  Poly res{powmod(a[0], MOD - 2)};
  while (static_cast<int>(res.size()) < n) {
    int len = static_cast<int>(res.size()) << 1;
    Poly cut(a.begin(), a.begin() + std::min(static_cast<int>(a.size()), len));
    Poly prod = multiply(multiply(res, res), cut);
    res.resize(len);
    for (int i = len / 2; i < std::min(len, static_cast<int>(prod.size())); i++) {
      res[i] = (MOD - prod[i]) % MOD;
    }
  }
  res.resize(n);
  return res;
}

Poly divide(Poly a, Poly b) {
  trim(a);
  trim(b);
  assert(!b.empty());
  if (a.size() < b.size()) {
    return {};
  }
  int quotient_size = static_cast<int>(a.size() - b.size() + 1);
  std::reverse(a.begin(), a.end());
  std::reverse(b.begin(), b.end());
  b.resize(quotient_size);
  Poly q = multiply(a, inverse(b, quotient_size));
  q.resize(quotient_size);
  std::reverse(q.begin(), q.end());
  trim(q);
  return q;
}

Poly modulo(const Poly &a, const Poly &b) {
  Poly q = divide(a, b);
  Poly res = subtract(a, multiply(q, b));
  if (res.size() >= b.size()) {
    res.resize(b.size() - 1);
  }
  trim(res);
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // (1 + 2x + 3x^2)(4 + 5x + 6x^2) = 4 + 13x + 28x^2 + 27x^3 + 18x^4.
  Poly a{1, 2, 3}, b{4, 5, 6};
  assert((add(a, b) == Poly{5, 7, 9}));
  assert((subtract(b, a) == Poly{3, 3, 3}));
  assert((multiply(a, b) == Poly{4, 13, 28, 27, 18}));

  // d/dx (5 + 7x + 11x^2) = 7 + 22x; integrating back chooses constant term 0.
  Poly p{5, 7, 11};
  assert((derivative(p) == Poly{7, 22}));
  assert((integral(derivative(p)) == Poly{0, 7, 11}));

  // (1 - x)^-1 = 1 + x + x^2 + x^3 + ... modulo x^6.
  Poly inv = inverse({1, MOD - 1}, 6);
  assert((inv == Poly{1, 1, 1, 1, 1, 1}));

  // (x^3 - 1) / (x - 1) = x^2 + x + 1, remainder 0.
  Poly dividend{MOD - 1, 0, 0, 1};
  Poly divisor{MOD - 1, 1};
  assert((divide(dividend, divisor) == Poly{1, 1, 1}));
  assert(modulo(dividend, divisor).empty());

  Poly ones(160, 1);
  Poly square = multiply(ones, ones);
  assert(square[0] == 1 && square[159] == 160 && square[318] == 1);
  return 0;
}
