/*

Primality Testing

*/

#include <cstdlib>  /* rand(), srand() */
#include <ctime>    /* time() */
#include <stdint.h> /* uint64_t */

/*

Trial division in O(sqrt(n)) to return whether n is prime
Applies an optimization based on the fact that all
primes greater than 3 take the form 6n + 1 or 6n - 1.

*/

template<class Int> bool is_prime(Int n) {
  if (n == 2 || n == 3) return true;
  if (n < 2 || !(n % 2) || !(n % 3)) return false;
  for (Int i = 5, w = 4; i * i <= n; i += (w = 6 - w))
    if (n % i == 0) return false;
  return true;
}

/*

Miller-Rabin Primality Test (Probabilistic)

Checks whether a number n is probably prime. If n is prime,
the function is guaranteed to return 1. If n is composite,
the function returns 1 with a probability of (1/4)^k,
where k is the number of iterations. With k = 1, the
probability of a composite being falsely predicted to be a
prime is 25%. If k = 5, the probability for this error is
just less than 0.1%. Thus, k = 18 to 20 is accurate enough
for most applications. All values of n < 2^63 is supported.

Complexity: O(k log^3(n)). In comparison to trial division,
the Miller-Rabin algorithm on 32-bit ints take ~45
operations for k = 10 iterations (~0.0001% error), while the
former takes ~10,000.

Warning: Due to the overflow of modular exponentiation,
         this will only work on inputs less than 2^63.

*/

uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
  uint64_t x = 0, y = a % m;
  for (; b > 0; b >>= 1) {
    if (b & 1) x = (x + y) % m;
    y = (y << 1) % m;
  }
  return x % m;
}

uint64_t powmod(uint64_t a, uint64_t b, uint64_t m) {
  uint64_t x = 1, y = a;
  for (; b > 0; b >>= 1) {
    if (b & 1) x = mulmod(x, y, m);
    y = mulmod(y, y, m);
  }
  return x % m;
}

//5 calls to rand() is unnecessary if RAND_MAX is 2^31-1
uint64_t rand64u() {
  return ((uint64_t)(rand() & 0xf) << 60) |
         ((uint64_t)(rand() & 0x7fff) << 45) |
         ((uint64_t)(rand() & 0x7fff) << 30) |
         ((uint64_t)(rand() & 0x7fff) << 15) |
         ((uint64_t)(rand() & 0x7fff));
}

bool is_probable_prime(long long n, int k = 20) {
  if (n < 2 || (n != 2 && !(n & 1))) return false;
  uint64_t s = n - 1, p = n - 1, x, r;
  while (!(s & 1)) s >>= 1;
  for (int i = 0; i < k; i++) {
    r = powmod(rand64u() % p + 1, s, n);
    for (x = s; x != p && r != 1 && r != p; x <<= 1)
      r = mulmod(r, r, n);
    if (r != p && !(x & 1)) return false;
  }
  return true;
}

/*

Miller-Rabin - Deterministic for all unsigned long long

Although Miller-Rabin is generally probabilistic, the seven
bases 2, 325, 9375, 28178, 450775, 9780504, 1795265022 have
been proven to deterministically test the primality of all
numbers under 2^64. See: http://miller-rabin.appspot.com/

Complexity: O(log^3(n)).
Warning: Due to the overflow of modular exponentiation,
         this will only work on inputs less than 2^63.

*/

bool is_prime_fast(long long n) {
  static const uint64_t witnesses[] =
    {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
  if (n <= 1) return false;
  if (n <= 3) return true;
  if ((n & 1) == 0) return false;
  uint64_t d = n - 1;
  int s = 0;
  for (; ~d & 1; s++) d >>= 1;
  for (int i = 0; i < 7; i++) {
    if (witnesses[i] > (uint64_t)n - 2) break;
    uint64_t x = powmod(witnesses[i], d, n);
    if (x == 1 || x == (uint64_t)n - 1) continue;
    bool flag = false;
    for (int j = 0; j < s; j++) {
      x = powmod(x, 2, n);
      if (x == 1) return false;
      if (x == (uint64_t)n - 1) {
        flag = true;
        break;
      }
    }
    if (!flag) return false;
  }
  return true;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  int len = 20;
  unsigned long long v[] = {
    0, 1, 2, 3, 4, 5, 11,
    1000000ull,
    772023803ull,
    792904103ull,
    813815117ull,
    834753187ull,
    855718739ull,
    876717799ull,
    897746119ull,
    2147483647ull,
    5705234089ull,
    5914686649ull,
    6114145249ull,
    6339503641ull,
    6548531929ull
  };
  for (int i = 0; i < len; i++) {
    bool p = is_prime(v[i]);
    assert(p == is_prime_fast(v[i]));
    assert(p == is_probable_prime(v[i]));
  }
  return 0;
}
