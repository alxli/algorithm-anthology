/*

Common number theory operations relating to modular arithmetic.

- `gcd(a, b)` returns the greatest common divisor of `a` and `b` using the Euclidean algorithm. This
  is mainly for educational purposes, as `std::gcd(a, b)` from `<numeric>` is available as of C++17
  (`__gcd(a, b)` from `<algorithm>` in C++14 and earlier).
- `lcm(a, b)` returns the lowest common multiple of `a` and `b`. This implemention is mainly for
  educational purposes, as `std::lcm(a, b)` from `<numeric>` is available as of C++17.
- `extended_euclid(a, b)` returns a pair $(x, y)$ of integers such that $\gcd(a, b) = ax + by$.
- `diophantine(a, b, c, x, y, g)` solves the linear Diophantine equation $ax + by = c$. It returns
  whether a solution exists (one does if and only if $\gcd(a, b)$ divides $c$), and on success sets
  `g` to $\gcd(a, b)$ and `(x, y)` to a particular solution bounded by $\max(|a|, |b|, |c|)$ in
  magnitude. A 128-bit intermediate is used where available to keep the scaling step overflow-free.
- `mod(a, b)` returns the value of `a` mod `b` under the true Euclidean definition of modulo, that
  is, the smallest nonnegative integer $m$ satisfying $a + bn = m$ for some integer $n$. Note that
  this is identical to the remainder operator `%` in C++ for nonnegative operands `a` and `b`, but
  the result will differ when an operand is negative.
- `mod_inverse(a, m)` returns an integer $x$ such that $ax \equiv 1 \pmod m$, where the arguments
  must satisfy $m > 0$ and $\gcd(a, m) = 1$.
- `generate_inverse(p)` returns a vector $v$ of integers where for each index $i$ in the vector,
  $i \cdot `v[i]` \equiv 1 \pmod p$, where the argument $p$ is prime.
- `crt(r1, m1, r2, m2, r, m)` merges the two congruences $x \equiv r_1 \pmod{m_1}$ and
  $x \equiv r_2 \pmod{m_2}$ for arbitrary moduli (not necessarily coprime). It returns whether the
  system is consistent, and on success sets `m` to `lcm(m_1, m_2)` and `r` to the unique solution
  in $[0, m)$. Fold it pairwise to merge more than two congruences.
- `garner_restore(a, p)` returns the smallest nonnegative solution $x$ for the system of
  simultaneous congruences $x \equiv `a[i]` \pmod{`p[i]`}$ for all indices $i$ in $[0, n)$, where
  `p` consists of pairwise coprime integers (unlike `crt`, which allows shared factors). The exact
  solution is unique modulo the product of all moduli, so that product and the final answer must fit
  in `int64_t`.
- `garner_restore_mod(a, p, m)` returns that same CRT solution modulo `m`. This is the right
  variant when the product of the moduli is too large to fit in `int64_t`.

Time Complexity:
- O(log(a + b)) per call to `gcd(a, b)`, `lcm(a, b)`, `extended_euclid(a, b)`,
  `mod_inverse(a, b)`, `diophantine(a, b, c, ...)`, and `crt(...)`.
- O(1) for `mod(a, b)`.
- O(p) for `generate_inverse(p)`.
- O(n^2) for `garner_restore(a, p)` and `garner_restore_mod(a, p, m)`.

Space Complexity:
- O(p) auxiliary heap space for `generate_inverse(p)`.
- O(n) auxiliary heap space for `garner_restore(a, p)` and `garner_restore_mod(a, p, m)`.
- O(1) auxiliary space for all other operations.

*/

#include <cstdint>
#include <type_traits>
#include <utility>
#include <vector>

template<class Int>
Int gcd(Int a, Int b) {
  while (b != 0) {
    Int t = b;
    b = a % b;
    a = t;
  }
  return (a < 0 ? -a : a);
}

template<class Int>
Int lcm(Int a, Int b) {
  if (a == 0 || b == 0) {
    return 0;
  }
  Int res = a / gcd(a, b) * b;
  return (res < 0 ? -res : res);
}

