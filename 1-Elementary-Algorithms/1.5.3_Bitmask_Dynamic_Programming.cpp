/*

Bitmask dynamic programming uses a bitmask to represent a small set or frontier, then treats that
mask as the DP state. This is useful when one dimension is small enough for $2^n$ states: assignment
problems can store which jobs are already used, set cover can store which elements are covered,
submask DP can split a set into groups, and grid tiling can store which cells on the current
frontier are already occupied.

For assignment, `dp[mask]` is the minimum cost after assigning the first `popcount(mask)` workers to
the selected jobs. The next worker tries every unused job. For set cover, `dp[mask]` is the minimum
number of chosen sets needed to cover exactly the elements in `mask`; adding one set moves to the
union mask. For set partitioning, iterating all submasks gives the recurrence
`dp[mask] = min(dp[mask ^ sub] + cost[sub])` over all masks this has O(3^n) transitions. For
domino tiling, the mask is a profile of the current row; this technique is often called DP on a
broken profile or plug DP. A domino covers exactly two edge-adjacent cells, so each piece is either
horizontal ($1 \times 2$) or vertical ($2 \times 1$).

- `assignment_min_cost(cost)` returns a pair `(sum, job)`, where `sum` is the minimum cost of
  assigning each worker to a distinct job, and `job[i]` is the job assigned to worker `i`.
- `minimum_set_cover(sets, universe_size)` returns a pair `(count, chosen)`, where `count` is the
  minimum number of sets needed to cover all elements from $0$ to `universe_size - 1`, and `chosen`
  contains one optimal list of set indices. If no cover exists, `count` is $-1$ and `chosen` is
  empty. Each input set is represented as a bitmask.
- `partition_min_cost(group_cost)` returns the minimum total cost to partition all elements into
  disjoint nonempty groups, where `group_cost[mask]` is the cost of taking `mask` as one group.
- `count_domino_tilings(rows, cols)` returns the number of ways to tile a `rows` by `cols` rectangle
  with $1 \times 2$ and $2 \times 1$ dominoes.

Time Complexity:
- O(n^2*2^n) per call to `assignment_min_cost()`, where $n$ is the number of workers and jobs.
- O(s*2^u) per call to `minimum_set_cover()`, where $s$ is the number of sets and $u$ is
  `universe_size`.
- O(3^n) per call to `partition_min_cost()`, where $n$ is the number of elements.
- O(r*c*2^c) per call to `count_domino_tilings()`, where $r$ and $c$ are the number of rows and
  columns, respectively.

Space Complexity:
- O(2^n + n) auxiliary for `assignment_min_cost()`.
- O(2^u + s) auxiliary for `minimum_set_cover()`.
- O(2^n) auxiliary for `partition_min_cost()`.
- O(2^c) auxiliary for `count_domino_tilings()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> assignment_min_cost(
    const std::vector<std::vector<int64_t>> &cost
) {
  int n = static_cast<int>(cost.size());
  assert(n < 31);
  for (const auto &row : cost) {
    assert(static_cast<int>(row.size()) == n);
  }
  int states = 1 << n;
  int64_t inf = std::numeric_limits<int64_t>::max() / 4;
  std::vector<int64_t> dp(states, inf);
  std::vector<int> parent(states, -1);
  dp[0] = 0;
  for (int mask = 0; mask < states; mask++) {
    if (dp[mask] == inf) {
      continue;
    }
    int worker = __builtin_popcount(static_cast<unsigned>(mask));
    if (worker == n) {
      continue;
    }
    for (int job = 0; job < n; job++) {
      if ((mask & (1 << job)) == 0 && dp[mask | (1 << job)] > dp[mask] + cost[worker][job]) {
        dp[mask | (1 << job)] = dp[mask] + cost[worker][job];
        parent[mask | (1 << job)] = job;
      }
    }
  }
  std::vector<int> job(n);
  for (int mask = states - 1, worker = n - 1; worker >= 0; worker--) {
    job[worker] = parent[mask];
    mask ^= 1 << job[worker];
  }
  return {dp[states - 1], job};
}

std::pair<int, std::vector<int>> minimum_set_cover(
    const std::vector<int> &sets, int universe_size
) {
  assert(0 <= universe_size && universe_size < 31);
  int n = static_cast<int>(sets.size());
  int states = 1 << universe_size;
  int full = states - 1;
  for (int set : sets) {
    assert((set & ~full) == 0);
  }
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
  std::vector<int> chosen;
  for (int mask = full; mask != 0; mask = prev[mask]) {
    int i = parent[mask];
    chosen.push_back(i);
  }
  std::reverse(chosen.begin(), chosen.end());
  return {dp[full], chosen};
}

int64_t partition_min_cost(const std::vector<int64_t> &group_cost) {
  int states = static_cast<int>(group_cost.size());
  assert(states > 0 && (states & (states - 1)) == 0);
  int64_t inf = std::numeric_limits<int64_t>::max() / 4;
  std::vector<int64_t> dp(states, inf);
  dp[0] = 0;
  for (int mask = 1; mask < states; mask++) {
    for (int sub = mask; sub > 0; sub = (sub - 1) & mask) {
      dp[mask] = std::min(dp[mask], dp[mask ^ sub] + group_cost[sub]);
    }
  }
  return dp[states - 1];
}

int64_t count_domino_tilings(int rows, int cols) {
  assert(rows >= 0 && cols >= 0);
  if (rows < cols) {
    return count_domino_tilings(cols, rows);
  }
  assert(cols < 31);
  int states = 1 << cols;
  std::vector<int64_t> dp(states, 0), next(states, 0);
  dp[0] = 1;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      std::fill(next.begin(), next.end(), 0);
      for (int mask = 0; mask < states; mask++) {
        if (dp[mask] == 0) {
          continue;
        }
        if (mask & (1 << c)) {
          next[mask ^ (1 << c)] += dp[mask];
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

int main() {
  std::vector<std::vector<int64_t>> cost{{9, 2, 7}, {6, 4, 3}, {5, 8, 1}};
  auto [assignment_cost, job] = assignment_min_cost(cost);
  assert(assignment_cost == 9);
  assert((job == std::vector<int>{1, 0, 2}));

  std::vector<int> sets{0b0011, 0b0110, 0b1100, 0b1000};
  auto [cover_count, chosen] = minimum_set_cover(sets, 4);
  assert(cover_count == 2);
  assert((chosen == std::vector<int>{0, 2}));

  auto [impossible_count, impossible_chosen] = minimum_set_cover(std::vector<int>{0b001, 0b010}, 3);
  assert(impossible_count == -1 && impossible_chosen.empty());

  std::vector<int64_t> group_cost{
      0,        // Empty group is unused.
      4, 6, 7,  // Singletons.
      6, 2, 3,  // Pairs.
      9,        // All three together.
  };
  assert(partition_min_cost(group_cost) == 7);  // Groups {0} and {1, 2}.

  assert(count_domino_tilings(0, 5) == 1);
  assert(count_domino_tilings(2, 3) == 3);
  assert(count_domino_tilings(3, 3) == 0);
  assert(count_domino_tilings(4, 4) == 36);
  return 0;
}
