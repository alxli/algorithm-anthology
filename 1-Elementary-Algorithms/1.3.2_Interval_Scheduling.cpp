/*

Selects the maximum number of non-overlapping intervals using the classic earliest-finish-time
greedy algorithm. This is the unweighted interval scheduling problem, also known as activity
selection: every interval has the same value, so the objective is to choose as many compatible
intervals as possible.

The greedy choice is safe because among all intervals that could be chosen first, taking one with
the earliest finish time leaves the most room for the remaining intervals. In any optimal solution,
the first chosen interval can be exchanged for an earliest-finishing compatible interval without
reducing the number of intervals selected. Repeating this argument after each choice proves the
greedy algorithm optimal. The weighted version in 1.3.3 needs dynamic programming instead.

Intervals are represented as half-open ranges $[`start`, `finish`)$, so two intervals are compatible
if the next interval's `start` is at least the previous interval's `finish`.

- `schedule_intervals(intervals)` returns one maximum-size compatible subset of intervals, in the
  order they are selected, as original indices into an input vector of `Interval` with fields
  `start` and `finish`. Every interval must satisfy `start` < `finish`.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of intervals.

Space Complexity:
- O(n) auxiliary and O(n) for the returned indices.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <numeric>
#include <vector>

struct Interval {
  int start, finish;
};

std::vector<int> schedule_intervals(const std::vector<Interval> &intervals) {
  int n = static_cast<int>(intervals.size());
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  for (const auto &iv : intervals) {
    assert(iv.start < iv.finish);
  }
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    return intervals[i].finish != intervals[j].finish ? intervals[i].finish < intervals[j].finish
                                                      : intervals[i].start < intervals[j].start;
  });
  std::vector<int> selected;
  int last_finish = INT_MIN;
  for (int i : order) {
    const auto &iv = intervals[i];
    if (iv.start >= last_finish) {
      selected.push_back(i);
      last_finish = iv.finish;
    }
  }
  return selected;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Interval> intervals{{1, 4}, {3, 5}, {0, 6}, {5, 7}, {8, 9}};
  vector<int> selected = schedule_intervals(intervals);
  // Earliest-finish greedy chooses original indices 0, 3, 4.
  assert((selected == vector<int>{0, 3, 4}));

  vector<Interval> touching{{0, 2}, {2, 4}, {4, 5}};
  assert(schedule_intervals(touching).size() == 3);
  return 0;
}
