/*

Polynomial operations treat a vector `a` as coefficients of $a_0 + a_1 x + \dots + a_{n-1} x^{n-1}$
over a field. This section implements the core algebra needed for formal power series and polynomial
division modulo the prime $998244353$. Multiplication uses the number theoretic transform, so
inverse and division are fast enough for large polynomials while still keeping the API close to the
underlying coefficient vectors.

The code aliases one modular field element as `Coeff` and one coefficient vector as `Poly`. Input
coefficients must lie in $[0, `MOD`)$.

- `eval(a, x)` returns the value of `a` at `x` modulo `MOD`; `x` is reduced modulo `MOD`.
- `add(a, b)` returns `a + b`.
- `subtract(a, b)` returns `a - b`.
- `multiply(a, b)` returns `a * b`.
- `derivative(a)` returns the formal derivative of `a`.
- `integral(a)` returns the formal antiderivative of `a` with constant term zero.
- `inverse(a, n)` returns the first `n` coefficients of `1 / a`, requiring `a[0]` to be nonzero.
- `log(a, n)` returns the first `n` coefficients of $\log a$, requiring `a[0]` to be $1$.
- `exp(a, n)` returns the first `n` coefficients of $\exp a$, requiring `a[0]` to be $0$.
- `sqrt(a, n)` returns the first `n` coefficients of the square root of `a` whose constant term is
  $1$, requiring `a[0]` to be $1$.
- `power(a, k, n)` returns the first `n` coefficients of $a^k$ for a nonnegative `uint64_t` exponent
  `k`; `k = 0` returns the constant series $1$.
- `divide(a, b)` returns the polynomial quotient of `a / b`, requiring `b` to be nonzero.
- `modulo(a, b)` returns the polynomial remainder of `a / b`, requiring `b` to be nonzero.

The inverse is a formal power series inverse modulo $x^n$: it finds $b$ such that
$a \cdot b \equiv 1 \pmod{x^n}$. Division uses the standard reversal trick: reverse both
polynomials, compute a truncated series inverse of the reversed divisor, multiply, truncate, and
reverse back.

The logarithm follows from $(\log a)' = a'/a$, integrating the truncated quotient. The exponential
and square root use Newton iteration, which doubles the number of correct coefficients each round
and so costs the same asymptotically as the final multiplication. To take a square root whose
constant term is not $1$, factor `a[0]` out and multiply back one of its modular square roots (see
6.3.5); if the lowest nonzero coefficient sits at an odd power of $x$, no square root exists. Power
factors $a = x^t c b$, where $b(0) = 1$, and uses $a^k = x^{tk} c^k \exp(k \log b)$.

Time Complexity:
- O(n) per call to `eval()`, `add()`, `subtract()`, `derivative()`, and `integral()`.
- O(|a||b|) per call to `multiply()` on small inputs and O(n log n) otherwise, where $n$ is the
  padded transform length.
- O(n log n) per call to `inverse(a, n)`, where $n$ is the requested length.
- O(n log n) per call to `log()`, `exp()`, and `sqrt()`, where $n$ is the requested length. The
  exponential carries the largest constant, since every Newton round computes a logarithm.
- O(n log n + log k) per call to `power()`, where $n$ is the requested length.
- O(n log n) per call to `divide()` and `modulo()`, where $n$ is the padded multiplication length.

Space Complexity:
- O(1) auxiliary per call to `eval()` and O(n) auxiliary for all other operations.

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

Coeff powmod(Coeff b, uint64_t e, Coeff m = MOD) {
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
  assert(n > 0 && (n & (n - 1)) == 0 && n <= (1 << MAX_POWER_OF_TWO));
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

Coeff eval(const Poly &a, Coeff x) {
  x = (x % MOD + MOD) % MOD;
  Coeff res = 0;
  for (auto it = a.rbegin(); it != a.rend(); ++it) {
    res = (res * x + *it) % MOD;
  }
  return res;
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
  int result_size = static_cast<int>(a.size() + b.size() - 1);
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

Poly log(const Poly &a, int n) {
  assert(n >= 0 && !a.empty() && a[0] == 1);
  if (n == 0) {
    return {};
  }
  // Since (log a)' = a'/a, integrating the truncated quotient recovers log a.
  Poly cut(a.begin(), a.begin() + std::min(static_cast<int>(a.size()), n));
  Poly res = multiply(derivative(cut), inverse(cut, n));
  res.resize(n - 1);
  res = integral(res);
  res.resize(n);
  return res;
}

Poly exp(const Poly &a, int n) {
  assert(n >= 0 && (a.empty() || a[0] == 0));
  if (n == 0) {
    return {};
  }
  // Newton iteration on log(f) - a = 0 gives f <- f*(1 - log(f) + a), doubling the number of
  // correct coefficients each round.
  Poly res{1};
  while (static_cast<int>(res.size()) < n) {
    int len = static_cast<int>(res.size()) << 1;
    Poly cut(a.begin(), a.begin() + std::min(static_cast<int>(a.size()), len));
    Poly correction = subtract(cut, log(res, len));
    correction.resize(std::max(1, static_cast<int>(correction.size())));
    correction[0] = (correction[0] + 1) % MOD;
    res = multiply(res, correction);
    res.resize(len);
  }
  res.resize(n);
  return res;
}

Poly sqrt(const Poly &a, int n) {
  assert(n >= 0 && !a.empty() && a[0] == 1);
  if (n == 0) {
    return {};
  }
  // Newton iteration on f^2 - a = 0 gives f <- (f + a/f) / 2.
  const Coeff INV2 = powmod(2, MOD - 2);
  Poly res{1};
  while (static_cast<int>(res.size()) < n) {
    int len = static_cast<int>(res.size()) << 1;
    Poly cut(a.begin(), a.begin() + std::min(static_cast<int>(a.size()), len));
    res = add(res, multiply(cut, inverse(res, len)));
    res.resize(len);
    for (Coeff &c : res) {
      c = c * INV2 % MOD;
    }
  }
  res.resize(n);
  return res;
}

Poly power(const Poly &a, uint64_t k, int n) {
  assert(n >= 0);
  if (n == 0) {
    return {};
  }
  Poly res(n);
  if (k == 0) {
    res[0] = 1;
    return res;
  }
  int limit = std::min(static_cast<int>(a.size()), n);
  int first = 0;
  while (first < limit && a[first] == 0) {
    first++;
  }
  if (first == limit || (first > 0 && k > static_cast<uint64_t>((n - 1) / first))) {
    return res;
  }
  int shift = static_cast<int>(static_cast<uint64_t>(first) * k);
  int len = n - shift;
  int terms = std::min(static_cast<int>(a.size()) - first, len);
  Poly normalized(a.begin() + first, a.begin() + first + terms);
  Coeff leading = normalized[0];
  Coeff leading_inv = powmod(leading, MOD - 2);
  for (Coeff &c : normalized) {
    c = c * leading_inv % MOD;
  }
  Poly exponent = log(normalized, len);
  Coeff k_mod = static_cast<Coeff>(k % static_cast<uint64_t>(MOD));
  for (Coeff &c : exponent) {
    c = c * k_mod % MOD;
  }
  Poly body = exp(exponent, len);
  Coeff leading_power = powmod(leading, k);
  for (int i = 0; i < len; i++) {
    res[i + shift] = body[i] * leading_power % MOD;
  }
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
  // 5 + 7x + 11x^2 evaluates to 63 at x = 2 and 9 at x = -1.
  Poly p{5, 7, 11};
  assert(eval(p, 2) == 63 && eval(p, -1) == 9);

  // (1 + 2x + 3x^2)(4 + 5x + 6x^2) = 4 + 13x + 28x^2 + 27x^3 + 18x^4.
  Poly a{1, 2, 3}, b{4, 5, 6};
  assert((add(a, b) == Poly{5, 7, 9}));
  assert((subtract(b, a) == Poly{3, 3, 3}));
  assert((multiply(a, b) == Poly{4, 13, 28, 27, 18}));

  // d/dx (5 + 7x + 11x^2) = 7 + 22x; integrating back chooses constant term 0.
  assert((derivative(p) == Poly{7, 22}));
  assert((integral(derivative(p)) == Poly{0, 7, 11}));

  // (1 - x)^-1 = 1 + x + x^2 + x^3 + ... modulo x^6.
  assert((inverse({1, MOD - 1}, 6) == Poly{1, 1, 1, 1, 1, 1}));

  // (x^3 - 1) / (x - 1) = x^2 + x + 1, remainder 0.
  Poly dividend{MOD - 1, 0, 0, 1};
  Poly divisor{MOD - 1, 1};
  assert((divide(dividend, divisor) == Poly{1, 1, 1}));
  assert(modulo(dividend, divisor).empty());

  // exp(x) = 1 + x + x^2/2 + x^3/6 + ..., checked by clearing each denominator.
  Poly exp_x = exp({0, 1}, 4);
  assert(exp_x[0] == 1 && exp_x[1] == 1);
  assert(exp_x[2] * 2 % MOD == 1 && exp_x[3] * 6 % MOD == 1);

  // log(1 + x) = x - x^2/2 + x^3/3 - ...
  Poly log_1x = log({1, 1}, 4);
  assert(log_1x[0] == 0 && log_1x[1] == 1);
  assert((log_1x[2] * 2 + 1) % MOD == 0 && log_1x[3] * 3 % MOD == 1);

  // Truncated to the same length, exp and log invert each other.
  Poly series{1, 5, 9, 2, 7};
  Poly logged = log(series, 5);
  assert(logged[0] == 0);
  assert(exp(logged, 5) == series);

  // A series square root squares back to the original, modulo x^5.
  Poly root = sqrt(series, 5);
  Poly root_squared = multiply(root, root);
  root_squared.resize(5);
  assert(root_squared == series);

  // (2x + x^2)^3 = 8x^3 + 12x^4 + 6x^5 + x^6.
  assert((power({0, 2, 1}, 3, 7) == Poly{0, 0, 0, 8, 12, 6, 1}));
  assert((power({}, 0, 4) == Poly{1, 0, 0, 0}));

  Poly ones(160, 1);
  Poly square = multiply(ones, ones);
  assert(square[0] == 1 && square[159] == 160 && square[318] == 1);
  return 0;
}
