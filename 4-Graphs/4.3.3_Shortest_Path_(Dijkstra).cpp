/*

Given a starting node in a weighted, directed graph with nonnegative weights only, visit every
reachable node and determine the minimum distance to each such node. Optionally, reconstruct the
shortest path to a destination node using the shortest-path tree from the predecessor array `pred`.

Dijkstra's algorithm repeatedly selects the unvisited node of smallest tentative distance using a
priority queue and relaxes its outgoing edges. Dijkstra's algorithm requires nonnegative edge
weights. Use Bellman-Ford or SPFA instead when negative edges are present. Because the weights are
nonnegative, a node's distance is final the first time it is removed from the queue.

- `dijkstra(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj`
  whose indices represent the nodes. Each edge is stored as (`neighbor`, `weight`), where `weight`
  is nonnegative.
- `get_path(dest)` returns the path from `start` to `dest`, or an empty vector if `dest` is
  unreachable, using the state left by the most recent call to `dijkstra()`.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n + m log n) per call, where $n$ is the number of nodes and $m$ is the number of edges.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(max(n, m)) auxiliary.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int64_t> dist;
std::vector<int> pred;

void dijkstra(int start) {
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
  dist[start] = 0;
  using qnode = std::pair<int64_t, int>;  // (distance, node)
  std::priority_queue<qnode, std::vector<qnode>, std::greater<>> pq;
  pq.emplace(0, start);
  while (!pq.empty()) {
    auto [du, u] = pq.top();
    pq.pop();
    if (du != dist[u]) {
      continue;
    }
    for (auto [v, w] : adj[u]) {
      if (dist[v] > dist[u] + w) {  // Overflow warning.
        dist[v] = dist[u] + w;
        pred[v] = u;
        pq.emplace(dist[v], v);
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
  //     w=2     w=2
  //  0 ----> 1 ----> 2
  //   \      |      /
  // w=8\  w=4|     /w=1
  //     \    v    /
  //      --> 3 <--
  adj.assign(4, {});
  adj[0].emplace_back(1, 2);
  adj[0].emplace_back(3, 8);
  adj[1].emplace_back(2, 2);
  adj[1].emplace_back(3, 4);
  adj[2].emplace_back(3, 1);
  int start = 0, dest = 3;
  dijkstra(start);
  assert(dist[dest] == 5);
  assert((get_path(dest) == vector<int>{0, 1, 2, 3}));
  return 0;
}
