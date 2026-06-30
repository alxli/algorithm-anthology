/*

Schedules unit-time jobs with deadlines and profits to maximize total profit. Each job takes one
time slot and must be scheduled at or before its deadline. The greedy algorithm processes jobs in
decreasing profit order and places each chosen job in the latest available slot before its deadline.

The latest-slot choice is safe because it uses the current high-profit job while leaving earlier
slots available for jobs with tighter deadlines. A disjoint-set forest over time slots makes this
efficient: `find_root(t)` returns the latest still-free slot at or before `t`, and occupying slot
`t` links it to the next candidate slot `t - 1`.

- `select_deadline_jobs(jobs)` returns a pair $(`profit`, `slot`)$ containing that maximum profit
  and `slot[i]`, the assigned time slot of input job `i`, or $-1$ if that job is not selected, for a
  vector of `Job` with fields `deadline` and `profit`. Deadlines are positive integer time slots.

Time Complexity:
- O(n log n) per call due to sorting, with near-constant disjoint-set operations.

Space Complexity:
- O(n) auxiliary.

*/

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

struct Job {
  int deadline;
  int64_t profit;
};

class SlotDSU {
  std::vector<int> root;

 public:
  explicit SlotDSU(int n) : root(n + 1) { std::iota(root.begin(), root.end(), 0); }

  int find_root(int u) {
    if (root[u] != u) {
      root[u] = find_root(root[u]);
    }
    return root[u];
  }

  void occupy(int u) { root[u] = find_root(u - 1); }
};

std::pair<int64_t, std::vector<int>> select_deadline_jobs(const std::vector<Job> &jobs) {
  int n = static_cast<int>(jobs.size());
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    return jobs[i].profit != jobs[j].profit ? jobs[i].profit > jobs[j].profit
                                            : jobs[i].deadline < jobs[j].deadline;
  });
  int max_deadline = 0;
  for (const auto &j : jobs) {
    max_deadline = std::max(max_deadline, j.deadline);
  }
  max_deadline = std::min(max_deadline, n);
  SlotDSU slots(max_deadline);
  std::vector<int> assigned(n, -1);
  int64_t res = 0;
  for (int i : order) {
    if (jobs[i].profit <= 0) {
      continue;
    }
    int slot = slots.find_root(std::min(jobs[i].deadline, max_deadline));
    if (slot > 0) {
      res += jobs[i].profit;
      assigned[i] = slot;
      slots.occupy(slot);
    }
  }
  return {res, assigned};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Job> jobs{{2, 100}, {1, 19}, {2, 27}, {1, 25}, {3, 15}};
  auto [profit, slot] = select_deadline_jobs(jobs);
  // Jobs 0, 2, 4 fill slots 2, 1, 3 for total profit 100 + 27 + 15.
  assert(profit == 142);
  assert((slot == vector<int>{2, -1, 1, -1, 3}));
  return 0;
}
