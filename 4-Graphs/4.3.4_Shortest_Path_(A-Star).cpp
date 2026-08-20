/*

Given a starting node and a destination node in a weighted, directed graph with nonnegative weights
only, determine the minimum distance between them, using a heuristic estimate of the remaining
distance to steer the search toward the destination. Optionally, reconstruct the shortest path using
the predecessor array `pred`.

A* is Dijkstra's algorithm with the priority queue keyed by $f(v) = g(v) + h(v)$, where $g(v)$ is
the best known distance from `start` to `v` and $h(v)$ estimates the remaining distance to `dest`.
Nodes whose estimated total already exceeds the answer are never expanded, so an informed heuristic
visits far fewer nodes than Dijkstra's algorithm while returning the same distance. The heuristic
must be admissible, never exceeding the true remaining distance, which guarantees that no shortest
path is discarded and that the first removal of `dest` from the queue yields its final distance.

A heuristic is consistent when $h(u) \leq w(u, v) + h(v)$ for every edge $(u, v)$ and
$h(`dest`) = 0$. Consistency implies admissibility and makes every distance final on first removal,
exactly as in Dijkstra's algorithm; an admissible but inconsistent heuristic may improve a node
after it has been expanded, which the stale-entry check below handles by re-expanding it. Setting
$h(v) = 0$ satisfies both and reduces this to Dijkstra's algorithm, so the heuristic is what buys
the speedup: use the Manhattan distance on a unit-weight grid, the Euclidean distance scaled by the
minimum edge weight in a geometric graph, or the number of misplaced tiles in a sliding puzzle.

- `astar(start, dest, h)` returns the minimum distance from `start` to `dest`, or `INF` if `dest` is
  unreachable, for a global, pre-populated adjacency list `adj` whose indices represent the nodes.
  Each edge is stored as (`neighbor`, `weight`), where `weight` is nonnegative. The heuristic `h(v)`
  must return an admissible numeric estimate of the remaining distance from node `v` to `dest`;
  integral and floating-point estimates are both supported.
- `get_path(dest)` returns the path from `start` to `dest`, or an empty vector if `dest` is
  unreachable, using the state left by the most recent call to `astar()`.

Since the search stops as soon as `dest` is expanded, only `dist[dest]` and the `pred` chain back to
`start` are final; other entries may remain tentative, so use Dijkstra's algorithm in section 4.3.3
when every distance is needed.

Time Complexity:
- O(n + m log n) per call in the worst case, where $n$ is the number of nodes and $m$ is the number
  of edges, plus one call to `h()` per queue insertion. A more informed heuristic lowers the
  constant by expanding fewer nodes, but does not improve the worst case.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(max(n, m)) auxiliary.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <cstdint>
#include <functional>
#include <queue>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int64_t> dist;
std::vector<int> pred;

template<typename Fn>
int64_t astar(int start, int dest, Fn h) {
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
  dist[start] = 0;
  using Cost = decltype(int64_t{} + h(0));
  using qnode = std::tuple<Cost, int64_t, int>;  // (estimated total, distance when queued, node)
  std::priority_queue<qnode, std::vector<qnode>, std::greater<>> pq;
  pq.emplace(Cost(h(start)), 0, start);
  while (!pq.empty()) {
    int64_t queued_dist = std::get<1>(pq.top());
    int u = std::get<2>(pq.top());
    pq.pop();
    if (queued_dist != dist[u]) {
      continue;  // Stale entry, left behind by a later improvement to dist[u].
    }
    if (u == dest) {
      return dist[u];
    }
    for (auto [v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        pred[v] = u;
        pq.emplace(Cost(dist[v]) + Cost(h(v)), dist[v], v);
      }
    }
  }
  return INF;
}

std::vector<int> get_path(int dest) {
  if (dist[dest] == INF) {
    return {};
  }
  std::vector<int> path;
  for (int v = dest; v != -1; v = pred[v]) {
    path.push_back(v);
  }
  std::reverse(path.begin(), path.end());
  return path;
}

/*** Example Usage ***/

#include <cassert>
#include <cstdlib>
using namespace std;

int main() {
  // A 3-by-3 grid of unit-weight edges with the center cell walled off, plus an
  // isolated node 9. Cell (r, c) is node 3*r + c.
  //   0 - 1 - 2
  //   |       |
  //   3       5
  //   |       |
  //   6 - 7 - 8
  const int R = 3, C = 3;
  adj.assign(R * C + 1, {});
  auto walled = [](int r, int c) { return r == 1 && c == 1; };
  auto add_edge = [](int u, int v) {
    adj[u].emplace_back(v, 1);
    adj[v].emplace_back(u, 1);
  };
  for (int r = 0; r < R; r++) {
    for (int c = 0; c < C; c++) {
      if (walled(r, c)) {
        continue;
      }
      if (r + 1 < R && !walled(r + 1, c)) {
        add_edge(r * C + c, (r + 1) * C + c);
      }
      if (c + 1 < C && !walled(r, c + 1)) {
        add_edge(r * C + c, r * C + c + 1);
      }
    }
  }

  int start = 0, dest = R * C - 1;
  auto manhattan = [=](int u) { return abs(u / C - (R - 1)) + abs(u % C - (C - 1)); };
  assert(astar(start, dest, manhattan) == 4);
  assert((get_path(dest) == vector<int>{0, 1, 2, 5, 8}));

  // A zero heuristic is admissible and consistent, making this Dijkstra's algorithm.
  auto zero = [](int) { return 0; };
  assert(astar(start, dest, zero) == 4);
  assert((get_path(dest) == vector<int>{0, 1, 2, 5, 8}));

  // Fractional admissible heuristics retain their priority instead of being truncated to int64_t.
  auto half_manhattan = [=](int u) { return manhattan(u) / 2.0; };
  assert(astar(start, dest, half_manhattan) == 4);

  assert(astar(start, R * C, zero) == INF);
  assert(get_path(R * C).empty());
  return 0;
}
