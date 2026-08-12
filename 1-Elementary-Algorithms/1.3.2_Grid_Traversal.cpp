/*

Traverses a rectangular grid as an implicit unweighted graph: each unblocked cell is a node joined
to its unblocked four-way orthogonal neighbors. Depth-first flood fill finds connected regions,
while breadth-first search finds shortest distances because every move has unit length. Seeding the
BFS with several cells computes the distance to the nearest source in one pass.

- `flood_fill(blocked, r, c)` returns all unblocked cells reachable from starting cell $(`r`, `c`)$,
  which must lie within the grid. If it is blocked, the returned vector is empty; otherwise, its
  size is the area of the containing component.
- `grid_components(blocked)` returns the connected components of cells marked zero, with each cell
  represented by a pair $(`r`, `c`)$. Movement is allowed in the four orthogonal directions.
- `grid_distances(blocked, sources)` returns an $R$ by $C$ matrix of shortest distances from the
  nearest source. Blocked and unreachable cells contain $-1$. Every source must be an unblocked cell
  represented by a pair $(`r`, `c`)$. Passing one source gives ordinary single-source shortest
  paths.
- `shortest_grid_path(blocked, source, target)` returns one shortest path from the unblocked cell
  `source` to `target`, including both endpoints, or an empty vector if `target` is blocked or
  unreachable. Both endpoints must lie within the grid.

Time Complexity:
- O(R*C) per call, where $R$ and $C$ are the grid dimensions.

Space Complexity:
- O(R*C) auxiliary, including the recursion stack used by `grid_components()` and O(R*C) for the
  returned result.

*/

#include <algorithm>
#include <cassert>
#include <queue>
#include <utility>
#include <vector>

void flood_fill_dfs(
    const std::vector<std::vector<char>> &blocked, int r, int c,
    std::vector<std::vector<char>> &visit, std::vector<std::pair<int, int>> &component
) {
  int rows = static_cast<int>(blocked.size());
  int cols = static_cast<int>(blocked[0].size());
  visit[r][c] = true;
  component.push_back({r, c});
  for (auto [r2, c2] : {std::pair{r - 1, c}, {r + 1, c}, {r, c - 1}, {r, c + 1}}) {
    if (0 <= r2 && r2 < rows && 0 <= c2 && c2 < cols && !blocked[r2][c2] && !visit[r2][c2]) {
      flood_fill_dfs(blocked, r2, c2, visit, component);
    }
  }
}

std::vector<std::pair<int, int>> flood_fill(
    const std::vector<std::vector<char>> &blocked, int r, int c
) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  assert(0 <= r && r < rows && 0 <= c && c < cols);
  if (blocked[r][c]) {
    return {};
  }
  std::vector<std::vector<char>> visit(rows, std::vector<char>(cols));
  std::vector<std::pair<int, int>> component;
  flood_fill_dfs(blocked, r, c, visit, component);
  return component;
}

std::vector<std::vector<std::pair<int, int>>> grid_components(
    const std::vector<std::vector<char>> &blocked
) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  std::vector<std::vector<std::pair<int, int>>> components;
  std::vector<std::vector<char>> visit(rows, std::vector<char>(cols));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (blocked[r][c] || visit[r][c]) {
        continue;
      }
      components.push_back({});
      flood_fill_dfs(blocked, r, c, visit, components.back());
    }
  }
  return components;
}

std::vector<std::vector<int>> grid_distances(
    const std::vector<std::vector<char>> &blocked, const std::vector<std::pair<int, int>> &sources
) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, -1));
  std::queue<std::pair<int, int>> q;
  for (auto [r, c] : sources) {
    assert(0 <= r && r < rows && 0 <= c && c < cols && !blocked[r][c]);
    if (dist[r][c] == -1) {
      dist[r][c] = 0;
      q.push({r, c});
    }
  }
  while (!q.empty()) {
    auto [r, c] = q.front();
    q.pop();
    for (auto [r2, c2] : {std::pair{r - 1, c}, {r + 1, c}, {r, c - 1}, {r, c + 1}}) {
      if (0 <= r2 && r2 < rows && 0 <= c2 && c2 < cols && !blocked[r2][c2] && dist[r2][c2] == -1) {
        dist[r2][c2] = dist[r][c] + 1;
        q.push({r2, c2});
      }
    }
  }
  return dist;
}

std::vector<std::pair<int, int>> shortest_grid_path(
    const std::vector<std::vector<char>> &blocked, std::pair<int, int> source,
    std::pair<int, int> target
) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  auto [sr, sc] = source;
  auto [r, c] = target;
  assert(0 <= sr && sr < rows && 0 <= sc && sc < cols && !blocked[sr][sc]);
  assert(0 <= r && r < rows && 0 <= c && c < cols);
  if (blocked[r][c]) {
    return {};
  }
  auto dist = grid_distances(blocked, {source});
  if (dist[r][c] == -1) {
    return {};
  }
  std::vector<std::pair<int, int>> path;
  while (!(r == sr && c == sc)) {
    path.push_back({r, c});
    for (auto [r2, c2] : {std::pair{r - 1, c}, {r + 1, c}, {r, c - 1}, {r, c + 1}}) {
      if (0 <= r2 && r2 < rows && 0 <= c2 && c2 < cols && dist[r2][c2] == dist[r][c] - 1) {
        r = r2;
        c = c2;
        break;
      }
    }
  }
  path.push_back(source);
  std::reverse(path.begin(), path.end());
  return path;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<char>> blocked{
      {0, 0, 1, 0},
      {1, 0, 1, 0},
      {0, 0, 1, 0},
  };
  auto components = grid_components(blocked);
  assert(components.size() == 2);
  assert(components[0].size() == 5 && components[1].size() == 3);
  assert(flood_fill(blocked, 0, 0).size() == 5);
  assert(flood_fill(blocked, 0, 2).empty());

  auto dist = grid_distances(blocked, {{0, 0}, {2, 3}});
  assert((dist == vector<vector<int>>{{0, 1, -1, 2}, {-1, 2, -1, 1}, {4, 3, -1, 0}}));
  assert(
      (shortest_grid_path(blocked, {0, 0}, {2, 1}) ==
       vector<pair<int, int>>{{0, 0}, {0, 1}, {1, 1}, {2, 1}})
  );
  assert(shortest_grid_path(blocked, {0, 0}, {0, 3}).empty());
  assert(shortest_grid_path(blocked, {0, 0}, {0, 2}).empty());
  return 0;
}
