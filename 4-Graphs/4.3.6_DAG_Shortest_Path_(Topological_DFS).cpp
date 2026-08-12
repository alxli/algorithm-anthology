/*

Given a starting node in a weighted, directed acyclic graph (DAG), determine the minimum distance to
every reachable node. Because the graph is acyclic, its nodes admit a topological ordering in which
every edge points forward. Relaxing edges in this order settles each node's distance in a single
pass, with no priority queue and no restriction on edge signs. This makes it both faster than
Dijkstra's algorithm and able to handle negative edge weights, as long as the graph has no cycles.
The same pass computes longest paths if the relaxation comparison is reversed, which is the standard
way to find the critical path in a schedule of dependent tasks.

- `dag_shortest_path(start)` populates `dist` and `pred` for a global, pre-populated adjacency list
  `adj` whose indices represent the nodes. Each edge is stored as (`neighbor`, `weight`) and may
  have any sign. `dist[v]` is set to `INF` for nodes not reachable from `start`, and `pred` stores
  the shortest-path tree for path reconstruction.
- `get_path(dest)` returns the path from `start` to `dest`, or an empty vector if `dest` is
  unreachable, using the state left by the most recent call to `dag_shortest_path()`.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n + m) per call, where $n$ is the number of nodes and $m$ is the number of edges.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph.
- O(n) auxiliary.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int64_t> dist;
std::vector<int> pred;

void dag_shortest_path(int start) {
  int n = static_cast<int>(adj.size());
  std::vector<char> visit(n);
  std::vector<int> order;
  auto dfs = [&](auto &&dfs, int u) -> void {
    visit[u] = true;
    for (auto [v, w] : adj[u]) {
      if (!visit[v]) {
        dfs(dfs, v);
      }
    }
    order.push_back(u);
  };
  for (int i = 0; i < n; i++) {
    if (!visit[i]) {
      dfs(dfs, i);
    }
  }
  std::reverse(order.begin(), order.end());
  dist.assign(n, INF);
  pred.assign(n, -1);
  dist[start] = 0;
  for (int u : order) {
    if (dist[u] == INF) {
      continue;
    }
    for (auto [v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {  // Overflow warning.
        dist[v] = dist[u] + w;
        pred[v] = u;
      }
    }
  }
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
  //        w=1    w=-2     w=3
  //     0 ----> 1 ----> 3 -----> 4
  //     |     /        ^
  // w=5 |   / w=1     /
  //     v v          /w=1
  //     2 ----------+
  adj.assign(5, {});
  adj[0].emplace_back(1, 1);
  adj[0].emplace_back(2, 5);
  adj[1].emplace_back(2, 1);
  adj[1].emplace_back(3, -2);  // A negative edge, which Dijkstra could not handle.
  adj[2].emplace_back(3, 1);
  adj[3].emplace_back(4, 3);
  dag_shortest_path(0);
  assert((dist == vector<int64_t>{0, 1, 2, -1, 2}));
  assert((pred == vector<int>{-1, 0, 1, 1, 3}));
  assert((get_path(4) == vector<int>{0, 1, 3, 4}));
  return 0;
}
