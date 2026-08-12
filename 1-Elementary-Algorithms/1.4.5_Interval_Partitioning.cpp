/*

Assigns intervals to the minimum number of groups so that no two intervals in the same group
overlap. This is the classic interval partitioning problem, also known as finding the minimum number
of rooms needed for meetings.

The greedy algorithm sorts intervals by start time and reuses the group whose current finish time is
earliest whenever possible. Otherwise, it creates a new group. This is optimal because the number of
groups must be at least the maximum number of intervals overlapping at any time. When the
earliest-finishing active group still overlaps the next interval, every active group overlaps it, so
creating a new group is unavoidable. If that group is free, reusing it preserves as many later
options as possible.

Intervals are treated as half-open ranges $[`start`, `finish`)$, so one interval may reuse a room
that another interval vacates at the same time.

- `partition_intervals(intervals)` returns a pair (`rooms`, `room`), where `rooms` is the minimum
  number of rooms and `room[i]` is the assigned room for input interval `i`. Each `Interval` has
  fields `start` and `finish`, which must satisfy `start` < `finish`.

Time Complexity:
- O(n log n) per call due to sorting and priority queue operations.

Space Complexity:
- O(n) auxiliary and O(n) for the returned assignment.

*/

#include <algorithm>
#include <cassert>
#include <functional>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>

struct Interval {
  int start, finish;
};

std::pair<int, std::vector<int>> partition_intervals(const std::vector<Interval> &intervals) {
  for (const auto &iv : intervals) {
    assert(iv.start < iv.finish);
  }
  int n = static_cast<int>(intervals.size());
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    return intervals[i].start != intervals[j].start ? intervals[i].start < intervals[j].start
                                                    : intervals[i].finish < intervals[j].finish;
  });
  std::vector<int> room(n);
  std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<>> pq;
  int rooms = 0;
  for (int i : order) {
    if (!pq.empty() && pq.top().first <= intervals[i].start) {
      int r = pq.top().second;
      pq.pop();
      room[i] = r;
    } else {
      room[i] = rooms++;
    }
    pq.emplace(intervals[i].finish, room[i]);
  }
  return {rooms, room};
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  vector<Interval> intervals{{0, 30}, {5, 10}, {15, 20}};
  auto [rooms, room] = partition_intervals(intervals);
  // The long interval overlaps both short intervals, but the short intervals can share a room.
  assert(rooms == 2);
  assert(room[1] == room[2]);
  assert(room[0] != room[1]);

  vector<Interval> touching{{0, 2}, {2, 4}, {4, 5}};
  auto [touching_rooms, touching_room] = partition_intervals(touching);
  assert(touching_rooms == 1 && *max_element(touching_room.begin(), touching_room.end()) == 0);
  assert(partition_intervals({}).first == 0);
  return 0;
}
