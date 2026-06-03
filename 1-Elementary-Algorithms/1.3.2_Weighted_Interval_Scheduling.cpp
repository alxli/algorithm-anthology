/*

Selects a maximum-weight subset of non-overlapping intervals using dynamic programming and binary
search. Unlike unweighted interval scheduling, the earliest finish-time greedy choice is not
sufficient when intervals have weights.

Intervals are represented as half-open ranges `[start, finish)`, so two intervals are compatible if
the next interval's `start` is at least the previous interval's `finish`.

- `weighted_interval_scheduling(intervals)` returns the maximum total weight of a compatible subset.
- `WeightedInterval` stores `start`, `finish`, and `weight`.

Time Complexity:
- O(n log n) per call due to sorting and binary searching compatible intervals.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <vector>

struct WeightedInterval {
  int start, finish;
  long long weight;

  WeightedInterval(int start = 0, int finish = 0, long long weight = 0)
      : start(start), finish(finish), weight(weight) {}
};

bool weighted_earlier_finish(const WeightedInterval &a, const WeightedInterval &b) {
  return a.finish != b.finish ? a.finish < b.finish : a.start < b.start;
}

long long weighted_interval_scheduling(std::vector<WeightedInterval> intervals) {
  std::sort(intervals.begin(), intervals.end(), weighted_earlier_finish);
  int n = intervals.size();
  std::vector<int> finish(n);
  for (int i = 0; i < n; i++) {
    finish[i] = intervals[i].finish;
  }
  std::vector<long long> dp(n + 1, 0);
  for (int i = 1; i <= n; i++) {
    int j = std::upper_bound(finish.begin(), finish.end(), intervals[i - 1].start) - finish.begin();
    dp[i] = std::max(dp[i - 1], dp[j] + intervals[i - 1].weight);
  }
  return dp[n];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<WeightedInterval> intervals;
  intervals.push_back(WeightedInterval(1, 3, 5));
  intervals.push_back(WeightedInterval(2, 5, 6));
  intervals.push_back(WeightedInterval(4, 6, 5));
  intervals.push_back(WeightedInterval(6, 7, 4));
  intervals.push_back(WeightedInterval(5, 8, 11));
  assert(weighted_interval_scheduling(intervals) == 17);
  return 0;
}
