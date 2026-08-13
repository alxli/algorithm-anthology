/*

Solves basic dynamic programming problems on a rectangular grid. In grid DP, each cell represents a
state and transitions usually come from already-computed neighboring cells. Processing rows from top
to bottom and columns from left to right makes predecessors above and to the left available when a
state is computed.

- `count_grid_paths(blocked)` returns the number of paths from the upper-left cell to the
  lower-right cell, moving only down or right and avoiding blocked cells marked nonzero. The state
  $dp(r, c)$ counts paths to cell $(r, c)$ from the counts above and left. Only the current row is
  retained.
- `min_cost_grid_path(cost)` returns a pair (`min_cost`, `path`) containing the minimum cost of a
  down/right path from the upper-left cell to the lower-right cell and the cells of one optimal path
  in order. The state $dp(r, c)$ stores the cheapest cost to cell $(r, c)$. Only the current row of
  costs is retained, while predecessor directions reconstruct one optimal path. If the grid is
  empty, `min_cost` is $0$ and `path` is empty.
- `largest_one_square(a)` returns a tuple (`side`, `top`, `left`) describing a largest all-nonzero
  square in `a`. The state $dp(c)$ is the largest square ending at the current cell in column $c$; a
  nonzero cell extends the minimum of the states above, left, and diagonally above-left. If no such
  square exists, the function returns $(0, -1, -1)$.

Overflow warning: Path counts and path costs must fit in `int64_t`.

Time Complexity:
- O(R*C) per call, where $R$ and $C$ are the grid dimensions.

Space Complexity:
- O(C) auxiliary for `count_grid_paths(blocked)`.
- O(R*C) auxiliary for predecessor directions, O(C) for path costs, and O(R + C) for the returned
  path from `min_cost_grid_path(cost)`.
- O(C) auxiliary for `largest_one_square(a)`.

*/

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <utility>
#include <vector>

int64_t count_grid_paths(const std::vector<std::vector<char>> &blocked) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  if (rows == 0 || cols == 0 || blocked[0][0] || blocked[rows - 1][cols - 1]) {
    return 0;
  }
  std::vector<int64_t> dp(cols);
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

std::pair<int64_t, std::vector<std::pair<int, int>>> min_cost_grid_path(
    const std::vector<std::vector<int>> &cost
) {
  int rows = static_cast<int>(cost.size());
  int cols = rows == 0 ? 0 : static_cast<int>(cost[0].size());
  if (rows == 0 || cols == 0) {
    return {0, {}};
  }
  std::vector<int64_t> dp(cols);
  std::vector<std::vector<char>> parent(rows, std::vector<char>(cols));
  dp[0] = cost[0][0];
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (r == 0 && c == 0) {
        continue;
      }
      if (r > 0 && (c == 0 || dp[c] <= dp[c - 1])) {
        dp[c] += cost[r][c];  // Overflow warning.
        parent[r][c] = 'U';
      } else {
        dp[c] = dp[c - 1] + cost[r][c];
        parent[r][c] = 'L';
      }
    }
  }
  // Optional: reconstruct one optimal path.
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
  return {dp.back(), path};
}

std::tuple<int, int, int> largest_one_square(const std::vector<std::vector<char>> &a) {
  int rows = static_cast<int>(a.size()), cols = rows == 0 ? 0 : static_cast<int>(a[0].size());
  std::vector<int> dp(cols);
  int best = 0, top = -1, left = -1;
  for (int r = 0; r < rows; r++) {
    int diagonal = 0;
    for (int c = 0; c < cols; c++) {
      int above = dp[c];
      dp[c] = a[r][c] ? 1 + std::min({above, c == 0 ? 0 : dp[c - 1], diagonal}) : 0;
      diagonal = above;
      if (dp[c] > best) {
        best = dp[c];
        top = r - best + 1;
        left = c - best + 1;
      }
    }
  }
  return {best, top, left};
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
  auto [min_cost, path] = min_cost_grid_path(cost);
  assert(min_cost == 7);
  assert((path == vector<pair<int, int>>{{0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}}));

  vector<vector<char>> binary{
      {1, 0, 1, 0, 0},
      {1, 0, 1, 1, 1},
      {1, 1, 1, 1, 1},
      {1, 0, 1, 1, 1},
  };
  assert((largest_one_square(binary) == tuple{3, 1, 2}));
  assert((largest_one_square({{0, 0}}) == tuple{0, -1, -1}));
  return 0;
}
