/*

Generate prime numbers using the Sieve of Eratosthenes, which repeatedly marks the multiples of each
prime as composite.

The range overload `sieve(lo, hi)` uses a segmented sieve, letting primes in a high but narrow
window be found without sieving everything below it. Since every composite at most `hi` has a prime
factor at most $\sqrt{`hi`}$, it first sieves the primes up to $\sqrt{`hi`}$, then uses only those
primes to mark composites within $[`lo`, `hi`]$ (each prime $p$ starting at the first of its
multiples that is at least both $p^2$ and `lo`). This needs only O(w + sqrt(h)) space, where
$w = `hi` - `lo` + 1$ and $h = `hi`$, instead of the O(h) space that sieving all of $[2, `hi`]$
would require.

The linear sieve finds the least prime factor of each integer. When processing `i`, it marks `i * p`
for each known prime `p` no greater than `least[i]`. Every composite `x` is therefore generated
exactly once: writing `x = i * p`, where `p` is its least prime factor, gives `p` $\leq$ `least[i]`,
while any representation using a larger prime is skipped. This single visit per composite gives the
O(n) running time.

- `sieve(n)` returns a vector of all the primes less than or equal to `n`.
- `sieve(lo, hi)` returns a vector of all the primes in the range $[`lo`, `hi`]$.
- `linear_sieve(n, &least_out)` returns a vector of all primes less than or equal to `n` in linear
  time. If `least_out` is not null, it is filled so that `(*least_out)[x]` is the least prime factor
  of `x` for every `x` $\geq 2$.

Time Complexity:
- O(n log log n) per call to `sieve(n)`.
- O((w + sqrt(`hi`))*log(log(`hi`))) per call to `sieve(lo, hi)`, where $w = `hi` - `lo` + 1$.
- O(n) per call to `linear_sieve()`.

Space Complexity:
- O(n) auxiliary for `sieve(n)`.
- O(w + sqrt(`hi`)) auxiliary for `sieve(lo, hi)`.
- O(n) auxiliary for `linear_sieve()`.

*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

std::vector<int> sieve(int n) {
  if (n < 2) {
    return {};
  }
  std::vector<char> prime(n + 1, true);
  for (int i = 2; i <= n / i; i++) {
    if (prime[i]) {
      for (int64_t j = 1LL * i * i; j <= n; j += i) {
        prime[j] = false;
      }
    }
  }
  std::vector<int> res;
  for (int i = 2; i <= n; i++) {
    if (prime[i]) {
      res.push_back(i);
    }
  }
  return res;
}

std::vector<int> sieve(int lo, int hi) {
  if (hi < 2 || lo > hi) {
    return {};
  }
  int sqrt_hi = sqrt(hi);
  while (1LL * (sqrt_hi + 1) * (sqrt_hi + 1) <= hi) {
    sqrt_hi++;
  }
  int fourth_root_hi = sqrt(sqrt_hi);
  while (1LL * (fourth_root_hi + 1) * (fourth_root_hi + 1) <= sqrt_hi) {
    fourth_root_hi++;
  }
  std::vector<char> prime1(sqrt_hi + 1, true), prime2(hi - lo + 1, true);
  for (int i = 2; i <= fourth_root_hi; i++) {
    if (prime1[i]) {
      for (int64_t j = 1LL * i * i; j <= sqrt_hi; j += i) {
        prime1[j] = false;
      }
    }
  }
  for (int i = 2; i <= sqrt_hi; i++) {
    if (prime1[i]) {
      int64_t first_multiple = ((static_cast<int64_t>(lo) + i - 1) / i) * i;
      int64_t start = std::max(static_cast<int64_t>(i) * i, first_multiple);
      for (int64_t j = start; j <= hi; j += i) {
        prime2[j - lo] = false;
      }
    }
  }
  std::vector<int> res;
  for (int i = (lo > 1) ? lo : 2; i <= hi; i++) {
    if (prime2[i - lo]) {
      res.push_back(i);
    }
  }
  return res;
}

std::vector<int> linear_sieve(int n, std::vector<int> *least_out = nullptr) {
  if (n < 2) {
    if (least_out != nullptr) {
      least_out->assign(std::max(n + 1, 0), 0);
    }
    return {};
  }
  std::vector<int> least(n + 1, 0), primes;
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
  if (least_out != nullptr) {
    *least_out = least;
  }
  return primes;
}

/*** Example Usage and Output:

sieve(n=10000000): 0.042641s
sieve([1000000000, 1005000000]): 0.01969s

***/

#include <cassert>
#include <ctime>
#include <iostream>
using namespace std;

int main() {
  int pmax = 10000000;
  time_t start;
  double delta;

  start = clock();
  auto p = sieve(pmax);
  delta = static_cast<double>((clock() - start)) / CLOCKS_PER_SEC;
  cout << "sieve(n=" << pmax << "): " << delta << "s" << endl;
  assert((sieve(1) == vector<int>{}));
  assert((sieve(10) == vector<int>{2, 3, 5, 7}));

  vector<int> least;
  assert(linear_sieve(pmax, &least) == p);
  assert(least[999983] == 999983);
  assert(least[999984] == 2);

  int lo = 1000000000, hi = 1005000000;
  start = clock();
  p = sieve(lo, hi);
  delta = static_cast<double>((clock() - start)) / CLOCKS_PER_SEC;
  cout << "sieve([" << lo << ", " << hi << "]): " << delta << "s" << endl;
  assert((sieve(14, 16) == vector<int>{}));
  assert((sieve(17, 19) == vector<int>{17, 19}));
  assert(!p.empty() && p.front() >= lo && p.back() <= hi);
  return 0;
}
