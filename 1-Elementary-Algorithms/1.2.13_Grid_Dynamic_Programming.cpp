/*

Solves basic dynamic programming problems on a rectangular grid. In grid DP, each cell represents a
state and transitions usually come from already-computed neighboring cells, most commonly the cell
above and the cell to the left. Processing rows from top to bottom and columns from left to right
therefore makes every predecessor available when a state is computed.

For path-counting, `dp[r][c]` stores the number of ways to reach cell $(`r`, `c`)$. For minimum-cost
paths, `dp[r][c]` stores the cheapest cost to reach that cell, and a predecessor direction
reconstructs one optimal path. The helpers below assume movement only down or right.

- `count_grid_paths(blocked)` returns the number of paths from the upper-left cell to the
  lower-right cell, moving only down or right and avoiding blocked cells marked nonzero.
- `min_grid_path_cost(cost)` returns a pair $(`sum`, `parent`)$, where `sum` is the minimum cost of
  a down/right path from the upper-left cell to the lower-right cell and `parent[r][c]` is `'U'` or
  `'L'`, indicating the predecessor direction for one optimal path. If the grid is empty, `sum` is
  $0$ and `parent` is empty.

Time Complexity:
- O(R*C) per call to `count_grid_paths(blocked)` and `min_grid_path_cost(cost)`, where $R$ and $C$
  are the grid dimensions.

Space Complexity:
- O(R*C) auxiliary for `count_grid_paths(blocked)` and `min_grid_path_cost(cost)`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

int64_t count_grid_paths(const std::vector<std::vector<char>> &blocked) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  for (int r = 0; r < rows; r++) {
    assert(static_cast<int>(blocked[r].size()) == cols);
  }
  if (rows == 0 || cols == 0 || blocked[0][0] || blocked[rows - 1][cols - 1]) {
    return 0;
  }
  std::vector<std::vector<int64_t>> dp(rows, std::vector<int64_t>(cols, 0));
  dp[0][0] = 1;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (blocked[r][c]) {
        dp[r][c] = 0;
        continue;
      }
      if (r > 0) {
        dp[r][c] += dp[r - 1][c];
      }
      if (c > 0) {
        dp[r][c] += dp[r][c - 1];
      }
    }
  }
  return dp[rows - 1][cols - 1];
}

std::pair<int64_t, std::vector<std::vector<char>>> min_grid_path_cost(
    const std::vector<std::vector<int>> &cost
) {
  int rows = static_cast<int>(cost.size());
  int cols = rows == 0 ? 0 : static_cast<int>(cost[0].size());
  if (rows == 0 || cols == 0) {
    return {0, {}};
  }
  for (int r = 0; r < rows; r++) {
    assert(static_cast<int>(cost[r].size()) == cols);
  }
  const int64_t INF = std::numeric_limits<int64_t>::max() / 4;
  std::vector<std::vector<int64_t>> dp(rows, std::vector<int64_t>(cols, INF));
  std::vector<std::vector<char>> parent(rows, std::vector<char>(cols, 0));
  dp[0][0] = cost[0][0];
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (r > 0 && dp[r][c] > dp[r - 1][c] + cost[r][c]) {
        dp[r][c] = dp[r - 1][c] + cost[r][c];
        parent[r][c] = 'U';
      }
      if (c > 0 && dp[r][c] > dp[r][c - 1] + cost[r][c]) {
        dp[r][c] = dp[r][c - 1] + cost[r][c];
        parent[r][c] = 'L';
      }
    }
  }
  return {dp[rows - 1][cols - 1], parent};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<char>> blocked{
      {0, 0, 0},
      {0, 1, 0},
      {0, 0, 0},
  };
  assert(count_grid_paths(blocked) == 2);

  vector<vector<int>> cost{
      {1, 3, 1},
      {1, 5, 1},
      {4, 2, 1},
  };
  auto [sum, parent] = min_grid_path_cost(cost);
  assert(sum == 7);

  vector<pair<int, int>> path;
  for (int r = 2, c = 2; r || c; parent[r][c] == 'U' ? --r : --c) {
    path.emplace_back(r, c);
  }
  path.emplace_back(0, 0);
  reverse(path.begin(), path.end());
  assert((path == vector<pair<int, int>>{{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}}));
  return 0;
}
