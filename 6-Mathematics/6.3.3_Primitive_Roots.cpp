/*

A primitive root modulo $m$ is a number $g$ whose powers generate every invertible residue modulo
$m$. Equivalently, the multiplicative order of $g$ modulo $m$ is exactly $\phi(m)$. Primitive roots
are useful for constructing generators of finite multiplicative groups, choosing roots for number
theoretic transforms, discrete logarithm setups, and randomized modular hashing or testing schemes
that need a full-period multiplicative generator.

Primitive roots do not exist for every $m$. They exist exactly for $m \in \{1, 2, 4, p^k, 2p^k\}$,
where $p$ is an odd prime and $k \geq 1$. This implementation first checks that classification by
factoring $m$, then factors $\phi(m)$. A candidate $g$ is primitive exactly when
$g^{\phi(m)} \equiv 1 \pmod m$ and $g^{\phi(m)/q} \not\equiv 1 \pmod m$ for every prime factor
$q$ of $\phi(m)$.

- `primitive_root(m)` returns the smallest primitive root modulo `m`, or `-1` if no primitive root
  exists.

For a prime modulus $p$, this reduces to the common contest test: factor $p - 1$, then scan for the
first $g$ such that $g^{(p - 1)/q} \not\equiv 1 \pmod p$ for every prime factor $q$ of $p - 1$.

Modular multiplication uses `__uint128_t` when available for speed, with a slower portable
double-and-add fallback to avoid overflow on compilers without 128-bit integers.

Time Complexity:
- O(sqrt m + g*k*log m), where `g` is the answer candidate tested and $k$ is the number of distinct
  prime factors of $\phi(m)$.

Space Complexity:
- O(k) auxiliary heap space.

*/

#include <cstdint>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

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

int64_t powmod(int64_t a, int64_t n, int64_t m) {
  int64_t res = 1 % m;
  for (a %= m; n > 0; n >>= 1) {
    if (n & 1) {
      res = mulmod(res, a, m);
    }
    a = mulmod(a, a, m);
  }
  return res;
}

std::vector<std::pair<int64_t, int>> factorize(int64_t n) {
  std::vector<std::pair<int64_t, int>> res;
  for (int64_t p = 2; p <= n / p; p += (p == 2 ? 1 : 2)) {
    if (n % p != 0) {
      continue;
    }
    int count = 0;
    while (n % p == 0) {
      n /= p;
      count++;
    }
    res.push_back({p, count});
  }
  if (n > 1) {
    res.push_back({n, 1});
  }
  return res;
}

int64_t primitive_root(int64_t m) {
  if (m <= 0) {
    return -1;
  }
  if (m == 1 || m == 2 || m == 4) {
    return m - 1;
  }
  std::vector<std::pair<int64_t, int>> factors = factorize(m);
  bool has_root = false;
  if (factors[0].first == 2) {
    has_root = factors[0].second == 1 && factors.size() == 2;
  } else {
    has_root = factors.size() == 1;
  }
  if (!has_root) {
    return -1;
  }
  int64_t phi = m;
  std::set<int64_t> phi_factors;
  for (auto [p, e] : factors) {
    phi = phi / p * (p - 1);
    if (e > 1) {
      phi_factors.insert(p);
    }
    for (auto [q, count] : factorize(p - 1)) {
      phi_factors.insert(q);
    }
  }
  for (int64_t g = 1; g < m; g++) {
    if (std::gcd(g, m) != 1 || powmod(g, phi, m) != 1) {
      continue;
    }
    bool ok = true;
    for (int64_t q : phi_factors) {
      if (powmod(g, phi / q, m) == 1) {
        ok = false;
        break;
      }
    }
    if (ok) {
      return g;
    }
  }
  return -1;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(primitive_root(1) == 0);
  assert(primitive_root(2) == 1);
  assert(primitive_root(4) == 3);

  assert(primitive_root(17) == 3);         // 3 has order phi(17) = 16.
  assert(primitive_root(998244353) == 3);  // Common NTT prime.
  assert(primitive_root(9) == 2);          // Odd prime power.
  assert(primitive_root(18) == 5);         // Twice an odd prime power.

  assert(primitive_root(8) == -1);
  assert(primitive_root(12) == -1);
  return 0;
}
