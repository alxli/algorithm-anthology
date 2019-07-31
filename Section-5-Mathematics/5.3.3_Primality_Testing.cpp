/*

Determine whether an integer n is prime. This can be done deterministically by
testing all numbers under sqrt(n) using trial division, probabilistically using
the Miller-Rabin test, or deterministically using the Miller-Rabin test if the
maximum input is known (2^63 - 1 for the purposes here).

- is_prime(n) returns whether the integer n is prime using an optimized trial
  division technique based on the fact that all primes greater than 6 must take
  the form 6n + 1 or 6n - 1.
- is_probable_prime(n, k) returns true if the integer n is prime, or false with
  an error probability of (1/4)^k if n is composite. In other words, the result
  is guaranteed to be correct if n is prime, but could be wrong with probability
  (1/4)^k if n is composite. This implementation uses uses exponentiation by
  squaring to support all signed 64-bit integers (up to and including 2^63 - 1).
- is_prime_fast(n) returns whether the signed 64-bit integer n is prime using
  a fully deterministic version of the Miller-Rabin test.

Time Complexity:
- O(sqrt n) per call to is_prime(n).
- O(k log^3(n)) per call to is_probable_prime(n, k).
- O(log^3(n)) per call to is_prime_fast(n).

Space Complexity:
- O(1) auxiliary space for all operations.

*/

#include <cstdlib>

template<class Int>
bool is_prime(Int n) {
  if (n == 2 || n == 3) {
    return true;
  }
  if (n < 2 || n % 2 == 0 || n % 3 == 0) {
    return false;
  }
  for (Int i = 5, w = 4; i*i <= n; i += w) {
    if (n % i == 0) {
      return false;
    }
    w = 6 - w;
  }
  return true;
}

typedef unsigned long long uint64;

uint64 mulmod(uint64 x, uint64 n, uint64 m) {
  uint64 a = 0, b = x % m;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = (a + b) % m;
    }
    b = (b << 1) % m;
  }
  return a % m;
}

uint64 powmod(uint64 x, uint64 n, uint64 m) {
  uint64 a = 1, b = x;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = mulmod(a, b, m);
    }
    b = mulmod(b, b, m);
  }
  return a % m;
}

uint64 rand64u() {
  return ((uint64)(rand() & 0xf) << 60) |
         ((uint64)(rand() & 0x7fff) << 45) |
         ((uint64)(rand() & 0x7fff) << 30) |
         ((uint64)(rand() & 0x7fff) << 15) |
         ((uint64)(rand() & 0x7fff));
}

bool is_probable_prime(long long n, int k = 20) {
  if (n == 2 || n == 3) {
    return true;
  }
  if (n < 2 || n % 2 == 0 || n % 3 == 0) {
    return false;
  }
  uint64 s = n - 1, p = n - 1;
  while (!(s & 1)) {
    s >>= 1;
  }
  for (int i = 0; i < k; i++) {
    uint64 x, r = powmod(rand64u() % p + 1, s, n);
    for (x = s; x != p && r != 1 && r != p; x <<= 1) {
      r = mulmod(r, r, n);
    }
    if (r != p && !(x & 1)) {
      return false;
    }
  }
  return true;
}

bool is_prime_fast(long long n) {
  static const int np = 9, p[] = {2, 3, 5, 7, 11, 13, 17, 19, 23};
  for (int i = 0; i < np; i++) {
    if (n % p[i] == 0) {
      return n == p[i];
    }
  }
  if (n < p[np - 1]) {
    return false;
  }
  uint64 t;
  int s = 0;
  for (t = n - 1; !(t & 1); t >>= 1) {
    s++;
  }
  for (int i = 0; i < np; i++) {
    uint64 r = powmod(p[i], t, n);
    if (r == 1) {
      continue;
    }
    bool ok = false;
    for (int j = 0; j < s && !ok; j++) {
      ok |= (r == (uint64)n - 1);
      r = mulmod(r, r, n);
    }
    if (!ok) {
      return false;
    }
  }
  return true;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  int len = 20;
  long long tests[] = {
    -1, 0, 1, 2, 3, 4, 5, 1000000LL, 772023803LL, 792904103LL, 813815117LL,
    834753187LL, 855718739LL, 876717799LL, 897746119LL, 2147483647LL,
    5705234089LL, 5914686649LL, 6114145249LL, 6339503641LL, 6548531929LL
  };
  for (int i = 0; i < len; i++) {
    bool p = is_prime(tests[i]);
    assert(p == is_prime_fast(tests[i]));
    assert(p == is_probable_prime(tests[i]));
  }
  return 0;
}
