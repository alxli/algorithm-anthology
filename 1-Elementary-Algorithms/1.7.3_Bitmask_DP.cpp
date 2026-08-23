/*

Bitmask dynamic programming uses a bitmask to represent a small set or frontier, then treats that
mask as the DP state. This is useful when one dimension is small enough for $2^n$ states.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

/*

The assignment problem matches workers one-to-one with jobs while minimizing the total cost. The
workers are processed in a fixed order: `dp[mask]` is the minimum cost after assigning the first
`popcount(mask)` workers to the selected jobs, and the next worker tries every unused job.

- `min_cost_assignment(cost)` returns a pair (`sum`, `job`), where `sum` is the minimum cost of
  assigning each worker to a distinct job, and `job[i]` is the job assigned to worker `i`. The input
  is a square matrix with one row per worker and one column per job.

Overflow warning: The accumulated cost must fit in `int64_t`.

Time Complexity:
- O(n*2^n) per call to `min_cost_assignment()`, where $n$ is the number of workers and jobs.

Space Complexity:
- O(2^n) auxiliary and O(n) for the returned assignment from `min_cost_assignment()`.

*/

std::pair<int64_t, std::vector<int>> min_cost_assignment(
    const std::vector<std::vector<int64_t>> &cost
) {
  int n = static_cast<int>(cost.size());
  assert(n < 31);
  int states = 1 << n;
  std::vector<int64_t> dp(states);
  std::vector<int> parent(states, -1);
  for (int mask = 0; mask < states; mask++) {
    int worker = __builtin_popcount(static_cast<unsigned>(mask));
    if (worker == n) {
      continue;
    }
    for (int job = 0; job < n; job++) {
      if ((mask & (1 << job)) == 0) {
        int next = mask | (1 << job);
        int64_t candidate = dp[mask] + cost[worker][job];  // Overflow warning.
        if (parent[next] == -1 || dp[next] > candidate) {
          dp[next] = candidate;
          parent[next] = job;
        }
      }
    }
  }
  // Optional: reconstruct one optimal assignment.
  std::vector<int> job(n);
  for (int mask = states - 1, worker = n - 1; worker >= 0; worker--) {
    job[worker] = parent[mask];
    mask ^= 1 << job[worker];
  }
  return {dp[states - 1], job};
}

/*

The minimum set cover problem asks for the fewest input sets whose union contains every element of a
fixed universe. Both input sets and covered subsets are represented as masks: `dp[mask]` is the
minimum number of chosen sets needed to cover exactly the elements in `mask`, and adding one set
moves to the union mask.

- `set_cover(sets, universe_size)` returns a pair (`count`, `chosen`), where `count` is the minimum
  number of sets needed to cover all elements in $[0, u)$, where $u$ is `universe_size`, and
  `chosen` contains one optimal list of set indices. If no cover exists, `count` is $-1$ and
  `chosen` is empty. Each input set is represented as a bitmask.

Time Complexity:
- O(s*2^u) per call, where $s$ is the number of sets and $u$ is `universe_size`.

Space Complexity:
- O(2^u) auxiliary and O(s) for the returned indices.

*/

std::pair<int, std::vector<int>> set_cover(const std::vector<int> &sets, int universe_size) {
  assert(0 <= universe_size && universe_size < 31);
  int states = 1 << universe_size;
  int full = states - 1;
  assert(std::all_of(sets.begin(), sets.end(), [full](int subset) {
    return (subset & ~full) == 0;
  }));
  int n = static_cast<int>(sets.size());
  int inf = n + 1;
  std::vector<int> dp(states, inf), parent(states, -1), prev(states, -1);
  dp[0] = 0;
  for (int mask = 0; mask < states; mask++) {
    if (dp[mask] == inf) {
      continue;
    }
    for (int i = 0; i < n; i++) {
      int next = mask | sets[i];
      if (dp[next] > dp[mask] + 1) {
        dp[next] = dp[mask] + 1;
        parent[next] = i;
        prev[next] = mask;
      }
    }
  }
  if (dp[full] == inf) {
    return {-1, {}};
  }
  // Optional: reconstruct one minimum set cover.
  std::vector<int> chosen;
  for (int mask = full; mask != 0; mask = prev[mask]) {
    int i = parent[mask];
    chosen.push_back(i);
  }
  std::reverse(chosen.begin(), chosen.end());
  return {dp[full], chosen};
}

