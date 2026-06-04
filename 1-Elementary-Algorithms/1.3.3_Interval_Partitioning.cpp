/*

Assigns intervals to the minimum number of groups so that no two intervals in the same group
overlap. This is the classic interval partitioning problem, also known as finding the minimum number
of rooms needed for meetings.

The greedy algorithm sorts intervals by start time and reuses the group whose current finish time is
earliest whenever possible. Otherwise, it creates a new group.

- `interval_partitioning(intervals)` returns `room[id]`, the assigned room for each original
  interval id.
- The number of rooms used is `1 + max(room[id])`, or 0 if there are no intervals.

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

std::vector<int> interval_partitioning(std::vector<PartitionInterval> intervals) {
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
  for (const auto &iv : intervals) {
    int id = iv.id;
    if (!pq.empty() && pq.top().first <= iv.start) {
      int r = pq.top().second;
      pq.pop();
      room[id] = r;
    } else {
      room[id] = rooms++;
    }
    pq.emplace(iv.finish, room[id]);
  }
  return room;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  vector<PartitionInterval> intervals{{0, 30, 0}, {5, 10, 1}, {15, 20, 2}};
  vector<int> room = interval_partitioning(intervals);
  assert(1 + *max_element(room.begin(), room.end()) == 2);
  assert(room[1] == room[2]);
  assert(room[0] != room[1]);
  return 0;
}
