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

  TimedJob(int duration = 0, int deadline = 0) : duration(duration), deadline(deadline) {}
};

bool earlier_deadline(const TimedJob &a, const TimedJob &b) {
  return a.deadline != b.deadline ? a.deadline < b.deadline : a.duration < b.duration;
}

int maximize_on_time_jobs(std::vector<TimedJob> jobs) {
  std::sort(jobs.begin(), jobs.end(), earlier_deadline);
  std::priority_queue<int> durations;
  int time = 0;
  for (int i = 0; i < static_cast<int>(jobs.size()); i++) {
    time += jobs[i].duration;
    durations.push(jobs[i].duration);
    if (time > jobs[i].deadline) {
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
  vector<TimedJob> jobs;
  jobs.push_back(TimedJob(3, 4));
  jobs.push_back(TimedJob(2, 3));
  jobs.push_back(TimedJob(1, 2));
  jobs.push_back(TimedJob(2, 7));
  assert(maximize_on_time_jobs(jobs) == 3);
  return 0;
}
