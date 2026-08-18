/*

The following functions evaluate the standard probability distributions. For a discrete
distribution, the probability mass function gives the probability of an exact outcome and the
cumulative distribution function gives the probability of that outcome or any smaller one. For a
continuous distribution, the density integrates to those probabilities rather than being one, so
only the cumulative function is a probability. Counts must be nonnegative, and every probability
argument must lie in $[0, 1]$.

Every mass function here is evaluated in logarithms and exponentiated once at the end. Computing
$\binom{n}{k} p^k (1 - p)^{n-k}$ directly overflows the binomial coefficient and underflows the
powers long before the product itself becomes small, whereas the logarithms stay near the magnitude
of the answer. `std::lgamma` supplies $\ln n!$ as $\ln \Gamma(n + 1)$ without forming $n!$, so the
factorials never exist, and outcomes are ordered so that no cancellation occurs.

Each cumulative function sums its mass function term by term, which keeps the code short and the
error small but costs one term per outcome. For a single tail probability at a very large count the
normal approximation with a continuity correction is the usual substitute, while `quantile()`
inverts any of these by the bisection of section 5.1.1.

- `log_factorial(n)` returns $\ln(n!)$, and `log_choose(n, k)` returns $\ln \binom{n}{k}$, which is
  $-\infty$ when `k` lies outside $[0, `n`]$.
- `binomial_pmf(n, k, p)` and `binomial_cdf(n, k, p)` evaluate the number of successes in `n`
  independent trials that each succeed with probability `p`.
- `geometric_pmf(k, p)` and `geometric_cdf(k, p)` evaluate the number of trials up to and including
  the first success, so `k` is at least $1$ and `p` must be positive.
- `negative_binomial_pmf(k, r, p)` and `negative_binomial_cdf(k, r, p)` evaluate the number of
  trials up to and including the `r`-th success, so `k` is at least `r`, which is at least $1$.
- `poisson_pmf(k, lambda)` and `poisson_cdf(k, lambda)` evaluate the number of events in one unit of
  time when events arrive independently at nonnegative mean rate `lambda`.
- `hypergeometric_pmf(n, k, draws, hits)` and `hypergeometric_cdf(n, k, draws, hits)` evaluate the
  number of marked items among `draws` items taken without replacement from a population of `n`
  items of which `k` are marked.
- `normal_pdf(x, mu = 0, sigma = 1)` and `normal_cdf(x, mu = 0, sigma = 1)` evaluate the normal
  distribution of mean `mu` and positive standard deviation `sigma`, defaulting to the standard
  normal distribution.
- `exponential_pdf(x, lambda)` and `exponential_cdf(x, lambda)` evaluate the waiting time until the
  next event when events arrive at positive mean rate `lambda`.
- `quantile(cdf, p, lo, hi, iterations = 100)` returns the smallest $x$ in [`lo`, `hi`] whose
  cumulative probability `cdf(x)` reaches `p`, for any nondecreasing `cdf`.

Sampling is left to `<random>`, whose `std::binomial_distribution`, `std::poisson_distribution`,
`std::normal_distribution`, and their siblings draw from exactly these families. What the standard
library does not provide, and this section supplies, is the ability to evaluate and invert them.

Overflow warning: `log_choose()` is exact only while `std::lgamma` resolves neighboring integers,
which holds well past the range where a `double` probability retains any precision.

Time Complexity:
- O(1) per call to every mass and density function, and to `log_factorial()` and `log_choose()`.
- O(k) per call to `binomial_cdf()`, `poisson_cdf()`, `negative_binomial_cdf()`, and
  `hypergeometric_cdf()`, where $k$ is the outcome whose tail is summed.
- O(1) per call to `geometric_cdf()`, `normal_cdf()`, and `exponential_cdf()`, which have closed
  forms.
- O(n) calls to `cdf()` per call to `quantile()`, where $n$ is the number of iterations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <limits>

double log_factorial(int n) {
  return std::lgamma(n + 1.0);
}

double log_choose(int n, int k) {
  if (k < 0 || k > n) {
    return -std::numeric_limits<double>::infinity();
  }
  return log_factorial(n) - log_factorial(k) - log_factorial(n - k);
}

double binomial_pmf(int n, int k, double p) {
  if (k < 0 || k > n) {
    return 0;
  }
  if (p <= 0) {
    return k == 0 ? 1 : 0;
  }
  if (p >= 1) {
    return k == n ? 1 : 0;
  }
  return std::exp(log_choose(n, k) + k * std::log(p) + (n - k) * std::log1p(-p));
}

double binomial_cdf(int n, int k, double p) {
  double res = 0;
  for (int i = 0; i <= std::min(k, n); i++) {
    res += binomial_pmf(n, i, p);
  }
  return std::min(res, 1.0);
}

double geometric_pmf(int k, double p) {
  return k < 1 ? 0 : p * std::pow(1 - p, k - 1);
}

double geometric_cdf(int k, double p) {
  return k < 1 ? 0 : -std::expm1(k * std::log1p(-p));
}

double negative_binomial_pmf(int k, int r, double p) {
  if (k < r || r < 1) {
    return 0;
  }
  if (p >= 1) {
    return k == r ? 1 : 0;
  }
  return std::exp(log_choose(k - 1, r - 1) + r * std::log(p) + (k - r) * std::log1p(-p));
}

double negative_binomial_cdf(int k, int r, double p) {
  double res = 0;
  for (int i = r; i <= k; i++) {
    res += negative_binomial_pmf(i, r, p);
  }
  return std::min(res, 1.0);
}

double poisson_pmf(int k, double lambda) {
  if (k < 0) {
    return 0;
  }
  if (lambda <= 0) {
    return k == 0 ? 1 : 0;
  }
  return std::exp(-lambda + k * std::log(lambda) - log_factorial(k));
}

double poisson_cdf(int k, double lambda) {
  double res = 0;
  for (int i = 0; i <= k; i++) {
    res += poisson_pmf(i, lambda);
  }
  return std::min(res, 1.0);
}

double hypergeometric_pmf(int n, int k, int draws, int hits) {
  if (hits < 0 || hits > k || draws - hits < 0 || draws - hits > n - k) {
    return 0;
  }
  return std::exp(log_choose(k, hits) + log_choose(n - k, draws - hits) - log_choose(n, draws));
}

double hypergeometric_cdf(int n, int k, int draws, int hits) {
  double res = 0;
  for (int i = 0; i <= hits; i++) {
    res += hypergeometric_pmf(n, k, draws, i);
  }
  return std::min(res, 1.0);
}

double normal_pdf(double x, double mu = 0, double sigma = 1) {
  static const double INV_SQRT_2PI = 0.3989422804014327;
  double z = (x - mu) / sigma;
  return INV_SQRT_2PI / sigma * std::exp(-0.5 * z * z);
}

double normal_cdf(double x, double mu = 0, double sigma = 1) {
  static const double INV_SQRT_2 = 0.7071067811865476;
  return 0.5 * std::erfc(-(x - mu) * INV_SQRT_2 / sigma);
}

double exponential_pdf(double x, double lambda) {
  return x < 0 ? 0 : lambda * std::exp(-lambda * x);
}

double exponential_cdf(double x, double lambda) {
  return x < 0 ? 0 : -std::expm1(-lambda * x);
}

template<typename Cdf>
double quantile(Cdf cdf, double p, double lo, double hi, int iterations = 100) {
  for (int i = 0; i < iterations; i++) {
    double mid = lo + (hi - lo) / 2;
    if (cdf(mid) >= p) {
      hi = mid;
    } else {
      lo = mid;
    }
  }
  return hi;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  // Three fair coin flips give two heads with probability 3/8.
  assert(EQ(binomial_pmf(3, 2, 0.5), 0.375));
  assert(EQ(binomial_cdf(3, 3, 0.5), 1));
  // Large counts stay finite where the binomial coefficient alone would not.
  assert(EQ(binomial_cdf(1000, 1000, 0.5), 1));
  assert(binomial_pmf(1000, 500, 0.5) > 0.02);

  // A fair die shows its first six on the third roll with probability 25/216.
  assert(EQ(geometric_pmf(3, 1.0 / 6), 25.0 / 216));
  assert(EQ(geometric_cdf(1, 0.25), 0.25));
  assert(EQ(negative_binomial_pmf(3, 2, 0.5), 0.25));

  assert(EQ(poisson_pmf(0, 2), exp(-2.0)));
  assert(EQ(poisson_cdf(1, 1), 2 * exp(-1.0)));

  // Two marked items among five drawn from ten, of which four are marked.
  assert(EQ(hypergeometric_pmf(10, 4, 5, 2), 10.0 / 21));
  assert(EQ(hypergeometric_cdf(10, 4, 5, 4), 1));

  assert(EQ(normal_cdf(0), 0.5));
  assert(fabs(normal_cdf(1.959963984540054) - 0.975) < 1e-9);
  assert(EQ(exponential_cdf(0, 3), 0));

  // Inverting a cumulative function recovers the value that produced it.
  assert(
      fabs(quantile([](double x) { return normal_cdf(x); }, 0.975, -10, 10) - 1.959963984540054) <
      1e-9
  );
  return 0;
}
