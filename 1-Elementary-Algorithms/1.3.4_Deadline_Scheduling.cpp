/*

Schedules unit-time jobs with deadlines and profits to maximize total profit. Each job takes one
time slot and must be scheduled at or before its deadline. The greedy algorithm processes jobs in
decreasing profit order and places each chosen job in the latest available slot before its deadline.

This is a compact scheduling pattern that combines sorting with a disjoint-set forest over available
time slots.

- `schedule_deadline_jobs(jobs)` returns the maximum total profit.
- `job_t` stores `deadline` and `profit`. Deadlines are positive integer time slots.

Time Complexity:
- O(n log n) per call due to sorting, with near-constant disjoint-set operations.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <vector>

struct job_t {
  int deadline;
  long long profit;

  job_t(int deadline = 0, long long profit = 0) : deadline(deadline), profit(profit) {}
};

bool higher_profit(const job_t &a, const job_t &b) {
  return a.profit != b.profit ? a.profit > b.profit : a.deadline < b.deadline;
}

class slot_dsu {
  std::vector<int> root;

 public:
  explicit slot_dsu(int n) : root(n + 1) {
    for (int i = 0; i <= n; i++) {
      root[i] = i;
    }
  }

  int find_root(int u) {
    if (root[u] != u) {
      root[u] = find_root(root[u]);
    }
    return root[u];
  }

  void occupy(int u) { root[u] = find_root(u - 1); }
};

long long schedule_deadline_jobs(std::vector<job_t> jobs) {
  std::sort(jobs.begin(), jobs.end(), higher_profit);
  int max_deadline = 0;
  for (int i = 0; i < (int)jobs.size(); i++) {
    max_deadline = std::max(max_deadline, jobs[i].deadline);
  }
  slot_dsu slots(max_deadline);
  long long res = 0;
  for (int i = 0; i < (int)jobs.size(); i++) {
    int slot = slots.find_root(jobs[i].deadline);
    if (slot > 0) {
      res += jobs[i].profit;
      slots.occupy(slot);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<job_t> jobs;
  jobs.push_back(job_t(2, 100));
  jobs.push_back(job_t(1, 19));
  jobs.push_back(job_t(2, 27));
  jobs.push_back(job_t(1, 25));
  jobs.push_back(job_t(3, 15));
  assert(schedule_deadline_jobs(jobs) == 142);
  return 0;
}
