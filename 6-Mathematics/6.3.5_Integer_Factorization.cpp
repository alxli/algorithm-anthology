/*

Compute the prime factorization of an integer. In the following implementations, the prime
factorization of $n$ is represented as a sorted vector of prime integers which together multiply to
$n$. Note that factors are duplicated in the vector in accordance to their multiplicity in the prime
factorization of $n$. For 0, 1, and prime numbers, the prime factorization is considered to be a
vector consisting of a single element - the input itself.

- `prime_factorize(n)` returns the prime factorization of `n` using trial division.
- `get_divisors(n)` returns a sorted vector of all (not merely prime) divisors of `n` using trial
  division.
- `fermat(n)` returns a factor of `n` (possibly 1 or itself) that is not necessarily prime. This
  algorithm is efficient for integers with two factors near $\sqrt{n}$, but is roughly as slow as
  trial division otherwise.
- `pollards_rho_brent(n)` returns a factor of `n` that is not necessarily prime using Pollard's rho
  algorithm with Brent's optimization. If `n` is prime, then `n` itself is returned. While this
  algorithm is non-deterministic and may fail to detect factors on certain runs of the same input,
  it can be placed in a loop to deterministically factor large integers, as done in
  `prime_factorize_big()`.
- `prime_factorize_big(n, trial_division_cutoff)` returns the prime factorization of a 64-bit
  integer `n` using a combination of trial division, the Miller-Rabin primality test, and Pollard's
  rho algorithm. `trial_division_cutoff` specifies the largest factor to test with trial division
  before falling back to the rho algorithm. This supports 64-bit integers up to and including
  $2^{63} - 1$.

Time Complexity:
- O(sqrt n) per call to `prime_factorize(n)`, `get_divisors(n)`, and `fermat(n)`.
- Unknown, but approximately O(n^(1/4)) per call to `pollards_rho_brent(n)` and
  `prime_factorize_big(n)`.

Space Complexity:
- O(f) auxiliary heap space for all operations, where $f$ is the number of factors returned.

*/

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

template<class Int>
std::vector<Int> prime_factorize(Int n) {
  if (n <= 3) {
    return std::vector<Int>(1, n);
  }
  std::vector<Int> res;
  for (Int i = 2;; i++) {
    int p = 0;
    Int q = n / i, r = n - q * i;
    if (i > q || (i == q && r > 0)) {
      break;
    }
    while (r == 0) {
      p++;
      n = q;
      q = n / i;
      r = n - q * i;
    }
    for (int j = 0; j < p; j++) {
      res.push_back(i);
    }
  }
  if (n > 1) {
    res.push_back(n);
  }
  return res;
}

