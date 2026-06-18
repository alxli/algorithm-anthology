/*

Given integers $a$, $b$, and a modulus $m$, the discrete logarithm problem asks for the smallest
nonnegative integer $x$ with $a^x$ congruent to $b$ modulo $m$. This is the modular analogue of a
logarithm and underlies Diffie-Hellman key exchange, order computations in the multiplicative group,
and many number-theory reductions.

The baby-step giant-step method writes $x = np - q$ for a block size $n$ near the square root of the
modulus. It tabulates the "baby steps" $b \cdot a^q$ for $q$ in $[0, n]$, then walks the "giant
steps" $a^{np}$ for increasing $p$ until one lands in the table, yielding $x = np - q$. This trades
the linear scan of all exponents for O(sqrt(m)) work and memory. A short reduction loop first
strips common factors between $a$ and $m$, so the routine works for any modulus, not only when $a$
and $m$ are coprime.

- `discrete_log(a, b, m)` returns the smallest nonnegative `x` with `a^x` congruent to `b` modulo
  `m`, or $-1$ if no such `x` exists. Requires `m` $\geq 1$; `a` and `b` are reduced modulo `m`
  internally.

Time Complexity:
- O(sqrt(m) * log(m)) per call to `discrete_log()`, dominated by the hashed baby-step table.

Space Complexity:
- O(sqrt(m)) auxiliary heap space for the table.

*/

#include <cmath>
#include <cstdint>
#include <numeric>
#include <unordered_map>

int64_t mulmod(int64_t a, int64_t b, int64_t m) {
#if defined(__SIZEOF_INT128__)
  return static_cast<int64_t>(static_cast<__uint128_t>(a) * b % m);
#else
  int64_t res = 0;
  for (a %= m; b > 0; b >>= 1) {
    if (b & 1) {
      res = res >= m - a ? res - (m - a) : res + a;
    }
    a = a >= m - a ? a - (m - a) : a + a;
  }
  return res;
#endif
}

int64_t discrete_log(int64_t a, int64_t b, int64_t m) {
  a %= m;
  b %= m;
  if (a < 0) {
    a += m;
  }
  if (b < 0) {
    b += m;
  }
  // Strip common factors of a and m so that a becomes invertible; add counts the exponents removed.
  int64_t k = 1 % m, add = 0, g;
  while ((g = std::gcd(a, m)) > 1) {
    if (b == k) {
      return add;
    }
    if (b % g != 0) {
      return -1;
    }
    b /= g;
    m /= g;
    add++;
    k = mulmod(k, a / g, m);
  }
  int64_t n = static_cast<int64_t>(std::sqrt(static_cast<double>(m))) + 1;
  int64_t an = 1;
  for (int64_t i = 0; i < n; i++) {
    an = mulmod(an, a, m);  // an = a^n, the giant step.
  }
  std::unordered_map<int64_t, int64_t> baby;
  int64_t cur = b;
  for (int64_t q = 0; q <= n; q++) {
    baby[cur] = q;  // Keep the largest q for each value to make the answer minimal.
    cur = mulmod(cur, a, m);
  }
  cur = k;
  for (int64_t p = 1; p <= n; p++) {
    cur = mulmod(cur, an, m);
    auto it = baby.find(cur);
    if (it != baby.end()) {
      return n * p - it->second + add;
    }
  }
  return -1;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // 2^x = 9 (mod 11): 2^6 = 64 = 9, and 6 is the smallest such exponent.
  assert(discrete_log(2, 9, 11) == 6);

  assert(discrete_log(3, 1, 7) == 0);    // a^0 = 1 always.
  assert(discrete_log(2, 3, 5) == 3);    // 2^3 = 8 = 3 (mod 5).
  assert(discrete_log(2, 1, 6) == 0);    // Works for composite, non-coprime moduli.
  assert(discrete_log(4, 6, 8) == -1);   // No power of 4 is 6 (mod 8).
  assert(discrete_log(5, 33, 58) == 9);  // 5^9 = 33 (mod 58).
  return 0;
}
