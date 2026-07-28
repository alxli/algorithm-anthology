/*

Given a starting node in a weighted, directed graph with possibly negative weights, visit every
reachable node and determine the minimum distance to each such node. Optionally, reconstruct the
shortest path to a destination node using the shortest-path tree from the predecessor array `pred`.

Bellman-Ford relaxes every edge in the graph up to $n - 1$ times, stopping early if a pass makes no
changes. Since any shortest path uses at most $n - 1$ edges, all distances are correct after these
passes unless a negative-weight cycle keeps reducing them, which a further pass detects.
Bellman-Ford will also detect whether the graph contains a negative-weight cycle reachable from the
start node, in which case the affected shortest paths are undefined. (To detect a negative cycle
anywhere in the graph, add a virtual source with zero-weight edges to every node and start from it.)

- `bellman_ford(n, start)` populates `dist` and `pred` for a global, pre-populated edge list `edges`
  whose endpoints must be numbered $[0, `n`)$, and returns whether no reachable negative-weight
  cycle was found.
- `get_path(dest)` returns the path from `start` to `dest`, or an empty vector if `dest` is
  unreachable, after the latest successful call to `bellman_ford()`.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n*m) per call, where $n$ is the number of nodes and $m$ is the number of edges.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <cstdint>
#include <tuple>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::tuple<int, int, int>> edges;  // (u, v, w)
std::vector<int64_t> dist;
std::vector<int> pred;

bool bellman_ford(int n, int start) {
  dist.assign(n, INF);
  pred.assign(n, -1);
  dist[start] = 0;
  for (int i = 0; i < n - 1; i++) {
    bool changed = false;
    for (auto [u, v, w] : edges) {
      // The dist[u] != INF guard avoids relaxing out of unreachable nodes: a negative edge from an
      // unreachable u would otherwise give v a bogus finite distance (INF + w < INF).
      if (dist[u] != INF && dist[v] > dist[u] + w) {
        dist[v] = dist[u] + w;
        pred[v] = u;
        changed = true;
      }
    }
    if (!changed) break;
  }
  // Check for a negative-weight cycle reachable from the start node.
  for (auto [u, v, w] : edges) {
    if (dist[u] != INF && dist[v] > dist[u] + w) {
      return false;
    }
  }
  return true;
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
using namespace std;

int main() {
  //    w=1      w=2
  // 0 -----> 1 -----> 2
  // |                 ^
  // +-----------------+
  //         w=5
  edges.emplace_back(0, 1, 1);
  edges.emplace_back(1, 2, 2);
  edges.emplace_back(0, 2, 5);
  int start = 0, dest = 2;
  bool ok = bellman_ford(3, start);
  assert(ok);
  assert(dist[dest] == 3);
  assert((get_path(dest) == vector<int>{0, 1, 2}));

  edges = {{0, 1, -1}, {1, 0, -1}};
  ok = bellman_ford(2, 0);
  assert(!ok);
  return 0;
}
