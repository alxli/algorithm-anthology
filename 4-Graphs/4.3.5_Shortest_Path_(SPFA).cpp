/*

Given a starting node in a weighted directed graph, compute shortest paths even when some edge
weights are negative. The Shortest Path Faster Algorithm (SPFA) is a queue-based optimization of
Bellman-Ford: instead of relaxing every edge in every round, it keeps a queue of nodes whose
distances have improved and relaxes only their outgoing edges. It is often fast on benign inputs,
but it still has Bellman-Ford's worst-case behavior and can be forced to run in O(n*m). Prefer
Dijkstra for nonnegative weights, and use SPFA mainly when negative edges are present and the input
is not adversarial.

- `spfa(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj` which
  uses its indices as nodes. Each edge is stored as (`neighbor`, `weight`). The function returns
  `false` if it detects a reachable negative cycle, and returns `true` otherwise.
- `get_path(dest)` returns the path from `start` to `dest`, or an empty vector if `dest` is
  unreachable, provided the most recent call to `spfa()` returned true. If it returned false, a
  reachable negative-weight cycle leaves the distances and paths undefined.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n*m) per call in the worst case, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(p) per call to `get_path()`, where $p$ is the number of nodes in the returned path.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary queue space.
- O(p) for the path returned by `get_path()`.

*/

#include <algorithm>
#include <cstdint>
#include <queue>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int64_t> dist;
std::vector<int> pred;

bool spfa(int start) {
  int n = static_cast<int>(adj.size());
  dist.assign(n, INF);
  pred.assign(n, -1);
  std::vector<int> path_edges(n);
  std::vector<char> in_queue(n);
  std::queue<int> q;
  dist[start] = 0;
  q.push(start);
  in_queue[start] = true;
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    in_queue[u] = false;
    for (auto [v, w] : adj[u]) {
      if (dist[v] > dist[u] + w) {  // Overflow warning.
        dist[v] = dist[u] + w;
        pred[v] = u;
        path_edges[v] = path_edges[u] + 1;
        if (path_edges[v] >= n) {
          return false;
        }
        if (!in_queue[v]) {
          q.push(v);
          in_queue[v] = true;
        }
      }
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
  //        w=4
  //     0 ----> 1
  //     |      /
  // w=5 |    / w=-2
  //     |  /
  //     v v   w=3
  //     2 ---------> 3
  adj.assign(4, {});
  adj[0].emplace_back(1, 4);
  adj[0].emplace_back(2, 5);
  adj[1].emplace_back(2, -2);
  adj[2].emplace_back(3, 3);
  spfa(0);
  assert((dist == vector<int64_t>{0, 4, 2, 5}) && (pred == vector<int>{-1, 0, 1, 2}));
  assert((get_path(3) == vector<int>{0, 1, 2, 3}));

  adj[3].emplace_back(1, -2);  // The cycle 1 -> 2 -> 3 -> 1 now has total weight -1.
  assert(!spfa(0));
  return 0;
}
