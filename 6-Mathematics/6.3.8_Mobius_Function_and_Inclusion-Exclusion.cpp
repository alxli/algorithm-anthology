/*

The Mobius function `mu(n)` is the multiplicative function defined as 1 if $n = 1$, 0 if $n$ is
divisible by the square of a prime, and $(-1)^k$ if $n$ is a product of $k$ distinct primes. It is
the coefficient that appears in Mobius inversion: if $g(n) = \sum_{d \mid n} f(d)$ for all $n$, then
$f(n) = \sum_{d \mid n} \mu(d) \, g(n / d)$. This formalizes the principle of inclusion-exclusion in
the divisor lattice, letting one recover a summand function from its divisor sums.

Inclusion-exclusion itself counts the size of a union by alternately adding and subtracting the
sizes of intersections. A canonical number-theoretic instance is counting integers in $[1, n]$ that
are coprime to a modulus $m$: subtract the multiples of each prime factor of $m$, add back those
counted twice, and so on. The signed terms are precisely the values of `mu` over the squarefree
divisors of $m$, so `count_coprime(n, m)` equals $\sum_{d \mid m} \mu(d) \lfloor n / d \rfloor$.

- `mobius(n)` returns `mu(n)` for a single $n$ by trial division.
- `mobius_sieve(n)` returns a vector `v` of length $n + 1$ where `v[i]` is `mu(i)`, computed together
  with a linear sieve.
- `count_coprime(n, m)` returns the number of integers in $[1, n]$ that are coprime to $m$, via
  inclusion-exclusion over the distinct prime factors of $m$.

Time Complexity:
- O(sqrt(n)) per call to `mobius(n)`.
- O(n) per call to `mobius_sieve(n)`.
- O(sqrt(m) + 2^k * k) per call to `count_coprime(n, m)`, where $k$ is the number of distinct prime
  factors of $m$ (at most 15 for $m$ within 64-bit range).

Space Complexity:
- O(n) auxiliary heap space for `mobius_sieve(n)`.
- O(k) auxiliary heap space for `count_coprime(n, m)`.
- O(1) auxiliary for `mobius(n)`.

*/

#include <vector>

int mobius(int n) {
  if (n == 1) {
    return 1;
  }
  int res = 1;
  for (int p = 2; p <= n / p; p++) {
    if (n % p == 0) {
      n /= p;
      if (n % p == 0) {
        return 0;  // p^2 divides the original n.
      }
      res = -res;
    }
  }
  if (n > 1) {
    res = -res;  // One remaining prime factor larger than its square root.
  }
  return res;
}

std::vector<int> mobius_sieve(int n) {
  std::vector<int> mu(n + 1, 0), primes;
  std::vector<bool> composite(n + 1, false);
  if (n >= 1) {
    mu[1] = 1;
  }
  for (int i = 2; i <= n; i++) {
    if (!composite[i]) {
      primes.push_back(i);
      mu[i] = -1;
    }
    for (int p : primes) {
      if (p > n / i) {
        break;
      }
      composite[i * p] = true;
      if (i % p == 0) {
        mu[i * p] = 0;  // p^2 divides i * p.
        break;
      }
      mu[i * p] = -mu[i];
    }
  }
  return mu;
}

long long count_coprime(long long n, long long m) {
  std::vector<long long> primes;
  for (long long p = 2; p <= m / p; p++) {
    if (m % p == 0) {
      primes.push_back(p);
      while (m % p == 0) {
        m /= p;
      }
    }
  }
  if (m > 1) {
    primes.push_back(m);
  }
  int k = primes.size();
  long long total = 0;
  for (int mask = 0; mask < (1 << k); mask++) {
    long long product = 1;
    int bits = 0;
    for (int i = 0; i < k; i++) {
      if ((mask >> i) & 1) {
        product *= primes[i];
        bits++;
      }
    }
    total += (bits & 1 ? -1 : 1) * (n / product);
  }
  return total;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(mobius(1) == 1);
  assert(mobius(2) == -1);
  assert(mobius(6) == 1);    // 2 * 3, two distinct primes.
  assert(mobius(12) == 0);   // Divisible by 2^2.
  assert(mobius(30) == -1);  // 2 * 3 * 5, three distinct primes.

  vector<int> mu = mobius_sieve(12);
  assert((mu == vector<int>{0, 1, -1, -1, 0, -1, 1, -1, 0, 0, 1, -1, 0}));

  assert(count_coprime(10, 6) == 3);  // 1, 5, 7
  assert(count_coprime(100, 1) == 100);
  return 0;
}
