/*

Given jobs that must run one at a time on a single machine, each with a processing time and a
weight, order them to minimize the weighted sum of completion times $\sum w_j C_j$, where $C_j$ is
the moment job $j$ finishes. Unlike the deadline problems of sections 1.5.6 and 1.5.7, nothing is
rejected and no deadline is missed; every order is feasible, and only the total cost differs.

Smith's rule sorts by the ratio of processing time to weight, running the job with the smallest
$p_j / w_j$ first. An exchange argument proves it optimal. Consider any schedule that runs job $b$
immediately before job $a$ with $p_a / w_a < p_b / w_b$, and swap the two. Every other job finishes
at the same moment, since the pair occupies the same total time, so the change in cost is
$p_a w_b - p_b w_a$, which the ratio inequality makes negative. Any schedule violating the ratio
order therefore improves under a swap, so an optimal schedule must be sorted by it. With equal
weights the rule reduces to shortest processing time first, the classic way to minimize the average
completion time.

- `min_weighted_completion_time(jobs)` returns the pair (`cost`, `order`), where `cost` is the
  minimum weighted sum of completion times and `order` lists the job indices in an optimal sequence.
  Each job is a pair (`time`, `weight`) with a positive weight and a nonnegative time.

Ties break by index, and the comparison cross-multiplies rather than divides so that it stays exact
on integers. Parallel machines are a different problem, NP-hard for general weights.

Overflow warning: the products in the comparator and the running completion time are accumulated in
`int64_t`, so the total processing time multiplied by the total weight must fit in that type.

Time Complexity:
- O(n log n) per call, where $n$ is the number of jobs.

Space Complexity:
- O(n) auxiliary and O(n) for the returned order.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> min_weighted_completion_time(
    const std::vector<std::pair<int, int>> &jobs
) {
  assert(std::all_of(jobs.begin(), jobs.end(), [](auto job) {
    return job.first >= 0 && job.second > 0;
  }));
  int n = static_cast<int>(jobs.size());
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int a, int b) {
    // Sort by time / weight ascending, cross-multiplied to stay exact.
    int64_t lhs = static_cast<int64_t>(jobs[a].first) * jobs[b].second;
    int64_t rhs = static_cast<int64_t>(jobs[b].first) * jobs[a].second;
    return lhs != rhs ? lhs < rhs : a < b;
  });
  int64_t clock = 0, cost = 0;
  for (int id : order) {
    clock += jobs[id].first;
    cost += clock * jobs[id].second;  // Overflow warning.
  }
  return {cost, order};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // (time, weight) pairs. Ratios are 3, 1, and 2, so job 1 runs first, then job 2, then job 0.
  auto [cost, order] = min_weighted_completion_time({{3, 1}, {2, 2}, {4, 2}});
  assert((order == vector<int>{1, 2, 0}));
  assert(cost == 2 * 2 + 6 * 2 + 9 * 1);  // Completion times 2, 6, and 9.

  // Equal weights reduce the rule to shortest processing time first.
  auto [spt_cost, spt_order] = min_weighted_completion_time({{5, 1}, {1, 1}, {3, 1}});
  assert((spt_order == vector<int>{1, 2, 0}));
  assert(spt_cost == 1 + 4 + 9);

  assert(min_weighted_completion_time({}).first == 0);
  return 0;
}
