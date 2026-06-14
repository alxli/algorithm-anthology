/*

Test whether an integer is prime and compute the prime factorization of an integer $n$. The
Miller-Rabin test writes $n - 1 = d \cdot 2^r$ and squares $a^d$ repeatedly, checking that the
sequence behaves the only way it can for a prime; a base $a$ that breaks the pattern is a witness
that $n$ is composite. For large composite inputs, Pollard's rho iterates a pseudorandom map modulo
$n$; two iterates that collide modulo a hidden prime factor reveal that factor through a GCD with
their difference. Prime factorizations are represented as sorted vectors of `(prime, exponent)`
pairs. For 0 and 1, the prime factorization is empty.

- `is_prime_slow(n)` returns whether the integer `n` is prime using trial division.
- `is_probable_prime(n, k)` returns whether `n` is probably prime using `k` random Miller-Rabin
  bases. The result is guaranteed correct when `n` is prime. When `n` is composite, it returns
  false with probability at least $1 - (1 / 4)^k$, so a true result has one-sided error probability
  at most $(1 / 4)^k$. This is mostly useful when adapting Miller-Rabin outside the known 64-bit
  deterministic setting.
- `is_prime(n)` returns whether the signed 64-bit integer `n` is prime using deterministic
  Miller-Rabin bases sufficient up to and including $2^{63} - 1$.
- `factorize_slow(n)` returns the prime factorization of `n` using trial division.
- `get_divisors_slow(n)` returns a sorted vector of all divisors of `n` using trial division.
- `rho_factor(n)` returns a factor of `n` that is not necessarily prime using Pollard's rho with
  Brent's optimization. If `n` is prime, then `n` itself is returned. While this algorithm is
  non-deterministic and may fail to detect factors on certain runs of the same input, it can be
  retried until a nontrivial factor is found, as done in `factorize()`.
- `cached_primes(n)` returns a cached vector of primes up to and including `n`, rebuilding it with a
  linear sieve if needed.
- `merge_factors(a, b)` merges two sorted compressed factorizations.
- `factorize_rho(n)` returns the prime factorization of a 64-bit integer using Miller-Rabin and
  Pollard's rho without initial trial division.
- `factorize(n, trial_division_cutoff)` returns the prime factorization of a 64-bit integer `n`
  using a combination of trial division, the Miller-Rabin primality test, and Pollard's rho 
  algorithm. `trial_division_cutoff` specifies the largest prime to test with trial division before
  falling back to the rho algorithm. This supports 64-bit integers up to and including $2^{63} - 1$.
- `divisors_from_factors(factors)` returns all divisors from a compressed factorization.
- `get_divisors(n)` returns all divisors of a 64-bit integer using `factorize()` followed by
  `divisors_from_factors()`.

Time Complexity:
- O(sqrt(n)) per call to `is_prime_slow(n)`.
- O(k log^3(n)) per call to `is_probable_prime(n, k)`.
- O(log^3(n)) per call to `is_prime(n)`.
- O(sqrt(n)) per call to `factorize_slow(n)` and `get_divisors_slow(n)`.
- O(n) to rebuild the prime cache in `cached_primes(n)`.
- Unknown, but approximately O(n^(1/4)) per call to `rho_factor(n)` and `factorize(n)`.

Space Complexity:
- O(f) auxiliary heap space for all operations, where $f$ is the number of factors returned.
- O(c) cached heap space for the largest `cached_primes(c)` call.

*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <utility>
#include <vector>

template<class Int>
bool is_prime_slow(Int n) {
  if (n == 2 || n == 3) {
    return true;
  }
  if (n < 2 || n % 2 == 0 || n % 3 == 0) {
    return false;
  }
  for (Int i = 5, w = 4; i <= n / i; i += w) {
    if (n % i == 0) {
      return false;
    }
    w = 6 - w;
  }
  return true;
}

template<class Int>
std::vector<std::pair<Int, int>> factorize_slow(Int n) {
  if (n <= 1) {
    return {};
  }
  std::vector<std::pair<Int, int>> res;
  for (Int i = 2; i <= n / i; i++) {
    if (n % i == 0) {
      int p = 0;
      do {
        n /= i;
        p++;
      } while (n % i == 0);
      res.emplace_back(i, p);
    }
  }
  if (n > 1) {
    res.emplace_back(n, 1);
  }
  return res;
}

template<class Int>
std::vector<Int> get_divisors_slow(Int n) {
  if (n <= 1) {
    return (n < 1) ? std::vector<Int>() : std::vector<Int>(1, 1);
  }
  std::vector<Int> res{1};
  for (const auto &factor : factorize_slow(n)) {
    int old_size = static_cast<int>(res.size());
    Int power = 1;
    for (int e = 1; e <= factor.second; e++) {
      power *= factor.first;
      for (int i = 0; i < old_size; i++) {
        res.push_back(res[i] * power);
      }
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}

uint64_t mulmod(uint64_t x, uint64_t n, uint64_t m) {
#if defined(__SIZEOF_INT128__)
  return static_cast<uint64_t>(static_cast<__uint128_t>(x) * n % m);
#else
  uint64_t a = 0, b = x % m;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = (a + b) % m;
    }
    b = (b << 1) % m;
  }
  return a % m;
#endif
}

uint64_t powmod(uint64_t x, uint64_t n, uint64_t m) {
  uint64_t a = 1, b = x;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = mulmod(a, b, m);
    }
    b = mulmod(b, b, m);
  }
  return a % m;
}

uint64_t rand64u() {
  static std::mt19937_64 rng(std::random_device{}());
  return rng();
}

bool is_probable_prime(int64_t n, int k = 20) {
  if (n == 2 || n == 3) {
    return true;
  }
  if (n < 2 || n % 2 == 0 || n % 3 == 0) {
    return false;
  }
  uint64_t s = n - 1, p = n - 1;
  while (!(s & 1)) {
    s >>= 1;
  }
  for (int i = 0; i < k; i++) {
    uint64_t x, r = powmod(rand64u() % (n - 3) + 2, s, n);  // random witness base in [2, n - 2]
    for (x = s; x != p && r != 1 && r != p; x <<= 1) {
      r = mulmod(r, r, n);
    }
    if (r != p && !(x & 1)) {
      return false;
    }
  }
  return true;
}

int64_t rho_factor(int64_t n) {
  if (n % 2 == 0) {
    return 2;
  }
  uint64_t y = rand64u() % (n - 1) + 1;
  uint64_t c = rand64u() % (n - 1) + 1;
  uint64_t m = rand64u() % (n - 1) + 1;
  uint64_t g = 1, r = 1, q = 1, ys = 0, x = 0;
  for (r = 1; g == 1; r <<= 1) {
    x = y;
    for (uint64_t i = 0; i < r; i++) {
      y = (mulmod(y, y, n) + c) % n;
    }
    for (uint64_t k = 0; k < r && g == 1; k += m) {
      ys = y;
      int64_t lim = std::min(m, r - k);
      for (int j = 0; j < lim; j++) {
        y = (mulmod(y, y, n) + c) % n;
        q = mulmod(q, (x > y) ? (x - y) : (y - x), n);
      }
      g = std::gcd(q, n);
    }
  }
  if (g == static_cast<uint64_t>(n)) {
    do {
      ys = (mulmod(ys, ys, n) + c) % n;
      g = std::gcd((x > ys) ? (x - ys) : (ys - x), n);
    } while (g <= 1);
  }
  return g;
}

bool is_prime(int64_t n) {
  if (n < 2) {
    return false;
  }
  static const int small_primes[] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
  for (int p : small_primes) {
    if (n % p == 0) {
      return n == p;
    }
  }
  if (n < 31 * 31) {
    return true;
  }
  uint64_t t;
  int s = 0;
  for (t = n - 1; !(t & 1); t >>= 1) {
    s++;
  }
  static const uint64_t bases[] = {2, 325, 9375, 28178, 450775, 9780504, 1795265022};
  for (uint64_t a : bases) {
    if (a % n == 0) {
      continue;
    }
    uint64_t r = powmod(a, t, n);
    if (r == 1) {
      continue;
    }
    bool ok = false;
    for (int j = 0; j < s && !ok; j++) {
      ok |= (r == static_cast<uint64_t>(n) - 1);
      r = mulmod(r, r, n);
    }
    if (!ok) {
      return false;
    }
  }
  return true;
}

std::vector<int> cached_primes(int n) {
  static int limit = 1;
  static std::vector<int> primes;
  if (n <= limit) {
    return primes;
  }
  std::vector<int> least(n + 1, 0);
  primes.clear();
  for (int i = 2; i <= n; i++) {
    if (least[i] == 0) {
      least[i] = i;
      primes.push_back(i);
    }
    for (int p : primes) {
      if (p > least[i] || i > n / p) {
        break;
      }
      least[i * p] = p;
    }
  }
  limit = n;
  return primes;
}

using vfactors = std::vector<std::pair<int64_t, int>>;

vfactors merge_factors(const vfactors &a, const vfactors &b) {
  vfactors res;
  int i = 0, j = 0;
  while (i < static_cast<int>(a.size()) || j < static_cast<int>(b.size())) {
    if (i < static_cast<int>(a.size()) && j < static_cast<int>(b.size()) &&
        a[i].first == b[j].first) {
      res.emplace_back(a[i].first, a[i].second + b[j].second);
      i++;
      j++;
    } else if (
        j == static_cast<int>(b.size()) ||
        (i < static_cast<int>(a.size()) && a[i].first < b[j].first)
    ) {
      res.push_back(a[i++]);
    } else {
      res.push_back(b[j++]);
    }
  }
  return res;
}

vfactors factorize_rho(int64_t n) {
  if (n <= 1) {
    return {};
  }
  if (n % 2 == 0) {
    return merge_factors(vfactors{{2, 1}}, factorize_rho(n / 2));
  }
  if (is_prime(n)) {
    return {{n, 1}};
  }
  int64_t p;
  do {
    p = rho_factor(n);
  } while (p == n);
  return merge_factors(factorize_rho(p), factorize_rho(n / p));
}

vfactors factorize(int64_t n, int trial_division_cutoff = 1000000) {
  if (n <= 1) {
    return {};
  }
  vfactors res;
  auto primes = cached_primes(trial_division_cutoff);
  for (int p : primes) {
    if (p > trial_division_cutoff || static_cast<int64_t>(p) > n / p) {
      break;
    }
    if (n % p == 0) {
      int cnt = 0;
      do {
        n /= p;
        cnt++;
      } while (n % p == 0);
      res.emplace_back(p, cnt);
    }
  }
  return merge_factors(res, factorize_rho(n));
}

std::vector<int64_t> divisors_from_factors(const vfactors &factors) {
  std::vector<int64_t> res{1};
  for (const auto &factor : factors) {
    int old_size = static_cast<int>(res.size());
    int64_t power = 1;
    for (int e = 1; e <= factor.second; e++) {
      power *= factor.first;
      for (int i = 0; i < old_size; i++) {
        res.push_back(res[i] * power);
      }
    }
  }
  std::sort(res.begin(), res.end());
  return res;
}

std::vector<int64_t> get_divisors(int64_t n) {
  if (n <= 1) {
    return (n < 1) ? std::vector<int64_t>() : std::vector<int64_t>(1, 1);
  }
  return divisors_from_factors(factorize(n));
}

/*** Example Usage and Output:

Primality test:
num:           is_prime_slow():  is_probable_prime():  is_prime():
772023803            0.008ms           0.006ms           0.003ms
2147483647           0.009ms           0.006ms           0.002ms
5705234089           0.013ms           0.000ms           0.000ms
6339503641           0.013ms           0.001ms           0.001ms
999966000289         0.162ms           0.001ms           0.000ms

Factorization test:
num:                    factorize_slow():   factorize():
4611686018427387904          0.002ms           0.006ms
9223372036854775807          0.044ms           0.010ms
999966000289                 0.469ms           0.043ms
9223361212852495307          0.902ms           0.105ms
2500052700017629            24.283ms           0.123ms

***/

