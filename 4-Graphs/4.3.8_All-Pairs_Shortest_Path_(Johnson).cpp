/*

Given a sparse, weighted directed graph with possibly negative edge weights, determine the minimum
distance between every pair of nodes. Johnson's algorithm first computes a potential $h(v)$ using
Bellman-Ford, then replaces each edge weight $w(u,v)$ with $w'(u,v) = w(u,v) + h(u) - h(v)$. Every
reduced weight is nonnegative, so Dijkstra's algorithm can run from every node. Restoring the
original distance afterward preserves all shortest paths because the potential terms telescope along
each path.

Initializing every potential to $0$ is equivalent to adding a virtual source with zero-weight edges
to all nodes. Bellman-Ford therefore detects a negative cycle anywhere in the graph, not only one
reachable from a particular original node. If such a cycle exists, shortest paths are undefined.

- `johnson_all_pairs(n)` populates `dist` and `next_node` for a global, pre-populated edge list
  `edges` whose nodes are numbered $[0, `n`)$. Each edge is stored as (`u`, `v`, `weight`).
  Afterward, `dist[u][v]` is the shortest distance from $u$ to $v$, or `INF` if $v$ is unreachable
  from $u$. An error is thrown if the graph contains a negative-weight cycle.
- `get_path(start, dest)` returns the shortest path from `start` to `dest`, or an empty vector if
  `dest` is unreachable from `start`, after the latest call to `johnson_all_pairs()`.

For path reconstruction, `next_node[u][v]` stores the first node after `u` on a shortest path to
`v`, or $-1$ if `v` is unreachable from `u`. Repeatedly advance `u` to `next_node[u][v]` until it
equals `v`.

All potentials, reduced weights, and path distances must fit in `int64_t`.

Time Complexity:
- O(n*m + n*(n + m)*log n) per call, where $n$ is the number of nodes and $m$ is the number of
  edges. For a sparse connected graph, this is O(n*m*log n).
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(n^2 + max(n, m)) for the graph, distance matrix, and auxiliary storage.
- O(p) for the path returned by `get_path()`.

*/

#include <cstdint>
#include <functional>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::tuple<int, int, int>> edges;  // (u, v, weight)
std::vector<std::vector<int64_t>> dist;
std::vector<std::vector<int>> next_node;

void johnson_all_pairs(int n) {
  std::vector<int64_t> potential(n, 0);
  for (int i = 0; i < n; i++) {
    bool changed = false;
    for (auto [u, v, w] : edges) {
      int64_t candidate = potential[u] + w;  // Overflow warning.
      if (candidate < potential[v]) {
        if (i == n - 1) {
          throw std::runtime_error("Negative-weight cycle found.");
        }
        potential[v] = candidate;
        changed = true;
      }
    }
    if (!changed) {
      break;
    }
  }
  std::vector<std::vector<std::pair<int, int64_t>>> adj(n);
  for (auto [u, v, w] : edges) {
    int64_t reduced = static_cast<int64_t>(w) + potential[u] - potential[v];  // Overflow warning.
    adj[u].emplace_back(v, reduced);
  }
  dist.assign(n, std::vector<int64_t>(n, INF));
  next_node.assign(n, std::vector<int>(n, -1));
  for (int start = 0; start < n; start++) {
    std::priority_queue<
        std::pair<int64_t, int>, std::vector<std::pair<int64_t, int>>, std::greater<>>
        pq;
    dist[start][start] = 0;
    next_node[start][start] = start;
    pq.emplace(0, start);
    while (!pq.empty()) {
      auto [du, u] = pq.top();
      pq.pop();
      if (du != dist[start][u]) {
        continue;
      }
      for (auto [v, w] : adj[u]) {
        int64_t candidate = du + w;  // Overflow warning.
        if (candidate < dist[start][v]) {
          dist[start][v] = candidate;
          next_node[start][v] = (u == start) ? v : next_node[start][u];
          pq.emplace(candidate, v);
        }
      }
    }
    for (int v = 0; v < n; v++) {
      if (dist[start][v] != INF) {
        dist[start][v] += potential[v] - potential[start];  // Overflow warning.
      }
    }
  }
}

std::vector<int> get_path(int start, int dest) {
  if (next_node[start][dest] == -1) {
    return {};
  }
  std::vector<int> path{start};
  while (start != dest) {
    start = next_node[start][dest];
    path.push_back(start);
  }
  return path;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //                      w=3
  //            +--------------------+
  //            |                    |
  //     w=1    v   w=-2       w=2   |
  // 0 -------> 1 -------> 2 ------> 3
  // |                     ^
  // +---------------------+
  //         w=4
  edges = {{0, 1, 1}, {0, 2, 4}, {1, 2, -2}, {2, 3, 2}, {3, 1, 3}};
  johnson_all_pairs(5);  // Node 4 is isolated.
  assert((dist[0] == vector<int64_t>{0, 1, -1, 1, INF}));
  assert((dist[1] == vector<int64_t>{INF, 0, -2, 0, INF}));
  assert((dist[2] == vector<int64_t>{INF, 5, 0, 2, INF}));
  assert((dist[3] == vector<int64_t>{INF, 3, 1, 0, INF}));
  assert((dist[4] == vector<int64_t>{INF, INF, INF, INF, 0}));
  assert((get_path(0, 3) == vector<int>{0, 1, 2, 3}));
  assert(get_path(0, 4).empty());

  edges = {{0, 1, -1}, {1, 0, -1}};
  bool threw = false;
  try {
    johnson_all_pairs(2);
  } catch (const runtime_error &) {
    threw = true;
  }
  assert(threw);
  return 0;
}
