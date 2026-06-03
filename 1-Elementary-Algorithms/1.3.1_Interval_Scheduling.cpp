/*

Selects the maximum number of non-overlapping intervals using the classic greedy algorithm that
sorts by earliest finish time. This is also known as activity selection.

Intervals are represented as half-open ranges `[start, finish)`, so two intervals are compatible if
the next interval's `start` is at least the previous interval's `finish`.

- `interval_scheduling(intervals)` returns one maximum-size compatible subset of intervals, in the
  order they are selected.
- `interval_t` stores `start`, `finish`, and the original `id`.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of intervals.

Space Complexity:
- O(n) auxiliary heap space for sorting and the answer.

*/

#include <algorithm>
#include <vector>

struct interval_t {
  int start, finish, id;

  interval_t(int start = 0, int finish = 0, int id = 0)
      : start(start), finish(finish), id(id) {}
};

bool earlier_finish(const interval_t &a, const interval_t &b) {
  return a.finish != b.finish ? a.finish < b.finish : a.start < b.start;
}

std::vector<interval_t> interval_scheduling(std::vector<interval_t> intervals) {
  std::sort(intervals.begin(), intervals.end(), earlier_finish);
  std::vector<interval_t> res;
  int last_finish = 0;
  bool first = true;
  for (int i = 0; i < (int)intervals.size(); i++) {
    if (first || intervals[i].start >= last_finish) {
      res.push_back(intervals[i]);
      last_finish = intervals[i].finish;
      first = false;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<interval_t> intervals;
  intervals.push_back(interval_t(1, 4, 0));
  intervals.push_back(interval_t(3, 5, 1));
  intervals.push_back(interval_t(0, 6, 2));
  intervals.push_back(interval_t(5, 7, 3));
  intervals.push_back(interval_t(8, 9, 4));
  vector<interval_t> chosen = interval_scheduling(intervals);
  assert(chosen.size() == 3);
  assert(chosen[0].id == 0);
  assert(chosen[1].id == 3);
  assert(chosen[2].id == 4);
  return 0;
}
