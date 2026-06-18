/*

Maintains the mean and variance of a stream of numbers in one pass using Welford's algorithm. The
naive formula $\operatorname{Var}(X) = E[X^2] - E[X]^2$ can lose precision when the two terms are
large and close together. Welford instead keeps the current mean and `m2`, the sum of squared
deviations from that mean. Each new value shifts the running mean by its deviation divided by the
new count, then adds the product of its deviations from the old and new means to `m2`. Population
and sample variance follow by dividing `m2` by $n$ or $n - 1$.

- `OnlineStatistics()` constructs an empty summary.
- `add(x)` incorporates one more value `x` into the summary.
- `count()` returns the number of values seen so far.
- `mean()` returns the current arithmetic mean.
- `variance_population()` returns population variance, dividing by $n$.
- `variance_sample()` returns sample variance, dividing by $n - 1$.

Time Complexity:
- O(1) per call to `add(x)` and each query.

Space Complexity:
- O(1) auxiliary.

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

bool close(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  OnlineStatistics stats;
  for (int x = 1; x <= 5; x++) {
    stats.add(x);
  }
  assert(stats.count() == 5);
  assert(close(stats.mean(), 3.0));
  assert(close(stats.variance_population(), 2.0));
  assert(close(stats.variance_sample(), 2.5));
  return 0;
}
