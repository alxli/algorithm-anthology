/*

Schedules unit-time jobs with deadlines and profits to maximize total profit. Each job takes one
time slot and must be scheduled at or before its deadline. The greedy algorithm processes jobs in
decreasing profit order and places each chosen job in the latest available slot before its deadline.

This is a compact scheduling pattern that combines sorting with a disjoint-set forest over available
time slots.

- `schedule_deadline_jobs(jobs)` returns the maximum total profit for a vector of `Job` with fields
  `deadline` and `profit`. Deadlines are positive integer time slots.

Time Complexity:
- O(n log n) per call due to sorting, with near-constant disjoint-set operations.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <cstdint>
#include <numeric>
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

int64_t schedule_deadline_jobs(std::vector<Job> jobs) {
  std::sort(jobs.begin(), jobs.end(), [](const Job &a, const Job &b) {
    return a.profit != b.profit ? a.profit > b.profit : a.deadline < b.deadline;
  });
  int max_deadline = 0;
  for (const auto &j : jobs) {
    max_deadline = std::max(max_deadline, j.deadline);
  }
  SlotDSU slots(max_deadline);
  int64_t res = 0;
  for (const auto &j : jobs) {
    int slot = slots.find_root(j.deadline);
    if (slot > 0) {
      res += j.profit;
      slots.occupy(slot);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Job> jobs{{2, 100}, {1, 19}, {2, 27}, {1, 25}, {3, 15}};
  assert(schedule_deadline_jobs(jobs) == 142);
  return 0;
}
