/*

Selects a maximum-size subset of jobs that can all be completed by their deadlines. Each job has a
processing time and deadline, and all jobs are available at time 0. The Moore-Hodgson algorithm
sorts by deadline and keeps the accepted jobs in a max-heap by processing time; whenever the
schedule becomes late, it removes the longest accepted job.

- `maximize_on_time_jobs(jobs)` returns the maximum number of jobs that can finish by their
  deadlines.
- `TimedJob` stores `duration` and `deadline`.

Time Complexity:
- O(n log n) per call due to sorting and heap operations.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <queue>
#include <vector>

struct TimedJob {
  int duration, deadline;
};

int maximize_on_time_jobs(std::vector<TimedJob> jobs) {
  std::sort(jobs.begin(), jobs.end(), [](const TimedJob &a, const TimedJob &b) {
    return a.deadline != b.deadline ? a.deadline < b.deadline : a.duration < b.duration;
  });
  std::priority_queue<int> durations;
  int time = 0;
  for (const auto &j : jobs) {
    time += j.duration;
    durations.push(j.duration);
    if (time > j.deadline) {
      time -= durations.top();
      durations.pop();
    }
  }
  return durations.size();
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<TimedJob> jobs{{3, 4}, {2, 3}, {1, 2}, {2, 7}};
  assert(maximize_on_time_jobs(jobs) == 3);
  return 0;
}
