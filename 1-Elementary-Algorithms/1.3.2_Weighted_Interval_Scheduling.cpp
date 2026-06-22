/*

Selects a maximum-weight subset of non-overlapping intervals using dynamic programming and binary
search. Unlike unweighted interval scheduling, the earliest finish-time greedy choice is not
sufficient when intervals have weights. With intervals sorted by finish time, the best total for the
first $i$ intervals either skips interval $i$ or adds its weight to the best total over intervals
finishing no later than its start, with that predecessor located by binary search.

Intervals are represented as half-open ranges [`start`, `finish`), so two intervals are compatible
if the next interval's `start` is at least the previous interval's `finish`.

- `select_weighted_intervals(intervals)` returns a pair (`weight`, `selected`) containing that
  maximum weight and the selected intervals as original input indices in execution order, from an
  input vector of `WeightedInterval` with fields `start`, `finish`, and `weight`. `dp[i]` stores the
  best answer using the first `i` intervals after sorting by finish time.

Time Complexity:
- O(n log n) per call due to sorting and binary searching compatible intervals.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

struct WeightedInterval {
  int start, finish;
  int64_t weight;
};

std::pair<int64_t, std::vector<int>> select_weighted_intervals(
    const std::vector<WeightedInterval> &intervals
) {
  int n = static_cast<int>(intervals.size());
  std::vector<int> order(n), finish(n), prev(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    return intervals[i].finish != intervals[j].finish ? intervals[i].finish < intervals[j].finish
                                                      : intervals[i].start < intervals[j].start;
  });
  for (int i = 0; i < n; i++) {
    finish[i] = intervals[order[i]].finish;
  }
  std::vector<int64_t> dp(n + 1, 0);
  std::vector<char> take(n + 1, false);
  for (int i = 1; i <= n; i++) {
    int j =
        std::upper_bound(finish.begin(), finish.begin() + i - 1, intervals[order[i - 1]].start) -
        finish.begin();
    prev[i - 1] = j;
    int64_t candidate = dp[j] + intervals[order[i - 1]].weight;
    if (dp[i - 1] < candidate) {
      dp[i] = candidate;
      take[i] = true;
    } else {
      dp[i] = dp[i - 1];
    }
  }
  std::vector<int> selected;
  for (int i = n; i > 0;) {
    if (take[i]) {
      selected.push_back(order[i - 1]);
      i = prev[i - 1];
    } else {
      i--;
    }
  }
  std::reverse(selected.begin(), selected.end());
  return {dp[n], selected};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<WeightedInterval> intervals{{1, 3, 5}, {2, 5, 6}, {4, 6, 5}, {6, 7, 4}, {5, 8, 11}};
  auto [weight, selected] = select_weighted_intervals(intervals);
  assert(weight == 17);
  assert((selected == vector<int>{1, 4}));
  return 0;
}