/*

The minimum-cost set partition problem divides all elements into disjoint nonempty groups when the
cost of every possible group is known. For each set `mask`, choosing a submask as one group leaves
`mask ^ sub` to be partitioned, giving the recurrence `dp[mask] = min(dp[mask ^ sub] + cost[sub])`.
Over all masks, this has O(3^n) transitions.

- `min_cost_partition(group_cost)` returns the minimum total cost to partition all elements into
  disjoint nonempty groups, where `group_cost[mask]` is the cost of taking `mask` as one group.

Overflow warning: The accumulated cost must fit in `int64_t`.

Time Complexity:
- O(3^n) per call, where $n$ is the number of elements.

Space Complexity:
- O(2^n) auxiliary.

*/

int64_t min_cost_partition(const std::vector<int64_t> &group_cost) {
  int states = static_cast<int>(group_cost.size());
  assert(states > 0 && (states & (states - 1)) == 0);
  std::vector<int64_t> dp(states);
  for (int mask = 1; mask < states; mask++) {
    dp[mask] = group_cost[mask];
    for (int sub = (mask - 1) & mask; sub > 0; sub = (sub - 1) & mask) {
      int64_t candidate = dp[mask ^ sub] + group_cost[sub];  // Overflow warning.
      dp[mask] = std::min(dp[mask], candidate);
    }
  }
  return dp[states - 1];
}

/*

Domino tiling asks for the number of ways to cover a rectangle with horizontal ($1 \times 2$) and
vertical ($2 \times 1$) dominoes. Processing cells in row-major order, a profile mask records which
cells on the current frontier are already occupied. An occupied current cell is cleared; otherwise,
placing a horizontal domino marks the next column, while placing a vertical one carries the current
column into the next row. This technique is often called DP on a broken profile or plug DP.

- `count_domino_tilings(rows, cols)` returns the number of ways to tile a `rows` by `cols` rectangle
  with $1 \times 2$ and $2 \times 1$ dominoes.

Overflow warning: The tiling count must fit in `int64_t`.

Time Complexity:
- O(R*C*2^C) per call, where $R$ and $C$ are the number of rows and columns, respectively.

Space Complexity:
- O(2^C) auxiliary.

*/

int64_t count_domino_tilings(int rows, int cols) {
  assert(rows >= 0 && cols >= 0);
  if (rows == 0 || cols == 0) {
    return 1;
  }
  if (rows < cols) {
    std::swap(rows, cols);
  }
  assert(cols < 31);
  int states = 1 << cols;
  std::vector<int64_t> dp(states), next(states);
  dp[0] = 1;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      next.assign(states, 0);
      for (int mask = 0; mask < states; mask++) {
        if (dp[mask] == 0) {
          continue;
        }
        if (mask & (1 << c)) {
          next[mask ^ (1 << c)] += dp[mask];  // Overflow warning.
        } else {
          if (c + 1 < cols && (mask & (1 << (c + 1))) == 0) {
            next[mask | (1 << (c + 1))] += dp[mask];
          }
          if (r + 1 < rows) {
            next[mask | (1 << c)] += dp[mask];
          }
        }
      }
      dp.swap(next);
    }
  }
  return dp[0];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<int64_t>> cost{{9, 2, 7}, {6, 4, 3}, {5, 8, 1}};
  auto [assignment_cost, job] = min_cost_assignment(cost);
  assert(assignment_cost == 9 && (job == vector<int>{1, 0, 2}));

  auto [cover_count, chosen] = set_cover(vector<int>{0b0011, 0b0110, 0b1100, 0b1000}, 4);
  assert(cover_count == 2 && (chosen == vector<int>{0, 2}));

  auto [impossible_count, impossible_chosen] = set_cover(vector<int>{0b001, 0b010}, 3);
  assert(impossible_count == -1 && impossible_chosen.empty());

  vector<int64_t> group_cost{
      0,        // Empty group is unused.
      4, 6, 7,  // Singletons.
      6, 2, 3,  // Pairs.
      9,        // All three together.
  };
  assert(min_cost_partition(group_cost) == 7);  // Groups {0} and {1, 2}.

  assert(count_domino_tilings(0, 1000000000) == 1);
  assert(count_domino_tilings(2, 3) == 3);
  assert(count_domino_tilings(3, 3) == 0);
  assert(count_domino_tilings(4, 4) == 36);
  return 0;
}
