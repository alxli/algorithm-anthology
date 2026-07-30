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
  from $u$. The function returns whether the graph contains no negative-weight cycle.
- `get_path(start, dest)` returns the shortest path from `start` to `dest`, or an empty vector if
  `dest` is unreachable from `start`, provided the most recent call to `johnson_all_pairs()`
  returned true.

For path reconstruction, `next_node[u][v]` stores the first node after `u` on a shortest path to
`v`, or $-1$ if `v` is unreachable from `u`. Repeatedly advance `u` to `next_node[u][v]` until it
equals `v`.

Overflow warning: All potentials, reduced weights, and path distances must fit in `int64_t`.

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
#include <tuple>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::tuple<int, int, int>> edges;  // (u, v, weight)
std::vector<std::vector<int64_t>> dist;
std::vector<std::vector<int>> next_node;

bool johnson_all_pairs(int n) {
  std::vector<int64_t> potential(n, 0);
  for (int i = 0; i < n; i++) {
    bool changed = false;
    for (auto [u, v, w] : edges) {
      int64_t pv = potential[u] + w;  // Overflow warning.
      if (pv < potential[v]) {
        if (i == n - 1) {
          return false;
        }
        potential[v] = pv;
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
  using qnode = std::pair<int64_t, int>;  // (distance, node)
  for (int start = 0; start < n; start++) {
    std::priority_queue<qnode, std::vector<qnode>, std::greater<>> pq;
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
        int64_t dv = du + w;  // Overflow warning.
        if (dv < dist[start][v]) {
          dist[start][v] = dv;
          next_node[start][v] = (u == start) ? v : next_node[start][u];
          pq.emplace(dv, v);
        }
      }
    }
    for (int v = 0; v < n; v++) {
      if (dist[start][v] != INF) {
        dist[start][v] += potential[v] - potential[start];  // Overflow warning.
      }
    }
  }
  return true;
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
  bool ok = johnson_all_pairs(5);  // Node 4 is isolated.
  assert(ok);
  assert((dist[0] == vector<int64_t>{0, 1, -1, 1, INF}));
  assert((dist[1] == vector<int64_t>{INF, 0, -2, 0, INF}));
  assert((dist[2] == vector<int64_t>{INF, 5, 0, 2, INF}));
  assert((dist[3] == vector<int64_t>{INF, 3, 1, 0, INF}));
  assert((dist[4] == vector<int64_t>{INF, INF, INF, INF, 0}));
  assert((get_path(0, 3) == vector<int>{0, 1, 2, 3}));
  assert(get_path(0, 4).empty());

  edges = {{0, 1, -1}, {1, 0, -1}};
  ok = johnson_all_pairs(2);
  assert(!ok);
  return 0;
}
