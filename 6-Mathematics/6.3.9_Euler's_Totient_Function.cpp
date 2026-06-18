/*

Euler's totient function $\phi(n)$ returns the number of positive integers less than or equal to $n$
that are relatively prime to $n$; that is, the number of integers $k$ in the range $[1, n]$ for
which $\gcd(n, k) = 1$.

The function is multiplicative, with $\phi(p^k) = p^k - p^{k-1}$ for a prime power, which yields the
product formula $\phi(n) = n \prod_{p \mid n} (1 - 1/p)$ taken over the distinct primes $p$ dividing
$n$. `phi(n)` applies this formula directly, factoring `n` by trial division. `phi_table(n)`
computes the whole range at once with a sieve of Eratosthenes: starting from `v[i] = i`, it sweeps
each prime $p$ across its multiples `j` and folds in the factor $(1 - 1/p)$ by replacing `v[j]` with
`v[j] - v[j] / p`. This is faster than the alternative that uses the divisor-sum identity
$\sum_{d \mid n} \phi(d) = n$ to subtract each `v[i]` from its multiples, which costs O(n log n).

The overload `phi_table(lo, hi)` computes the totients of a high but narrow range with a segmented
sieve, analogous to the segmented prime sieve. It sieves the primes up to $\sqrt{`hi`}$, applies
each one's $(1 - 1/p)$ factor to its multiples within [`lo`, `hi`] while dividing that prime out of
a parallel copy of every value, and finally folds in any single leftover prime factor greater than
$\sqrt{`hi`}$ (a number at most `hi` can have at most one such factor).

- `phi(n)` returns Euler's totient of `n`.
- `phi_table(n)` returns a vector `v` of length `n + 1` such that `v[i]` stores `phi(i)` for every
  `i` in the range $[0, n]$.
- `phi_table(lo, hi)` returns a vector `v` of length $`hi` - `lo` + 1$ such that `v[i - lo]` stores
  `phi(i)` for every `i` in the range [`lo`, `hi`]. This overload assumes `lo` $\geq$ 0; if `hi` <
  `lo`, it returns an empty vector.

Time Complexity:
- O(sqrt n) per call to `phi(n)`.
- O(n log log n) per call to `phi_table(n)`.
- O((`hi` - `lo`)*log(log(`hi`)) + sqrt(`hi`)) per call to `phi_table(lo, hi)`.

Space Complexity:
- O(1) auxiliary space for `phi(n)`.
- O(n) auxiliary heap space for `phi_table(n)`.
- O(`hi` - `lo` + sqrt(`hi`)) auxiliary heap space for `phi_table(lo, hi)`.

*/

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <vector>

int phi(int n) {
  int res = n;
  for (int i = 2; i <= n / i; i++) {
    if (n % i == 0) {
      while (n % i == 0) {
        n /= i;
      }
      res -= res / i;
    }
  }
  if (n > 1) {
    res -= res / n;
  }
  return res;
}

std::vector<int> phi_table(int n) {
  std::vector<int> res(n + 1);
  std::iota(res.begin(), res.end(), 0);
  for (int i = 2; i <= n; i++) {
    if (res[i] == i) {  // i is prime, so apply its (1 - 1/i) factor to every multiple.
      for (int j = i; j <= n; j += i) {
        res[j] -= res[j] / i;
      }
    }
  }
  return res;
}

std::vector<int> phi_table(int lo, int hi) {
  if (hi < lo) {
    return {};
  }
  int root = static_cast<int>(sqrt(hi));
  while (1LL * (root + 1) * (root + 1) <= hi) {
    root++;
  }
  // Sieve the primes up to sqrt(hi); these are the only ones that can divide a value in [lo, hi].
  std::vector<bool> composite(root + 1, false);
  std::vector<int> primes;
  for (int i = 2; i <= root; i++) {
    if (!composite[i]) {
      primes.push_back(i);
      for (int64_t j = 1LL * i * i; j <= root; j += i) {
        composite[j] = true;
      }
    }
  }
  std::vector<int> res(hi - lo + 1);
  std::vector<int64_t> remaining(hi - lo + 1);
  for (int i = lo; i <= hi; i++) {
    res[i - lo] = i;
    remaining[i - lo] = i;
  }
  for (int p : primes) {
    // Start at the first multiple of p that is >= lo, but never below p itself (this skips 0).
    int64_t start = std::max(1LL * p, ((1LL * lo + p - 1) / p) * p);
    for (int64_t j = start; j <= hi; j += p) {
      int idx = static_cast<int>(j - lo);
      res[idx] -= res[idx] / p;
      while (remaining[idx] % p == 0) {
        remaining[idx] /= p;
      }
    }
  }
  for (int i = lo; i <= hi; i++) {
    int idx = i - lo;
    if (remaining[idx] > 1) {  // a single prime factor greater than sqrt(hi) is left over
      res[idx] -= static_cast<int>(res[idx] / remaining[idx]);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(phi(1) == 1);
  assert(phi(9) == 6);
  assert(phi(1234567) == 1224720);
  const int n = 1000;
  vector<int> pt = phi_table(n);
  for (int i = 0; i <= n; i++) {
    assert(pt[i] == phi(i));
  }
  // The segmented overload agrees with phi(), including over a low range (with 0 and 1) and a
  // high, narrow window where many values have a prime factor exceeding sqrt(hi).
  vector<int> lphi = phi_table(0, 50);
  for (int i = 0; i <= 50; i++) {
    assert(lphi[i] == phi(i));
  }
  int lo = 1000000000, hi = 1000000100;
  vector<int> hphi = phi_table(lo, hi);
  for (int i = lo; i <= hi; i++) {
    assert(hphi[i - lo] == phi(i));
  }
  return 0;
}
