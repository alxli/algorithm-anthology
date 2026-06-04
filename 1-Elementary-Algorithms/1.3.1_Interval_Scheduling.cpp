/*

Selects the maximum number of non-overlapping intervals using the classic greedy algorithm that
sorts by earliest finish time. This is also known as activity selection.

Intervals are represented as half-open ranges `[start, finish)`, so two intervals are compatible if
the next interval's `start` is at least the previous interval's `finish`.

- `interval_scheduling(intervals)` returns one maximum-size compatible subset of intervals, in the
  order they are selected.
- `Interval` stores `start`, `finish`, and the original `id`.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of intervals.

Space Complexity:
- O(n) auxiliary heap space for sorting and the answer.

*/

#include <algorithm>
#include <vector>

struct Interval {
  int start, finish, id;

  explicit Interval(int start = 0, int finish = 0, int id = 0)
      : start(start), finish(finish), id(id) {}
};

std::vector<Interval> interval_scheduling(std::vector<Interval> intervals) {
  std::sort(intervals.begin(), intervals.end(), [](const Interval &a, const Interval &b) {
    return a.finish != b.finish ? a.finish < b.finish : a.start < b.start;
  });
  std::vector<Interval> res;
  int last_finish = 0;
  bool first = true;
  for (const auto &iv : intervals) {
    if (first || iv.start >= last_finish) {
      res.push_back(iv);
      last_finish = iv.finish;
      first = false;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Interval> intervals;
  intervals.emplace_back(1, 4, 0);
  intervals.emplace_back(3, 5, 1);
  intervals.emplace_back(0, 6, 2);
  intervals.emplace_back(5, 7, 3);
  intervals.emplace_back(8, 9, 4);
  vector<Interval> chosen = interval_scheduling(intervals);
  assert(chosen.size() == 3);
  assert(chosen[0].id == 0);
  assert(chosen[1].id == 3);
  assert(chosen[2].id == 4);
  return 0;
}
