/*

Core integer and modular-arithmetic operations built around the Euclidean algorithm. Extended GCD
produces Bezout coefficients, which solve linear Diophantine equations and modular inverses, while
the Chinese remainder routines combine compatible congruences into one residue class.

- `gcd(a, b)` returns the greatest common divisor of `a` and `b` using the Euclidean algorithm. This
  is mainly for educational purposes, as `std::gcd(a, b)` from `<numeric>` is available as of C++17
  (`__gcd(a, b)` from `<algorithm>` in C++14 and earlier).
- `lcm(a, b)` returns the least common multiple of `a` and `b`. This implementation is mainly for
  educational purposes, as `std::lcm(a, b)` from `<numeric>` is available as of C++17.
- `extended_gcd(a, b)` returns a pair $(x, y)$ of integers such that $\gcd(a, b) = ax + by$.
- `diophantine(a, b, c, &g, &x, &y)` solves the linear Diophantine equation $ax + by = c$, returning
  whether a solution exists (one does if and only if $\gcd(a, b)$ divides $c$). `g` is always set to
  $\gcd(a, b)$, while (`x`, `y`) is set only on success to a particular solution bounded by
  $\max(|a|, |b|, |c|)$ in magnitude. Every other solution is $(`x` + t(`b`/`g`), `y` - t(`a`/`g`))$
  for an integer $t$. A 128-bit intermediate is used where available to keep the scaling step
  overflow-free.
- `mod(a, m)` returns the least nonnegative residue of `a` modulo `m`, that is, the unique value in
  $[0, `m`)$ congruent to `a`, where `m` must be positive. Unlike the C++ remainder operator `%`,
  whose result follows the sign of `a`, the result is never negative.
- `mod_inverse(a, m)` returns an integer $x$ such that $ax \equiv 1 \pmod m$, where the arguments
  must satisfy $m > 0$ and $\gcd(a, m) = 1$.
- `mod_inverse_table(p)` returns a vector `v` where `v[i]` is the modular inverse of `i` modulo the
  prime `p` for every valid index `i`.
- `crt(r1, m1, r2, m2, &r, &m)` merges the two congruences $x \equiv r_1 \pmod{m_1}$ and
  $x \equiv r_2 \pmod{m_2}$ for arbitrary moduli (not necessarily coprime). It returns whether the
  system is consistent, and on success sets `m` to `lcm(m_1, m_2)` and `r` to the unique solution in
  $[0, `m`)$. Both moduli must be positive, and their least common multiple must fit in `int64_t`.
  Fold it pairwise to merge more than two congruences.
- `garner_restore(a, p)` returns the smallest nonnegative solution whose residue modulo `p[i]` is
  `a[i]` at every valid index. The entries of `p` must be pairwise coprime (unlike `crt`, which
  allows shared factors) and positive, and `a` and `p` must have equal sizes. The exact solution is
  unique modulo the product of all moduli, so that product and the final answer must fit in
  `int64_t`.
- `garner_restore_mod(a, p, m)` returns that same CRT solution modulo `m`. This is the right variant
  when the product of the moduli is too large to fit in `int64_t`; `m` must be positive.

Overflow warning: Every intermediate and result of the templated signed-integer helpers must be
representable by `Int`; in particular, the minimum value of `Int` cannot be negated or divided by
$-1$.

Time Complexity:
- O(log M) per call to `gcd()`, `lcm()`, `extended_gcd()`, `diophantine()`, `mod_inverse()`, and
  `crt()`, where $M$ is the largest relevant input magnitude.
- O(1) per call to `mod()`.
- O(p) per call to `mod_inverse_table()`.
- O(n^2) per call to `garner_restore()` and `garner_restore_mod()`.

Space Complexity:
- O(p) auxiliary for `mod_inverse_table()`.
- O(n) auxiliary for `garner_restore()` and `garner_restore_mod()`.
- O(1) auxiliary for all other operations.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

template<typename Int>
Int gcd(Int a, Int b) {
  while (b != 0) {
    Int t = b;
    b = a % b;
    a = t;
  }
  return (a < 0 ? -a : a);
}

template<typename Int>
Int lcm(Int a, Int b) {
  if (a == 0 || b == 0) {
    return 0;
  }
  Int res = a / gcd(a, b) * b;
  return (res < 0 ? -res : res);
}

template<typename Int>
std::pair<Int, Int> extended_gcd(Int a, Int b) {
  Int x = 1, y = 0, x1 = 0, y1 = 1;
  while (b != 0) {
    Int q = a / b, prev_x1 = x1, prev_y1 = y1, prev_b = b;
    x1 = x - q * x1;
    y1 = y - q * y1;
    b = a - q * b;
    x = prev_x1;
    y = prev_y1;
    a = prev_b;
  }
  return (a > 0) ? std::pair<Int, Int>{x, y} : std::pair<Int, Int>{-x, -y};
}

template<typename Int>
bool diophantine(Int a, Int b, Int c, Int *g, Int *x, Int *y) {
  if (a == 0 && b == 0) {
    *g = *x = *y = 0;
    return c == 0;
  }
  if (a == 0) {
    *g = (b < 0) ? -b : b;
    if (c % b != 0) {
      return false;
    }
    *x = 0;
    *y = c / b;
    return true;
  }
  if (b == 0) {
    *g = (a < 0) ? -a : a;
    if (c % a != 0) {
      return false;
    }
    *x = c / a;
    *y = 0;
    return true;
  }
  *g = gcd(a, b);
  if (c % *g != 0) {
    return false;
  }
  // Absorb the bulk of c into a*dx + b*dy, then scale the base solution by the small remainder so
  // the reported (x, y) stay bounded by max(|a|, |b|, |c|). The scaled product is taken modulo b
  // (resp. a) through a 128-bit intermediate where available, to avoid overflow.
  std::pair<Int, Int> base = extended_gcd(a, b);
  Int dx = c / a;
  c -= dx * a;
  Int dy = c / b;
  c -= dy * b;
  Int f = c / *g;
#if defined(__SIZEOF_INT128__)
  __extension__ typedef std::conditional_t<sizeof(Int) <= 4, int64_t, __int128> wide_t;
#else
  using wide_t = int64_t;
#endif
  *x = dx + static_cast<Int>(static_cast<wide_t>(base.first) * f % b);
  *y = dy + static_cast<Int>(static_cast<wide_t>(base.second) * f % a);
  return true;
}

template<typename Int>
Int mod(Int a, Int m) {
  assert(m > 0);
  Int r = a % m;
  return (r < 0) ? (r + m) : r;
}

template<typename Int>
Int mod_inverse(Int a, Int m) {
  assert(m > 0 && gcd(a, m) == 1);
  return mod(extended_gcd(a, m).first, m);
}

std::vector<int> mod_inverse_table(int p) {
  assert(p >= 2);
  std::vector<int> res(p);
  res[1] = 1;
  for (int i = 2; i < p; i++) {
    res[i] = (p - (p / i) * res[p % i] % p) % p;
  }
  return res;
}

bool crt(int64_t r1, int64_t m1, int64_t r2, int64_t m2, int64_t *r, int64_t *m) {
  r1 = mod(r1, m1);
  r2 = mod(r2, m2);
  int64_t g, x, y;
  if (!diophantine(m1, -m2, r2 - r1, &g, &x, &y)) {
    return false;
  }
  *m = m1 / g * m2;
#if defined(__SIZEOF_INT128__)
  __extension__ typedef __int128 int128_t;
  *r = static_cast<int64_t>(
      (static_cast<int128_t>(r1) + static_cast<int128_t>(m1) * mod(x, m2 / g)) % *m
  );
#else
  *r = mod(r1 + m1 * mod(x, m2 / g), *m);  // Overflow warning.
#endif
  return true;
}

std::vector<int64_t> garner_digits(const std::vector<int> &a, const std::vector<int> &p) {
  assert(a.size() == p.size());
  assert(std::all_of(p.begin(), p.end(), [](int m) { return m > 0; }));
  int n = static_cast<int>(a.size());
  std::vector<int64_t> x(n);
  for (int i = 0; i < n; i++) {
    x[i] = mod(static_cast<int64_t>(a[i]), static_cast<int64_t>(p[i]));
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      // Reduce mod p[i] each step; otherwise x[i] compounds to ~p^i and overflows int64_t.
      x[i] = mod_inverse(static_cast<int64_t>(p[j]), static_cast<int64_t>(p[i])) * (x[i] - x[j]);
      x[i] = (x[i] % p[i] + p[i]) % p[i];
    }
  }
  return x;
}

int64_t garner_restore(const std::vector<int> &a, const std::vector<int> &p) {
  assert(a.size() == p.size());
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return 0;
  }
  std::vector<int64_t> x = garner_digits(a, p);
  int64_t res = x[0], m = 1;
  for (int i = 1; i < n; i++) {
    m *= p[i - 1];
    res += x[i] * m;
  }
  return res;
}

int64_t garner_restore_mod(const std::vector<int> &a, const std::vector<int> &p, int64_t m) {
  assert(a.size() == p.size() && m > 0);
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return 0;
  }
  std::vector<int64_t> x = garner_digits(a, p);
  int64_t res = 0;
  for (int i = n - 1; i >= 0; i--) {
#if defined(__SIZEOF_INT128__)
    __extension__ typedef __int128 int128_t;
    res = static_cast<int64_t>((static_cast<int128_t>(res) * p[i] + x[i]) % m);
#else
    res = (res * p[i] + x[i]) % m;  // Requires the intermediate product to fit without int128.
#endif
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(mod(-5, 3) == 1);  // Negative dividends wrap up into [0, m).
  assert(mod(5, 3) == 2);
  assert(mod(-6, 3) == 0);
  {
    for (int a = -20; a <= 20; a++) {
      for (int b = -20; b <= 20; b++) {
        int g = gcd(a, b);
        auto [x, y] = extended_gcd(a, b);
        assert(g == a * x + b * y);
        if (g == 1 && b > 1) {
          assert(mod(a * mod_inverse(a, b), b) == 1);
        }
      }
    }
  }
  {
    int p = 17;
    auto res = mod_inverse_table(p);
    for (int i = 0; i < p; i++) {
      if (i > 0) {
        assert(mod(i * res[i], p) == 1);
      }
    }
  }
  {
    vector<int> a{2, 3, 1}, m{3, 4, 5};
    int x = garner_restore(a, m);
    assert(x == garner_restore_mod(a, m, 1000000007));
    int n = static_cast<int>(a.size());
    for (int i = 0; i < n; i++) {
      assert(mod(x, m[i]) == a[i]);
    }
    assert(x == 11);
  }
  assert(garner_restore(vector<int>{-1}, vector<int>{5}) == 4);
#if defined(__SIZEOF_INT128__)
  {  // Garner modulo another number works even when the product of CRT moduli is too large.
    vector<int> p{1000000007, 1000000009, 1000000033};
    __extension__ typedef __int128 int128_t;
    int128_t x = (int128_t{1} << 80) + 123456789;
    vector<int> a;
    for (int m : p) {
      a.push_back(static_cast<int>(x % m));
    }
    int64_t m = 998244353;
    assert(garner_restore_mod(a, p, m) == static_cast<int64_t>(x % m));
  }
#endif
  {  // Diophantine: exhaustively verify solvability and that solutions satisfy a*x + b*y == c.
    for (int a = -8; a <= 8; a++) {
      for (int b = -8; b <= 8; b++) {
        for (int c = -8; c <= 8; c++) {
          int g, x, y, gg = gcd(a, b);
          bool ok = diophantine(a, b, c, &g, &x, &y);
          assert(ok == (gg == 0 ? c == 0 : c % gg == 0));
          assert(g == gg);  // g is set even when no solution exists.
          if (ok) {
            assert(a * x + b * y == c);
          }
        }
      }
    }
  }
  {  // CRT: merge two congruences, including non-coprime and inconsistent moduli.
    for (int m1 = 1; m1 <= 12; m1++) {
      for (int m2 = 1; m2 <= 12; m2++) {
        for (int r1 = 0; r1 < m1; r1++) {
          for (int r2 = 0; r2 < m2; r2++) {
            int64_t r, m;
            bool ok = crt(r1, m1, r2, m2, &r, &m);
            int limit = lcm(m1, m2), want = -1;
            for (int v = 0; v < limit && want < 0; v++) {
              if (v % m1 == r1 && v % m2 == r2) {
                want = v;
              }
            }
            assert(ok == (want >= 0));
            if (ok) {
              assert(m == limit && r == want);
            }
          }
        }
      }
    }
  }
  return 0;
}
