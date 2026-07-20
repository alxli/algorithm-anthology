/*

Maintains the mean and variance of a stream of numbers in one pass using Welford's algorithm. The
naive formula $\operatorname{Var}(X) = E[X^2] - E[X]^2$ can lose precision when the two terms are
large and close together. Welford instead maintains the current mean $\bar{x}_n$ (stored as `avg`)
and the unnormalized second central moment $M_{2,n} = \sum_{i=1}^{n}(x_i - \bar{x}_n)^2$ (stored as
`m2`). Each new value shifts the running mean by its deviation divided by the new count, then adds
the product of its deviations from the old and new means to `m2`. Population and sample variance
follow by dividing `m2` by $n$ or $n - 1$.

- `OnlineStatistics()` constructs an empty summary.
- `add(x)` incorporates one more value `x` into the summary.
- `count()` returns the number of values seen so far.
- `mean()` returns the current arithmetic mean, or $0$ if the summary is empty.
- `variance_population()` returns population variance, dividing by $n$, or $0$ if the summary is
  empty.
- `variance_sample()` returns sample variance, dividing by $n - 1$, or $0$ if fewer than two values
  have been added.

Time Complexity:
- O(1) per call to `add(x)` and each query.

Space Complexity:
- O(1) storage.

*/

class OnlineStatistics {
  int n;
  double avg, m2;

 public:
  OnlineStatistics() : n(0), avg(0), m2(0) {}

  void add(double x) {
    n++;
    double delta = x - avg;
    avg += delta / n;
    double delta2 = x - avg;
    m2 += delta * delta2;
  }

  int count() const { return n; }
  double mean() const { return avg; }
  double variance_population() const { return n == 0 ? 0 : m2 / n; }
  double variance_sample() const { return n <= 1 ? 0 : m2 / (n - 1); }
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>

bool EQ(double a, double b) {
  return std::fabs(a - b) < 1e-9;
}

int main() {
  OnlineStatistics empty;
  assert(empty.count() == 0);
  assert(EQ(empty.mean(), 0.0));
  assert(EQ(empty.variance_population(), 0.0));
  assert(EQ(empty.variance_sample(), 0.0));

  OnlineStatistics singleton;
  singleton.add(7);
  assert(singleton.count() == 1);
  assert(EQ(singleton.mean(), 7.0));
  assert(EQ(singleton.variance_population(), 0.0));
  assert(EQ(singleton.variance_sample(), 0.0));

  OnlineStatistics stats;
  for (int x = 1; x <= 5; x++) {
    stats.add(x);
  }
  assert(stats.count() == 5);
  assert(EQ(stats.mean(), 3.0));
  assert(EQ(stats.variance_population(), 2.0));
  assert(EQ(stats.variance_sample(), 2.5));

  OnlineStatistics shifted;
  shifted.add(1e12 + 1);
  shifted.add(1e12 + 2);
  shifted.add(1e12 + 3);
  assert(EQ(shifted.mean(), 1e12 + 2));
  assert(EQ(shifted.variance_population(), 2.0 / 3));
  assert(EQ(shifted.variance_sample(), 1.0));
  return 0;
}
