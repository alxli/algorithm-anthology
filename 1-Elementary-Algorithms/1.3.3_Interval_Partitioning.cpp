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
#include <queue>
#include <utility>
#include <vector>

struct partition_interval_t {
  int start, finish, id;

  partition_interval_t(int start = 0, int finish = 0, int id = 0)
      : start(start), finish(finish), id(id) {}
};

bool earlier_start(const partition_interval_t &a, const partition_interval_t &b) {
  return a.start != b.start ? a.start < b.start : a.finish < b.finish;
}

std::vector<int> interval_partitioning(std::vector<partition_interval_t> intervals) {
  std::sort(intervals.begin(), intervals.end(), earlier_start);
  std::vector<int> room(intervals.size());
  std::priority_queue<
      std::pair<int, int>, std::vector<std::pair<int, int>>, std::greater<std::pair<int, int>>>
      pq;
  int rooms = 0;
  for (int i = 0; i < (int)intervals.size(); i++) {
    int id = intervals[i].id;
    if (!pq.empty() && pq.top().first <= intervals[i].start) {
      int r = pq.top().second;
      pq.pop();
      room[id] = r;
    } else {
      room[id] = rooms++;
    }
    pq.push(std::make_pair(intervals[i].finish, room[id]));
  }
  return room;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  vector<partition_interval_t> intervals;
  intervals.push_back(partition_interval_t(0, 30, 0));
  intervals.push_back(partition_interval_t(5, 10, 1));
  intervals.push_back(partition_interval_t(15, 20, 2));
  vector<int> room = interval_partitioning(intervals);
  assert(1 + *max_element(room.begin(), room.end()) == 2);
  assert(room[1] == room[2]);
  assert(room[0] != room[1]);
  return 0;
}