template<class Int>
std::pair<Int, Int> extended_euclid(Int a, Int b) {
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

template<class Int>
bool diophantine(Int a, Int b, Int c, Int &x, Int &y, Int &g) {
  if (a == 0 && b == 0) {
    x = y = g = 0;
    return c == 0;
  }
  if (a == 0) {
    g = (b < 0) ? -b : b;
    if (c % b != 0) {
      return false;
    }
    x = 0;
    y = c / b;
    return true;
  }
  if (b == 0) {
    g = (a < 0) ? -a : a;
    if (c % a != 0) {
      return false;
    }
    x = c / a;
    y = 0;
    return true;
  }
  g = gcd(a, b);
  if (c % g != 0) {
    return false;
  }
  // Absorb the bulk of c into a*dx + b*dy, then scale the base solution by the small remainder so
  // the reported (x, y) stay bounded by max(|a|, |b|, |c|). The scaled product is taken modulo b
  // (resp. a) through a 128-bit intermediate where available, to avoid overflow.
  std::pair<Int, Int> base = extended_euclid(a, b);
  Int dx = c / a;
  c -= dx * a;
  Int dy = c / b;
  c -= dy * b;
  Int f = c / g;
#if defined(__SIZEOF_INT128__)
  __extension__ typedef std::conditional_t<sizeof(Int) <= 4, int64_t, __int128> wide_t;
#else
  using wide_t = int64_t;
#endif
  x = dx + static_cast<Int>(static_cast<wide_t>(base.first) * f % b);
  y = dy + static_cast<Int>(static_cast<wide_t>(base.second) * f % a);
  return true;
}

template<class Int>
Int mod(Int a, Int m) {
  Int r = a % m;
  return (r >= 0) ? r : (r + m);
}

template<class Int>
Int mod_inverse(Int a, Int m) {
  return mod(extended_euclid(a, m).first, m);
}

std::vector<int> generate_inverse(int p) {
  std::vector<int> res(p);
  res[1] = 1;
  for (int i = 2; i < p; i++) {
    res[i] = (p - (p / i) * res[p % i] % p) % p;
  }
  return res;
}

bool crt(int64_t r1, int64_t m1, int64_t r2, int64_t m2, int64_t &r, int64_t &m) {
  r1 = mod(r1, m1);
  r2 = mod(r2, m2);
  int64_t x, y, g;
  if (!diophantine(m1, -m2, r2 - r1, x, y, g)) {
    return false;
  }
  m = m1 / g * m2;
  r = mod(r1 + m1 * mod(x, m2 / g), m);
  return true;
}

std::vector<int64_t> garner_digits(const std::vector<int> &a, const std::vector<int> &p) {
  int n = static_cast<int>(a.size());
  std::vector<int64_t> x(a.begin(), a.end());
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
  {
    for (int a = -20; a <= 20; a++) {
      for (int b = -20; b <= 20; b++) {
        int g = gcd(a, b);
        auto [x, y] = extended_euclid(a, b);
        assert(g == a * x + b * y);
        if (g == 1 && b > 1) {
          int inv = mod_inverse(a, b);
          assert(mod(a * inv, b) == 1);
        }
      }
    }
  }
  {
    int p = 17;
    auto res = generate_inverse(p);
    for (int i = 0; i < p; i++) {
      if (i > 0) {
        assert(mod(i * res[i], p) == 1);
      }
    }
  }
  {
    vector<int> a{2, 3, 1}, m{3, 4, 5};
    int n = static_cast<int>(a.size());
    int x = garner_restore(a, m);
    assert(x == garner_restore_mod(a, m, 1000000007));
    for (int i = 0; i < n; i++) {
      assert(mod(x, m[i]) == a[i]);
    }
    assert(x == 11);
  }
#if defined(__SIZEOF_INT128__)
  {  // Garner modulo another number works even when the product of CRT moduli is too large.
    vector<int> p{1000000007, 1000000009, 1000000033};
    __extension__ typedef __int128 int128_t;
    int128_t x = (static_cast<int128_t>(1) << 80) + 123456789;
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
          int x, y, g, gg = gcd(a, b);
          bool ok = diophantine(a, b, c, x, y, g);
          assert(ok == (gg == 0 ? c == 0 : c % gg == 0));
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
            bool ok = crt(r1, m1, r2, m2, r, m);
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
