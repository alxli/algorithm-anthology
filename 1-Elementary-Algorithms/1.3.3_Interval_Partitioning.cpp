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

Intervals are treated as half-open ranges [`start`, `finish`), so one interval may reuse a room
that another interval vacates at the same time.

- `partition_intervals(intervals)` returns a vector `room`, where `room[id]` is the assigned room
  for each input interval given as a vector of `PartitionInterval` with fields `start`, `finish`,
  and `id`. The number of rooms used in the returned assignment is $1 + \max(`room[id]`)$, or 0 if
  there are no intervals.

Time Complexity:
- O(n log n) per call due to sorting and priority queue operations.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

struct PartitionInterval {
  int start, finish, id;
};

std::vector<int> partition_intervals(std::vector<PartitionInterval> intervals) {
  std::sort(
      intervals.begin(), intervals.end(),
      [](const PartitionInterval &a, const PartitionInterval &b) {
        return a.start != b.start ? a.start < b.start : a.finish < b.finish;
      }
  );
  std::vector<int> room(intervals.size());
  std::priority_queue<
      std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>>
      pq;
  int rooms = 0;
  for (const auto &[start, finish, id] : intervals) {
    if (!pq.empty() && pq.top().first <= start) {
      int r = pq.top().second;
      pq.pop();
      room[id] = r;
    } else {
      room[id] = rooms++;
    }
    pq.emplace(finish, room[id]);
  }
  return room;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  vector<PartitionInterval> intervals{{0, 30, 0}, {5, 10, 1}, {15, 20, 2}};
  vector<int> room = partition_intervals(intervals);
  assert(1 + *max_element(room.begin(), room.end()) == 2);
  assert(room[1] == room[2]);
  assert(room[0] != room[1]);
  return 0;
}