template<class Int>
std::vector<Int> get_divisors(Int n) {
  if (n <= 1) {
    return (n < 1) ? std::vector<Int>() : std::vector<Int>(1, 1);
  }
  std::vector<Int> res;
  for (Int i = 1; i <= n / i; i++) {
    if (n % i == 0) {
      res.push_back(i);
      if (i != n / i) {
        res.push_back(n / i);
      }
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}

long long fermat(long long n) {
  if (n % 2 == 0) {
    return 2;
  }
  long long x = sqrt(n), y = 0, r = x * x - y * y - n;
  while (r != 0) {
    if (r < 0) {
      r += x + x + 1;
      x++;
    } else {
      r -= y + y + 1;
      y++;
    }
  }
  return (x == y) ? (x + y) : (x - y);
}

using uint64 = unsigned long long;

// Hot path: Can simply `return (unsigned __int128)x * n % m;` for a major constant-factor speedup
// if the GCC/Clang __int128 extension is available. Otherwise, use the portable double-and-add
// version below, which still won't overflow a signed 64-bit `long long`.
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
  static std::mt19937_64 rng(std::random_device{}());
  return rng();
}

uint64 gcd(uint64 a, uint64 b) {
  while (b != 0) {
    uint64 t = b;
    b = a % b;
    a = t;
  }
  return a;
}

long long pollards_rho_brent(long long n) {
  if (n % 2 == 0) {
    return 2;
  }
  uint64 y = rand64u() % (n - 1) + 1;
  uint64 c = rand64u() % (n - 1) + 1;
  uint64 m = rand64u() % (n - 1) + 1;
  uint64 g = 1, r = 1, q = 1, ys = 0, x = 0;
  for (r = 1; g == 1; r <<= 1) {
    x = y;
    for (uint64 i = 0; i < r; i++) {
      y = (mulmod(y, y, n) + c) % n;
    }
    for (uint64 k = 0; k < r && g == 1; k += m) {
      ys = y;
      long long lim = std::min(m, r - k);
      for (int j = 0; j < lim; j++) {
        y = (mulmod(y, y, n) + c) % n;
        q = mulmod(q, (x > y) ? (x - y) : (y - x), n);
      }
      g = gcd(q, n);
    }
  }
  if (g == static_cast<uint64>(n)) {
    do {
      ys = (mulmod(ys, ys, n) + c) % n;
      g = gcd((x > ys) ? (x - ys) : (ys - x), n);
    } while (g <= 1);
  }
  return g;
}

bool is_prime_fast(long long n) {
  static const int np = 12, p[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37};
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

std::vector<long long> prime_factorize_big(
    long long n, long long trial_division_cutoff = 1000000LL
) {
  if (n <= 3) {
    return std::vector<long long>(1, n);
  }
  std::vector<long long> res;
  for (; n % 2 == 0; n /= 2) {
    res.push_back(2);
  }
  for (; n % 3 == 0; n /= 3) {
    res.push_back(3);
  }
  for (long long i = 5, w = 4; i <= trial_division_cutoff && i <= n / i; i += w) {
    for (; n % i == 0; n /= i) {
      res.push_back(i);
    }
    w = 6 - w;
  }
  for (long long p; n > trial_division_cutoff && !is_prime_fast(n); n /= p) {
    do {
      p = pollards_rho_brent(n);
    } while (p == n);
    res.push_back(p);
  }
  if (n != 1) {
    res.push_back(n);
  }
  sort(res.begin(), res.end());
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <set>
using namespace std;

void validate(long long n, const vector<long long> &factors) {
  if (n == 1 || is_prime_fast(n)) {
    assert(factors == vector<long long>(1, n));
    return;
  }
  long long prod = 1;
  for (long long f : factors) {
    assert(is_prime_fast(f));
    prod *= f;
  }
  assert(prod == n);
}

int main() {
  {  // Small tests.
    for (int i = 1; i <= 10000; i++) {
      auto v1 = prime_factorize(static_cast<long long>(i));
      auto v2 = prime_factorize_big(i);
      validate(i, v1);
      assert(v1 == v2);
      auto d = get_divisors(i);
      set<int> s(d.begin(), d.end());
      assert(d.size() == s.size());
      for (int j = 1; j <= i; j++) {
        if (i % j == 0) {
          assert(s.count(j));
        }
      }
    }
  }
  {  // Fermat works best for numbers with two factors close to each other.
    long long n = 1000003LL * 100000037;
    assert(fermat(n) == 1000003);
  }
  {  // Large tests.
    const vector<long long> tests{
        3LL * 3 * 5 * 7 * 9949 * 9967 * 1000003,
        2LL * 1000003 * 1000000007,
        999961LL * 1000033,
        357267896789127671LL,
        2LL * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 3 * 5 * 5 * 7 * 7 * 11 * 13 * 17 * 19 * 23 * 29 *
            31 * 37,
        2LL * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 3 * 5 * 5 * 7 * 7 * 35336848213,
        2LL * 2 * 2 * 2 * 2 * 2 * 2 * 3 * 3 * 3 * 3 * 5 * 5 * 7 * 7 * 186917 * 186947,
    };
    for (const auto t : tests) {
      validate(t, prime_factorize_big(t));
    }
  }
  return 0;
}
