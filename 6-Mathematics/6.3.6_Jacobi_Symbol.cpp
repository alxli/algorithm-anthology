/*

The Jacobi symbol $(a / n)$, defined for an odd positive integer $n$, generalizes the Legendre
symbol. For an odd prime $p$ the Legendre symbol $(a / p)$ is 0 when $p$ divides $a$, 1 when $a$ is
a nonzero quadratic residue modulo $p$, and $-1$ when $a$ is a non-residue. The Jacobi symbol
extends this to composite $n$ by multiplying the Legendre symbols of the prime factors of $n$,
counted with multiplicity. It is computed without factoring $n$, using the law of quadratic
reciprocity together with the supplementary rules for $-1$ and $2$, in the style of the binary GCD.

The symbol is fully multiplicative in $a$ and in $n$. For prime $n$ it exactly decides quadratic
residuosity, so it is the fast way to test whether a modular square root exists. For composite $n$,
beware that $(a / n) = 1$ does not guarantee that $a$ is a quadratic residue; this asymmetry is what
the Solovay-Strassen primality test exploits.

- `jacobi(a, n)` returns the Jacobi symbol (`a` / `n`) as one of $-1$, 0, or 1. The modulus `n` must
  be a positive odd integer; `a` is reduced modulo `n` internally and may be negative. The result is
  0 exactly when `a` and `n` share a common factor.

Time Complexity:
- O(log a * log n) per call to `jacobi()`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cassert>
#include <cstdint>
#include <utility>

int jacobi(int64_t a, int64_t n) {
  assert(n > 0 && n % 2 == 1);
  a %= n;
  if (a < 0) {
    a += n;
  }
  int result = 1;
  while (a != 0) {
    while (a % 2 == 0) {
      a /= 2;
      // (2 / n) is -1 when n is 3 or 5 modulo 8.
      if (n % 8 == 3 || n % 8 == 5) {
        result = -result;
      }
    }
    std::swap(a, n);
    // Quadratic reciprocity flips the sign when both are 3 modulo 4.
    if (a % 4 == 3 && n % 4 == 3) {
      result = -result;
    }
    a %= n;
  }
  return n == 1 ? result : 0;
}

/*** Example Usage ***/

using namespace std;

int64_t legendre_via_euler(int64_t a, int64_t p) {
  int64_t r = 1 % p;
  for (int64_t b = ((a % p) + p) % p, e = (p - 1) / 2; e > 0; e >>= 1) {
    if (e & 1) {
      r = r * b % p;
    }
    b = b * b % p;
  }
  return r == p - 1 ? -1 : r;  // r is 0, 1, or p - 1.
}

int main() {
  assert(jacobi(2, 15) == 1);  // (2/15) = (2/3)(2/5) = (-1)(-1).
  assert(jacobi(7, 15) == -1);
  assert(jacobi(3, 15) == 0);  // gcd(3, 15) = 3.
  assert(jacobi(1001, 9907) == -1);

  // For odd primes the Jacobi symbol equals the Legendre symbol from Euler's criterion.
  for (int64_t p : {3, 5, 7, 11, 13, 101, 9973}) {
    for (int64_t a = 0; a < 50; a++) {
      assert(jacobi(a, p) == legendre_via_euler(a, p));
    }
  }
  return 0;
}
