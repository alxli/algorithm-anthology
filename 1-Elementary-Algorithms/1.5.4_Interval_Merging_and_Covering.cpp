/*

Two fundamental interval problems ask either for the union of a collection of intervals or for a
minimum-size subcollection that covers a target interval. For the union, sorting by start time makes
all intervals that overlap the current merged interval consecutive, so one scan can extend its
finish or begin the next disjoint interval.

For minimum covering, maintain the rightmost point covered so far. Among every interval starting at
or before that frontier, greedily select one with the farthest finish. Any feasible cover must next
use one of those intervals, and replacing that choice with the farthest-reaching one cannot make the
remaining target harder to cover. This is different from interval scheduling, which selects the
compatible interval with the earliest finish.

Intervals are represented as half-open ranges $[`start`, `finish`)$. Touching intervals may be
merged because their union is another half-open interval with no gap.

- `merge_intervals(intervals)` returns the union as disjoint intervals sorted by start. Every input
  interval must satisfy `start` < `finish`.
- `cover_interval(intervals, lo, hi)` returns a minimum-size list of original input indices whose
  intervals cover $[`lo`, `hi`)$, in the order selected, or `std::nullopt` if coverage is
  impossible. Every input interval must satisfy `start` < `finish`, and `lo` must not exceed `hi`.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of intervals.

Space Complexity:
- O(n) auxiliary and O(n) for the returned intervals or indices.

*/

#include <algorithm>
#include <cassert>
#include <numeric>
#include <optional>
#include <vector>

struct Interval {
  int start, finish;
};

std::vector<Interval> merge_intervals(std::vector<Interval> intervals) {
  assert(std::all_of(intervals.begin(), intervals.end(), [](const Interval &iv) {
    return iv.start < iv.finish;
  }));
  std::sort(intervals.begin(), intervals.end(), [](const Interval &a, const Interval &b) {
    return a.start != b.start ? a.start < b.start : a.finish < b.finish;
  });
  std::vector<Interval> merged;
  for (const auto &iv : intervals) {
    if (merged.empty() || merged.back().finish < iv.start) {
      merged.push_back(iv);
    } else {
      merged.back().finish = std::max(merged.back().finish, iv.finish);
    }
  }
  return merged;
}

std::optional<std::vector<int>> cover_interval(
    const std::vector<Interval> &intervals, int lo, int hi
) {
  assert(lo <= hi);
  assert(std::all_of(intervals.begin(), intervals.end(), [](const Interval &iv) {
    return iv.start < iv.finish;
  }));
  int n = static_cast<int>(intervals.size());
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    if (intervals[i].start != intervals[j].start) {
      return intervals[i].start < intervals[j].start;
    }
    return intervals[i].finish != intervals[j].finish ? intervals[i].finish > intervals[j].finish
                                                      : i < j;
  });
  std::vector<int> selected;
  int pos = 0, frontier = lo;
  while (frontier < hi) {
    int best = -1, best_finish = frontier;
    while (pos < n && intervals[order[pos]].start <= frontier) {
      int i = order[pos++];
      if (best_finish < intervals[i].finish ||
          (best_finish == intervals[i].finish && best != -1 && i < best)) {
        best = i;
        best_finish = intervals[i].finish;
      }
    }
    if (best == -1) {
      return std::nullopt;
    }
    selected.push_back(best);
    frontier = best_finish;
  }
  return selected;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<Interval> intervals{{8, 10}, {1, 3}, {2, 6}, {15, 18}, {10, 12}};
  auto merged = merge_intervals(intervals);
  assert(merged.size() == 3);
  assert(merged[0].start == 1 && merged[0].finish == 6);
  assert(merged[1].start == 8 && merged[1].finish == 12);
  assert(merged[2].start == 15 && merged[2].finish == 18);

  vector<Interval> cover{{0, 2}, {1, 5}, {4, 7}, {6, 10}, {2, 6}, {8, 11}};
  assert(cover_interval(cover, 0, 10) == vector<int>({0, 4, 3}));
  assert(!cover_interval({{0, 2}, {3, 5}}, 0, 5));
  assert(cover_interval({}, 4, 4) == vector<int>());
  return 0;
}
