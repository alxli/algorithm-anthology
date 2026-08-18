/*

The divisor-counting function $d(n)$ counts the positive divisors of $n$, and the divisor-sum
function $\sigma(n)$ adds them up. Both are multiplicative, so each is determined by the prime
factorization $n = \prod p_i^{e_i}$: choosing an exponent in $[0, e_i]$ for every prime gives
$d(n) = \prod (e_i + 1)$, and summing the resulting geometric series for each prime gives
$\sigma(n) = \prod (1 + p_i + \dots + p_i^{e_i})$. A single value is therefore computed by trial
division, factoring $n$ as the divisors are accumulated.

- `divisor_count(n)` returns $d(n)$, the number of positive divisors of `n`, which must be positive.
- `divisor_sum(n)` returns $\sigma(n)$, the sum of the positive divisors of `n`, which must be
  positive.

For every value in a range, factoring each number separately is wasteful. Instead, sweep over each
candidate divisor $d$ and add its contribution to every multiple of $d$. The total work is the
harmonic sum $n/1 + n/2 + \dots + n/n$, which is O(n log n). The same sweep computes any
divisor-indexed accumulation, such as the divisor sums used by the Mobius inversion of section
6.3.12.

- `divisor_count_table(n)` returns a vector `v` of length `n + 1` such that `v[i]` is the number of
  divisors of `i` for every `i` in $[1, `n`]$, with `v[0]` left as $0$.
- `divisor_sum_table(n)` returns a vector `v` of length `n + 1` such that `v[i]` is the sum of the
  divisors of `i` for every `i` in $[1, `n`]$, with `v[0]` left as $0$.

The aliquot sum $\sigma(n) - n$ adds the proper divisors, marking `n` perfect when it equals `n` and
pairing amicable numbers when each is the other's. For an `n` too large for trial division, factor
it with Pollard's rho in section 6.3.9 and apply the product formulas; to tabulate a high narrow
window, sieve it with the primes up to $\sqrt{`hi`}$ as in section 6.3.8.

Overflow warning: $\sigma(n)$ can be several times larger than `n`, so `divisor_sum()` overflows
`int64_t` somewhat before `n` itself does. Trial division binds first in practice, since $\sqrt{n}$
divisions are already too slow well below that point; factor with section 6.3.9 instead.

Time Complexity:
- O(sqrt(n)) per call to `divisor_count(n)` and `divisor_sum(n)`.
- O(n log n) per call to `divisor_count_table(n)` and `divisor_sum_table(n)`.

Space Complexity:
- O(1) auxiliary for `divisor_count()` and `divisor_sum()`.
- O(n) for the vectors returned by `divisor_count_table(n)` and `divisor_sum_table(n)`.

*/

#include <cassert>
#include <cstdint>
#include <vector>

int64_t divisor_count(int64_t n) {
  assert(n > 0);
  int64_t res = 1;
  for (int64_t p = 2; p <= n / p; p++) {
    if (n % p == 0) {
      int e = 0;
      for (; n % p == 0; n /= p) {
        e++;
      }
      res *= e + 1;
    }
  }
  if (n > 1) {  // A prime cofactor larger than the square root is left over.
    res *= 2;
  }
  return res;
}

int64_t divisor_sum(int64_t n) {
  assert(n > 0);
  int64_t res = 1;
  for (int64_t p = 2; p <= n / p; p++) {
    if (n % p == 0) {
      int64_t power = 1, series = 1;
      for (; n % p == 0; n /= p) {
        power *= p;
        series += power;
      }
      res *= series;  // Overflow warning.
    }
  }
  if (n > 1) {
    res *= n + 1;
  }
  return res;
}

std::vector<int> divisor_count_table(int n) {
  std::vector<int> res(n + 1);
  for (int d = 1; d <= n; d++) {
    for (int i = d; i <= n; i += d) {
      res[i]++;
    }
  }
  return res;
}

std::vector<int64_t> divisor_sum_table(int n) {
  std::vector<int64_t> res(n + 1);
  for (int d = 1; d <= n; d++) {
    for (int i = d; i <= n; i += d) {
      res[i] += d;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(divisor_count(1) == 1);
  assert(divisor_count(12) == 6);  // 1, 2, 3, 4, 6, and 12.
  assert(divisor_count(999999937) == 2);
  assert(divisor_sum(1) == 1);
  assert(divisor_sum(12) == 28);
  assert(divisor_sum(6) == 12);  // 6 is perfect, since its aliquot sum is 6.

  // 220 and 284 are the smallest amicable pair.
  assert(divisor_sum(220) - 220 == 284);
  assert(divisor_sum(284) - 284 == 220);

  auto counts = divisor_count_table(100);
  auto sums = divisor_sum_table(100);
  assert(counts[0] == 0 && sums[0] == 0);
  for (int i = 1; i <= 100; i++) {
    assert(counts[i] == divisor_count(i));
    assert(sums[i] == divisor_sum(i));
  }
  return 0;
}
