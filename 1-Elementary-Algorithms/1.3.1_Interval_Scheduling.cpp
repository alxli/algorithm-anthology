/*

Selects the maximum number of non-overlapping intervals using the classic earliest-finish-time
greedy algorithm. This is the unweighted interval scheduling problem, also known as activity
selection: every interval has the same value, so the objective is to choose as many compatible
intervals as possible.

The greedy choice is safe because among all intervals that could be chosen first, taking one with
the earliest finish time leaves the most room for the remaining intervals. In any optimal solution,
the first chosen interval can be exchanged for an earliest-finishing compatible interval without
reducing the number of intervals selected. Repeating this argument after each choice proves the
greedy algorithm optimal. The weighted version in 1.3.2 needs dynamic programming instead.

Intervals are represented as half-open ranges $[`start`, `finish`)$, so two intervals are compatible
if the next interval's `start` is at least the previous interval's `finish`.

- `schedule_intervals(intervals)` returns one maximum-size compatible subset of intervals, in the
  order they are selected, from an input vector of `Interval` with fields `start`, `finish`, and the
  original `id`.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of intervals.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <climits>
#include <vector>

struct Interval {
  int start, finish, id;
};

std::vector<Interval> schedule_intervals(std::vector<Interval> intervals) {
  std::sort(intervals.begin(), intervals.end(), [](const Interval &a, const Interval &b) {
    return a.finish != b.finish ? a.finish < b.finish : a.start < b.start;
  });
  std::vector<Interval> res;
  int last_finish = INT_MIN;
  for (const auto &iv : intervals) {
    if (iv.start >= last_finish) {
      res.push_back(iv);
      last_finish = iv.finish;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Interval> intervals{{1, 4, 0}, {3, 5, 1}, {0, 6, 2}, {5, 7, 3}, {8, 9, 4}};
  vector<Interval> chosen = schedule_intervals(intervals);
  // Earliest-finish greedy chooses ids 0, 3, 4.
  assert(chosen.size() == 3);
  assert(chosen[0].id == 0);
  assert(chosen[1].id == 3);
  assert(chosen[2].id == 4);

  vector<Interval> touching{{0, 2, 0}, {2, 4, 1}, {4, 5, 2}};
  assert(schedule_intervals(touching).size() == 3);
  return 0;
}
