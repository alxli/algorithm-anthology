/*

Given an integer $a$ and an odd prime $p$, find a modular square root: an $x$ with $x^2$ congruent
to $a$ modulo $p$. Exactly half of the nonzero residues modulo a prime are squares (quadratic
residues), so a solution exists only for those; when it does, the two roots are $x$ and $p - x$.

The Tonelli-Shanks algorithm handles the general case. Euler's criterion, $a^{(p-1)/2}$ modulo $p$,
first tests whether $a$ is a residue at all. When $p$ is congruent to 3 modulo 4 the root is simply
$a^{(p+1)/4}$. Otherwise the algorithm factors $p - 1$ as $s \cdot 2^e$, picks any quadratic
non-residue to generate the 2-power part of the group, and repeatedly squares a running value to
find the order of the current error term, correcting the candidate root until the error becomes 1.

- `mod_sqrt(a, p)` returns the smaller of the two square roots of `a` modulo the odd prime `p`, or
  $-1$ if `a` is not a quadratic residue. The value 0 maps to 0. The argument `a` is reduced modulo
  `p` internally. The modulus `p` must be an odd prime.

Time Complexity:
- O(log^2 p) per call in the worst case, from the squaring loop nested inside the exponentiations.

Space Complexity:
- O(1) auxiliary.

*/

#include <cstdint>

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

int64_t mod_sqrt(int64_t a, int64_t p) {
  a %= p;
  if (a < 0) {
    a += p;
  }
  if (a == 0) {
    return 0;
  }
  if (powmod(a, (p - 1) / 2, p) != 1) {
    return -1;  // a is a quadratic non-residue.
  }
  if (p % 4 == 3) {
    int64_t x = powmod(a, (p + 1) / 4, p);
    return x < p - x ? x : p - x;
  }
  // Write p - 1 = s * 2^e with s odd.
  int64_t s = p - 1, e = 0;
  while (s % 2 == 0) {
    s /= 2;
    e++;
  }
  // Any quadratic non-residue generates the 2-power part of the group.
  int64_t n = 2;
  while (powmod(n, (p - 1) / 2, p) != p - 1) {
    n++;
  }
  int64_t x = powmod(a, (s + 1) / 2, p);  // Candidate root, off by a 2-power factor.
  int64_t b = powmod(a, s, p);            // Error term, a power of two in order.
  int64_t g = powmod(n, s, p);            // Generator of the relevant 2-group.
  int64_t r = e;
  while (true) {
    int64_t t = b, order = 0;
    while (order < r && t != 1) {
      t = mulmod(t, t, p);
      order++;
    }
    if (order == 0) {
      return x < p - x ? x : p - x;
    }
    int64_t gs = powmod(g, static_cast<int64_t>(1) << (r - order - 1), p);
    g = mulmod(gs, gs, p);
    x = mulmod(x, gs, p);
    b = mulmod(b, g, p);
    r = order;
  }
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int64_t r = mod_sqrt(2, 113);
  assert(r != -1 && mulmod(r, r, 113) == 2);
  assert(r == 51);  // 51^2 = 2601 = 2 (mod 113); the other root is 62.

  assert(mod_sqrt(0, 7) == 0);
  assert(mod_sqrt(5, 41) != -1);  // p = 1 (mod 4) exercises the full algorithm.
  assert(mod_sqrt(4, 7) == 2);    // p = 3 (mod 4) fast path.
  assert(mod_sqrt(3, 7) == -1);   // 3 is a non-residue modulo 7.

  // Every residue's reported root squares back to it.
  for (int64_t a = 0; a < 41; a++) {
    int64_t x = mod_sqrt(a, 41);
    if (x != -1) {
      assert(mulmod(x, x, 41) == a);
    }
  }
  return 0;
}
