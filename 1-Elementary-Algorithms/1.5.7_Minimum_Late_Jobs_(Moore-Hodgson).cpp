/*

Selects a maximum-size subset of jobs that can all be completed by their deadlines. Each job has a
processing time and deadline, and all jobs are available at time $0$. The Moore-Hodgson algorithm
sorts by deadline and keeps the accepted jobs in a max-heap by processing time; whenever the
schedule becomes late, it removes the longest accepted job.

After considering jobs up to some deadline, if the accepted set no longer fits, at least one
accepted job must be removed. Removing the longest one leaves the most remaining time while keeping
the same number of removed jobs, so it is never worse than removing a shorter accepted job.
Repeating this repair after each deadline leaves a largest feasible accepted set.

- `max_on_time_jobs(jobs)` returns the selected jobs as original input indices in a feasible
  earliest-deadline-first execution order, for an input vector of `TimedJob` with fields `duration`
  and `deadline`. Durations and deadlines must be nonnegative integers.

Overflow warning: The total duration must fit in `int64_t`.

Time Complexity:
- O(n log n) per call due to sorting and heap operations.

Space Complexity:
- O(n) auxiliary and O(n) for the returned indices.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <queue>
#include <utility>
#include <vector>

struct TimedJob {
  int duration, deadline;
};

std::vector<int> max_on_time_jobs(const std::vector<TimedJob> &jobs) {
  assert(std::all_of(jobs.begin(), jobs.end(), [](const TimedJob &job) {
    return job.duration >= 0 && job.deadline >= 0;
  }));
  int n = static_cast<int>(jobs.size());
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    return jobs[i].deadline != jobs[j].deadline ? jobs[i].deadline < jobs[j].deadline
                                                : jobs[i].duration < jobs[j].duration;
  });
  std::priority_queue<std::pair<int, int>> accepted;
  std::vector<char> selected(n);
  int64_t time = 0;
  for (int i : order) {
    time += jobs[i].duration;  // Overflow warning.
    accepted.emplace(jobs[i].duration, i);
    selected[i] = true;
    if (time > jobs[i].deadline) {
      time -= accepted.top().first;
      selected[accepted.top().second] = false;
      accepted.pop();
    }
  }
  std::vector<int> res;
  for (int i : order) {
    if (selected[i]) {
      res.push_back(i);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<TimedJob> jobs{{3, 4}, {2, 3}, {1, 2}, {2, 7}};
  assert((max_on_time_jobs(jobs) == vector<int>{2, 1, 3}));
  return 0;
}