#include <cassert>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <set>
using namespace std;

void validate(int64_t n, const vfactors &factors) {
  if (n <= 1) {
    assert(factors.empty());
    return;
  }
  if (is_prime(n)) {
    assert((factors == vfactors{{n, 1}}));
    return;
  }
  int64_t prod = 1;
  for (const auto &factor : factors) {
    assert(is_prime(factor.first));
    for (int i = 0; i < factor.second; i++) {
      prod *= factor.first;
    }
  }
  assert(prod == n);
}

int main() {
  {  // Small primality tests.
    // clang-format off
    vector<pair<int, bool>> cases{
        {-10, false}, {-1, false}, {0, false}, {1, false}, // non-primes
        {2, true}, {3, true}, {4, false}, {5, true},
        {9, false},    // divisible by 3
        {25, false},   // square of small prime
        {29, true},    // last small-prime table entry
        {31, true},    // just past small-prime table
        {49, false},   // square, not divisible by 2 or 3
        {121, false},  // 11^2
        {169, false},  // 13^2
        {961, false},  // 31^2, boundary for n < 31*31 shortcut
        {997, true},   // prime near 1000
        {1001, false}, // 7*11*13
        {2047, false}, // 23*89, pseudoprime to base 2
        {341, false},  // 11*31, Fermat pseudoprime to base 2
        {561, false},  // Carmichael number
    };
    // clang-format on
    for (const auto [n, expected] : cases) {
      assert(is_prime_slow(n) == expected);
      assert(is_probable_prime(n) == expected);
      assert(is_prime(n) == expected);
    }
  }
  {  // Primality test benchmark.
    vector<int64_t> nums{
        772023803LL, 2147483647LL, 5705234089LL, 6339503641LL, 999966000289LL,
    };
    cout << "Primality test:" << endl;
    cout << setw(15) << left << "num:";
    cout << setw(18) << left << "is_prime_slow():";
    cout << setw(20) << left << "is_probable_prime():";
    cout << setw(13) << right << "is_prime():" << endl;
    cout << fixed << setprecision(3);
    for (int64_t n : nums) {
      clock_t start = clock();
      bool p1 = is_prime_slow(n);
      double t1 = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
      start = clock();
      bool p2 = is_probable_prime(n);
      double t2 = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
      start = clock();
      bool p3 = is_prime(n);
      double t3 = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
      assert(p1 == p2 && p1 == p3);
      cout << setw(12) << left << n << right;
      cout << setw(14) << 1000 * t1 << "ms";
      cout << setw(16) << 1000 * t2 << "ms";
      cout << setw(16) << 1000 * t3 << "ms" << endl;
    }
    cout << endl;
  }
  {  // Small factorization tests.
    for (int64_t i = 1; i <= 10000; i++) {
      auto v1 = factorize_slow(i);
      auto v2 = factorize(i);
      validate(i, v1);
      assert(v1 == v2);
      auto d = get_divisors_slow(i);
      set<int> s(d.begin(), d.end());
      assert(d.size() == s.size());
      for (int j = 1; j <= i; j++) {
        if (i % j == 0) {
          assert(s.count(j));
        }
      }
    }
  }
  for (int64_t i : {1, 2, 36, 9999, 10000}) {
    auto d = get_divisors_slow(i);
    assert(std::vector<int64_t>(d.begin(), d.end()) == get_divisors(i));
  }
  {  // Compressed factors are convenient for building divisors.
    vfactors factors{{2, 3}, {3, 2}, {5, 1}};
    vector<int64_t> divisors = divisors_from_factors(factors);
    assert(divisors.size() == 24);
    assert(divisors.front() == 1 && divisors.back() == 360);
    assert(factorize(360) == factors);
    assert(get_divisors(360) == get_divisors_slow<int64_t>(360));
  }
  {  // Large factorization tests.
    const vector<int64_t> nums{
        (1LL << 62),                    // high power of 2
        9223372036854775807LL,          // 2^63 - 1, many small factors
        999983LL * 999983,              // square of a large prime
        1900009LL * 1910009 * 2541547,  // three medium-size prime factors
        50000017LL * 50001037,          // 16-digit semiprime
    };
    cout << "Factorization test:" << endl;
    cout << setw(24) << left << "num:";
    cout << setw(20) << left << "factorize_slow():";
    cout << "factorize():" << endl;
    cout << fixed << setprecision(3);
    for (int64_t n : nums) {
      clock_t start = clock();
      vfactors factors1 = factorize_slow(n);
      double t1 = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
      start = clock();
      vfactors factors2 = factorize(n);
      double t2 = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
      validate(n, factors1);
      validate(n, factors2);
      assert(factors1 == factors2);
      cout << setw(26) << left << n << right << setw(8) << 1000 * t1 << "ms";
      cout << setw(16) << 1000 * t2 << "ms" << endl;
      cout << left;
    }
  }
  return 0;
}
