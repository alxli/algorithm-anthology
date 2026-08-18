/*

Online stream summaries update useful statistics as values arrive without storing the full stream.
This section uses Welford's algorithm to maintain the mean and variance of a stream in constant
space, then merges Welford summaries inside a queue to maintain the same statistics over a sliding
window. For the median of a stream or of a trailing window, see section 1.6.5.

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
#include <utility>
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

To additionally support FIFO removals, Welford's update cannot simply be run backwards to drop the
oldest value: reversing it turns `m2` into a difference, which loses the very property that made the
forward pass stable, and the resulting error has no way to correct itself as the window advances.

Summaries are mergeable instead. Two summaries of disjoint groups combine by shifting the mean
toward the larger group and adding the spread between the two group means, so that
$M_2 = M_{2,A} + M_{2,B} + \delta^2 n_A n_B / n$, where $\delta$ is the gap between the group means.
That merge is associative, which makes summaries a monoid and puts them within reach of the sliding
window aggregation (SWAG) technique of section 1.2.5: a queue of two stacks, each entry storing the
fold of everything between it and its end of the queue, reports the fold of the entire queue in O(1)
amortized time. The class below is that structure specialized to this summary, so section 1.2.5 is
the place to look for the general version over any associative operation, and for why flipping the
back stack onto the front stack costs only O(1) amortized per value.

Every value still enters through a forward Welford update of a one-element summary, and the folds
only ever add nonnegative terms, so the statistics of a window are as stable as those of a whole
stream.

- `StatisticsQueue()` constructs an empty summary.
- `add(x)` incorporates `x` as the newest value of the queue.
- `remove()` drops the oldest value of the queue, which must be nonempty.
- `count()` returns the number of values currently in the queue.
- `mean()`, `var_pop()`, and `var_samp()` return the same statistics as above, over the values
  currently in the queue.

The window is left to the caller rather than fixed by the constructor. A trailing window of width
$w$ is one call to `add()` followed by a call to `remove()` whenever `count()` exceeds $w$, while a
variable-width window from the two-pointer patterns of section 1.2.3 instead removes values until
its own condition holds again.

Time Complexity:
- O(1) amortized per call to `add()` and `remove()`, and O(1) per query.

Space Complexity:
- O(n) storage, where $n$ is the number of values currently in the queue.

*/

class StatisticsQueue {
  struct Summary {
    int n;
    double mean, m2;
  };

  // Chan's parallel merge of two disjoint groups, which never subtracts a value out of m2.
  static Summary merge(const Summary &a, const Summary &b) {
    int n = a.n + b.n;
    double delta = b.mean - a.mean;
    return {n, a.mean + delta * b.n / n, a.m2 + b.m2 + delta * delta * a.n * b.n / n};
  }

  // Each stack entry is (summary of one value, fold), where the fold merges that value with all
  // entries between it and its end of the queue (the front for front_stack, back for back_stack).
  std::vector<std::pair<Summary, Summary>> front_stack, back_stack;

  Summary fold() const {
    if (front_stack.empty()) {
      return back_stack.empty() ? Summary{} : back_stack.back().second;
    }
    if (back_stack.empty()) {
      return front_stack.back().second;
    }
    return merge(front_stack.back().second, back_stack.back().second);
  }

 public:
  void add(double x) {
    Summary value{1, x, 0};
    back_stack.emplace_back(
        value, back_stack.empty() ? value : merge(back_stack.back().second, value)
    );
  }

  void remove() {
    assert(count() > 0);
    if (front_stack.empty()) {
      // Flip the back stack onto the front stack, reversing order so the oldest value ends up on
      // top, and recompute the front folds in queue order.
      while (!back_stack.empty()) {
        Summary oldest = back_stack.back().first;
        back_stack.pop_back();
        front_stack.emplace_back(
            oldest, front_stack.empty() ? oldest : merge(oldest, front_stack.back().second)
        );
      }
    }
    front_stack.pop_back();
  }

  int count() const { return static_cast<int>(front_stack.size() + back_stack.size()); }
  double mean() const { return fold().mean; }

  double var_pop() const {
    Summary s = fold();
    return s.n == 0 ? 0 : s.m2 / s.n;
  }

  double var_samp() const {
    Summary s = fold();
    return s.n <= 1 ? 0 : s.m2 / (s.n - 1);
  }
};

/*** Example Usage ***/

#include <cassert>
#include <cmath>
#include <vector>
using namespace std;

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

  StatisticsQueue empty_queue;
  assert(empty_queue.count() == 0 && EQ(empty_queue.mean(), 0.0));
  assert(EQ(empty_queue.var_pop(), 0.0) && EQ(empty_queue.var_samp(), 0.0));

  // A trailing window of width 3, maintained by the caller.
  StatisticsQueue window;
  for (int x = 1; x <= 5; x++) {
    window.add(x);
    if (window.count() > 3) {
      window.remove();
    }
  }
  assert(window.count() == 3);  // The window holds 3, 4, and 5.
  assert(EQ(window.mean(), 4.0));
  assert(EQ(window.var_pop(), 2.0 / 3) && EQ(window.var_samp(), 1.0));

  // The same shifted values as above, now with an extra value removed from the front first.
  StatisticsQueue shifted_window;
  shifted_window.add(1e12);
  shifted_window.add(1e12 + 1);
  shifted_window.add(1e12 + 2);
  shifted_window.add(1e12 + 3);
  shifted_window.remove();
  assert(EQ(shifted_window.mean(), 1e12 + 2));
  assert(EQ(shifted_window.var_pop(), 2.0 / 3) && EQ(shifted_window.var_samp(), 1.0));

  // A variable-width window, which a fixed size cannot express: extend while the population
  // variance stays at most 1, shrinking from the front whenever it does not.
  StatisticsQueue spread;
  int longest = 0;
  for (double x : {1.0, 1.0, 2.0, 8.0, 8.0, 9.0}) {
    spread.add(x);
    while (spread.var_pop() > 1) {
      spread.remove();
    }
    if (spread.count() > longest) {
      longest = spread.count();
    }
  }
  assert(longest == 3);  // The values 1, 1, and 2 have population variance 2/9.

  return 0;
}
