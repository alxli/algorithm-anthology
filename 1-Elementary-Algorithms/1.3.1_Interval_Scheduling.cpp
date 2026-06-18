/*

Selects the maximum number of non-overlapping intervals using the classic greedy algorithm that
sorts by earliest finish time. This is also known as activity selection.

Intervals are represented as half-open ranges [`start`, `finish`), so two intervals are compatible
if the next interval's `start` is at least the previous interval's `finish`.

- `interval_scheduling(intervals)` returns one maximum-size compatible subset of intervals, in the
  order they are selected, from an input vector of `Interval` with fields `start`, `finish`, and the
  original `id`.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of intervals.

Space Complexity:
- O(n) auxiliary heap space for sorting and the answer.

*/

#include <algorithm>
#include <climits>
#include <vector>

struct Interval {
  int start, finish, id;
};

std::vector<Interval> interval_scheduling(std::vector<Interval> intervals) {
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
  vector<Interval> chosen = interval_scheduling(intervals);
  assert(chosen.size() == 3);
  assert(chosen[0].id == 0);
  assert(chosen[1].id == 3);
  assert(chosen[2].id == 4);
  return 0;
}
