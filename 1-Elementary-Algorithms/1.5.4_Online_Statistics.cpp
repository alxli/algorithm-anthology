/*

Online stream summaries update useful statistics as values arrive without storing the full stream.
This section uses Welford's algorithm for the mean and variance and two balanced heaps for the
median.

The naive variance formula $\operatorname{Var}(X) = E[X^2] - E[X]^2$ can lose precision when the two
terms are large and close together. Welford instead maintains the current mean $\bar{x}_n$ (stored
as `avg`) and the unnormalized second central moment $M_{2,n} = \sum_{i=1}^{n}(x_i - \bar{x}_n)^2$
(stored as `m2`). Each new value shifts the running mean by its deviation divided by the new count,
then adds the product of its deviations from the old and new means to `m2`.

- `OnlineStatistics()` constructs an empty summary.
- `add(x)` incorporates one more value `x` into the summary.
- `count()` returns the number of values seen so far.
- `mean()` returns the current arithmetic mean, or $0$ if the summary is empty.
- `var_pop()` returns population variance, dividing by $n$, or $0$ if the summary is empty.
- `var_samp()` returns sample variance, dividing by $n - 1$, or $0$ if fewer than two values have
  been added.

Time Complexity:
- O(1) per call to `add()` and each query.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <functional>
#include <queue>
#include <vector>

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
  double var_pop() const { return n == 0 ? 0 : m2 / n; }
  double var_samp() const { return n <= 1 ? 0 : m2 / (n - 1); }
};

/*

Maintains the exact median of a stream using linear storage. A max-heap stores the lower half of the
values and a min-heap stores the upper half. The heaps are balanced so that the lower heap has
either the same number of values as the upper heap or one extra value. Their roots are therefore the
middle value or values of the stream.

- `OnlineMedian<T>()` constructs an empty median tracker for an ordered numeric type `T`.
- `add(x)` inserts `x` into the median tracker.
- `count()` and `empty()` return the number of values and whether the tracker is empty.
- `lower_median()` returns the lower of the two middle values, or the middle value for an odd count.
  The tracker must be nonempty.
- `median()` returns the middle value for an odd count or the arithmetic mean of the two middle
  values for an even count. The tracker must be nonempty.

Time Complexity:
- O(log n) per call to `add()` and O(1) per query, where $n$ is the number of values.

Space Complexity:
- O(n) storage.

*/

template<typename T>
class OnlineMedian {
  std::priority_queue<T> lower;
  std::priority_queue<T, std::vector<T>, std::greater<T>> upper;

 public:
  bool empty() const { return lower.empty(); }
  int count() const { return static_cast<int>(lower.size() + upper.size()); }

  void add(const T &x) {
    if (lower.empty() || x <= lower.top()) {
      lower.push(x);
    } else {
      upper.push(x);
    }
    if (lower.size() < upper.size()) {
      lower.push(upper.top());
      upper.pop();
    } else if (lower.size() > upper.size() + 1) {
      upper.push(lower.top());
      lower.pop();
    }
  }

  const T &lower_median() const {
    assert(!empty());
    return lower.top();
  }

  double median() const {
    assert(!empty());
    if (lower.size() != upper.size()) {
      return static_cast<double>(lower.top());
    }
    return static_cast<double>(lower.top()) / 2 + static_cast<double>(upper.top()) / 2;
  }
};

/*** Example Usage ***/

#include <cmath>

bool EQ(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  OnlineStatistics empty;
  assert(empty.count() == 0 && EQ(empty.mean(), 0.0));
  assert(EQ(empty.var_pop(), 0.0) && EQ(empty.var_samp(), 0.0));

  OnlineStatistics singleton;
  singleton.add(7);
  assert(singleton.count() == 1 && EQ(singleton.mean(), 7.0));
  assert(EQ(singleton.var_pop(), 0.0) && EQ(singleton.var_samp(), 0.0));

  OnlineStatistics stats;
  for (int x = 1; x <= 5; x++) {
    stats.add(x);
  }
  assert(stats.count() == 5 && EQ(stats.mean(), 3.0));
  assert(EQ(stats.var_pop(), 2.0) && EQ(stats.var_samp(), 2.5));

  OnlineStatistics shifted;
  shifted.add(1e12 + 1);
  shifted.add(1e12 + 2);
  shifted.add(1e12 + 3);
  assert(EQ(shifted.mean(), 1e12 + 2));
  assert(EQ(shifted.var_pop(), 2.0 / 3) && EQ(shifted.var_samp(), 1.0));

  OnlineMedian<int> medians;
  assert(medians.empty() && medians.count() == 0);
  medians.add(5);
  assert(EQ(medians.median(), 5));
  medians.add(1);
  assert(EQ(medians.median(), 3));
  medians.add(9);
  assert(EQ(medians.median(), 5));
  medians.add(3);
  assert(medians.lower_median() == 3);
  assert(EQ(medians.median(), 4));
  return 0;
}
