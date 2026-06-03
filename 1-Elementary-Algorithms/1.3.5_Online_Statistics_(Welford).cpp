/*

Maintains the mean and variance of a stream of numbers in one pass using
Welford's algorithm. This is more numerically stable than maintaining the sum of
values and the sum of squares separately.

- `online_statistics()` constructs an empty summary.
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

class online_statistics {
  int n;
  double avg, m2;

 public:
  online_statistics() : n(0), avg(0), m2(0) {}

  void add(double x) {
    n++;
    double delta = x - avg;
    avg += delta / n;
    double delta2 = x - avg;
    m2 += delta * delta2;
  }

  int count() const {
    return n;
  }

  double mean() const {
    return avg;
  }

  double variance_population() const {
    return n == 0 ? 0 : m2 / n;
  }

  double variance_sample() const {
    return n <= 1 ? 0 : m2 / (n - 1);
  }
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>

bool close(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  online_statistics stats;
  for (int x = 1; x <= 5; x++) {
    stats.add(x);
  }
  assert(stats.count() == 5);
  assert(close(stats.mean(), 3.0));
  assert(close(stats.variance_population(), 2.0));
  assert(close(stats.variance_sample(), 2.5));
  return 0;
}
