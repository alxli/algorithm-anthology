/*

Schedules unit-time jobs with deadlines and profits to maximize total profit. Each job takes one
time slot and must be scheduled at or before its deadline. The greedy algorithm processes jobs in
decreasing profit order and places each chosen job in the latest available slot before its deadline.

This is a compact scheduling pattern that combines sorting with a disjoint-set forest over available
time slots.

- `schedule_deadline_jobs(jobs)` returns the maximum total profit.
- `Job` stores `deadline` and `profit`. Deadlines are positive integer time slots.

Time Complexity:
- O(n log n) per call due to sorting, with near-constant disjoint-set operations.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <vector>

struct Job {
  int deadline;
  long long profit;

  Job(int deadline = 0, long long profit = 0) : deadline(deadline), profit(profit) {}
};

bool higher_profit(const Job &a, const Job &b) {
  return a.profit != b.profit ? a.profit > b.profit : a.deadline < b.deadline;
}

class SlotDSU {
  std::vector<int> root;

 public:
  explicit SlotDSU(int n) : root(n + 1) {
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

long long schedule_deadline_jobs(std::vector<Job> jobs) {
  std::sort(jobs.begin(), jobs.end(), higher_profit);
  int max_deadline = 0;
  for (int i = 0; i < static_cast<int>(jobs.size()); i++) {
    max_deadline = std::max(max_deadline, jobs[i].deadline);
  }
  SlotDSU slots(max_deadline);
  long long res = 0;
  for (int i = 0; i < static_cast<int>(jobs.size()); i++) {
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
  vector<Job> jobs;
  jobs.push_back(Job(2, 100));
  jobs.push_back(Job(1, 19));
  jobs.push_back(Job(2, 27));
  jobs.push_back(Job(1, 25));
  jobs.push_back(Job(3, 15));
  assert(schedule_deadline_jobs(jobs) == 142);
  return 0;
}
