/*

Traverses a rectangular grid as an implicit unweighted graph: each unblocked cell is a node joined
to its unblocked (four-way by default) orthogonal neighbors. Depth-first flood fill finds connected
regions, while breadth-first search finds shortest distances because every move has unit length.
Seeding the BFS with several cells computes the distance to the nearest source in one pass.

- `inside(r, c, rows, cols)` returns whether $0 \leq `r` < `rows`$ and $0 \leq `c` < `cols`$.
- `adj4(r, c)` returns the four orthogonally adjacent coordinates without filtering those outside
  the grid. For eight-way movement, also include the four diagonal neighbors.
- `component_area_perimeter(blocked, r, c)` uses flood fill to return the area and perimeter of the
  component containing (`r`, `c`). If the starting cell is blocked, both values are $0$.
- `grid_components(blocked)` uses repeated flood fills to return the connected components of cells
  marked zero, with each cell represented by a pair (`r`, `c`). Movement is allowed in the four
  orthogonal directions.
- `grid_bfs(blocked, sources)` returns the pair (`dist`, `pred`) from every cell in `sources`. Each
  source must be an unblocked cell represented by a pair (`r`, `c`). Passing one source gives
  ordinary single-source BFS. Blocked and unreachable cells have distance $-1$.
- `get_path(pred, tr, tc)` uses the predecessor grid returned by `grid_bfs()` to return the path
  from a nearest source to (`tr`, `tc`), including both endpoints, or an empty vector if the target
  is blocked or unreachable.

Time Complexity:
- O(R*C) per call to `component_area_perimeter()`, `grid_components()`, or `grid_bfs()`, where $R$
  and $C$ are the grid dimensions.
- O(p) per call to `get_path()`, where $p$ is the number of cells in the returned path.

Space Complexity:
- O(R*C) auxiliary, including the recursion stack used by `component_area_perimeter()` and
  `grid_components()`.
- O(R*C) for the returned `dist` and `pred`, and O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <array>
#include <cassert>
#include <queue>
#include <utility>
#include <vector>

bool inside(int r, int c, int rows, int cols) {
  return 0 <= r && r < rows && 0 <= c && c < cols;
}

std::array<std::pair<int, int>, 4> adj4(int r, int c) {
  return {{{r - 1, c}, {r + 1, c}, {r, c - 1}, {r, c + 1}}};
}

template<typename Fn>
void grid_dfs(
    const std::vector<std::vector<char>> &blocked, int r, int c,
    std::vector<std::vector<char>> &visit, const Fn &f
) {
  int rows = static_cast<int>(blocked.size());
  int cols = static_cast<int>(blocked[0].size());
  visit[r][c] = true;
  f(r, c);
  for (auto [r2, c2] : adj4(r, c)) {
    if (inside(r2, c2, rows, cols) && !blocked[r2][c2] && !visit[r2][c2]) {
      grid_dfs(blocked, r2, c2, visit, f);
    }
  }
}

std::pair<int, int> component_area_perimeter(
    const std::vector<std::vector<char>> &blocked, int r, int c
) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  assert(inside(r, c, rows, cols));
  if (blocked[r][c]) {
    return {0, 0};
  }
  int area = 0, perimeter = 0;
  std::vector<std::vector<char>> visit(rows, std::vector<char>(cols));
  grid_dfs(blocked, r, c, visit, [&](int r, int c) {
    area++;
    for (auto [r2, c2] : adj4(r, c)) {
      perimeter += !inside(r2, c2, rows, cols) || blocked[r2][c2];
    }
  });
  return {area, perimeter};
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
      components.emplace_back();
      grid_dfs(blocked, r, c, visit, [&](int r, int c) { components.back().emplace_back(r, c); });
    }
  }
  return components;
}

auto grid_bfs(
    const std::vector<std::vector<char>> &blocked, const std::vector<std::pair<int, int>> &sources
) {
  int rows = static_cast<int>(blocked.size());
  int cols = rows == 0 ? 0 : static_cast<int>(blocked[0].size());
  std::vector<std::vector<int>> dist(rows, std::vector<int>(cols, -1));
  std::vector<std::vector<std::pair<int, int>>> pred(
      rows, std::vector<std::pair<int, int>>(cols, {-1, -1})
  );
  std::queue<std::pair<int, int>> q;
  for (auto [r, c] : sources) {
    assert(inside(r, c, rows, cols) && !blocked[r][c]);
    if (dist[r][c] == -1) {
      dist[r][c] = 0;
      pred[r][c] = {r, c};
      q.emplace(r, c);
    }
  }
  while (!q.empty()) {
    auto [r, c] = q.front();
    q.pop();
    for (auto [r2, c2] : adj4(r, c)) {
      if (inside(r2, c2, rows, cols) && !blocked[r2][c2] && dist[r2][c2] == -1) {
        dist[r2][c2] = dist[r][c] + 1;
        pred[r2][c2] = {r, c};
        q.emplace(r2, c2);
      }
    }
  }
  return std::pair{std::move(dist), std::move(pred)};
}

std::vector<std::pair<int, int>> get_path(
    const std::vector<std::vector<std::pair<int, int>>> &pred, int tr, int tc
) {
  int rows = static_cast<int>(pred.size());
  int cols = rows == 0 ? 0 : static_cast<int>(pred[0].size());
  assert(inside(tr, tc, rows, cols));
  if (pred[tr][tc].first == -1) {
    return {};
  }
  std::vector<std::pair<int, int>> path;
  std::pair cur{tr, tc};
  while (pred[cur.first][cur.second] != cur) {
    path.push_back(cur);
    cur = pred[cur.first][cur.second];
  }
  path.push_back(cur);
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
  assert((component_area_perimeter(blocked, 0, 0) == pair{5, 12}));
  assert((component_area_perimeter(blocked, 0, 2) == pair{0, 0}));

  auto [dist, pred] = grid_bfs(blocked, {{0, 0}, {2, 3}});
  assert((dist == vector<vector<int>>{{0, 1, -1, 2}, {-1, 2, -1, 1}, {4, 3, -1, 0}}));
  assert((get_path(pred, 2, 1) == vector<pair<int, int>>{{0, 0}, {0, 1}, {1, 1}, {2, 1}}));
  assert((get_path(pred, 0, 3) == vector<pair<int, int>>{{2, 3}, {1, 3}, {0, 3}}));
  assert(get_path(pred, 0, 2).empty());
  return 0;
}
