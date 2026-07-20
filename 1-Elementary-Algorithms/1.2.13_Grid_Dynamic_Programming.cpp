/*

Solves basic dynamic programming problems on a rectangular grid. In grid DP, each cell represents a
state and transitions usually come from already-computed neighboring cells, most commonly the cell
above and the cell to the left. Processing rows from top to bottom and columns from left to right
therefore makes every predecessor available when a state is computed.

For path-counting, `dp[r][c]` stores the number of ways to reach cell $(r, c)$. For minimum-cost
paths, `dp[r][c]` stores the cheapest cost to reach that cell, and a predecessor direction
reconstructs one optimal path. The helpers below assume movement only down or right.

- `count_grid_paths(blocked)` returns the number of paths from the upper-left cell to the
  lower-right cell, moving only down or right and avoiding blocked cells marked nonzero.
- `min_grid_path_cost(cost)` returns a pair $(`sum`, `path`)$ containing the minimum cost of a
  down/right path from the upper-left cell to the lower-right cell and the cells of one optimal path
  in order. If the grid is empty, `sum` is $0$ and `path` is empty.

Path counts and path costs must fit in `int64_t`.

Time Complexity:
- O(R*C) per call to `count_grid_paths(blocked)` and `min_grid_path_cost(cost)`, where $R$ and $C$
  are the grid dimensions.

Space Complexity:
- O(C) auxiliary for `count_grid_paths(blocked)`.
- O(R*C) auxiliary and O(R + C) for the returned path from `min_grid_path_cost(cost)`.

*/

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

int64_t count_grid_paths(const std::vector<std::vector<char>> &blocked) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  if (rows == 0 || cols == 0 || blocked[0][0] || blocked[rows - 1][cols - 1]) {
    return 0;
  }
  std::vector<int64_t> dp(cols, 0);
  dp[0] = 1;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (blocked[r][c]) {
        dp[c] = 0;
      } else if (c > 0) {
        dp[c] += dp[c - 1];  // Overflow warning.
      }
    }
  }
  return dp[cols - 1];
}

std::pair<int64_t, std::vector<std::pair<int, int>>> min_grid_path_cost(
    const std::vector<std::vector<int>> &cost
) {
  int rows = static_cast<int>(cost.size());
  int cols = rows == 0 ? 0 : static_cast<int>(cost[0].size());
  if (rows == 0 || cols == 0) {
    return {0, {}};
  }
  std::vector<std::vector<int64_t>> dp(rows, std::vector<int64_t>(cols));
  std::vector<std::vector<char>> parent(rows, std::vector<char>(cols, 0));
  dp[0][0] = cost[0][0];
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (r == 0 && c == 0) {
        continue;
      }
      if (c == 0 || (r > 0 && dp[r - 1][c] <= dp[r][c - 1])) {
        dp[r][c] = dp[r - 1][c] + cost[r][c];  // Overflow warning.
        parent[r][c] = 'U';
      } else {
        dp[r][c] = dp[r][c - 1] + cost[r][c];
        parent[r][c] = 'L';
      }
    }
  }
  std::vector<std::pair<int, int>> path;
  for (int r = rows - 1, c = cols - 1;;) {
    path.emplace_back(r, c);
    if (r == 0 && c == 0) {
      break;
    }
    if (parent[r][c] == 'U') {
      r--;
    } else {
      c--;
    }
  }
  std::reverse(path.begin(), path.end());
  return {dp[rows - 1][cols - 1], path};
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
  auto [sum, path] = min_grid_path_cost(cost);
  assert(sum == 7);
  assert((path == vector<pair<int, int>>{{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}}));
  return 0;
}
