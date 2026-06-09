/*

Given a starting node in a weighted, directed acyclic graph (DAG), determine the minimum distance to
every reachable node. Because the graph is acyclic, its nodes admit a topological ordering in which
every edge points forward. Relaxing edges in this order settles each node's distance in a single
pass, with no priority queue and no restriction on edge signs. This makes it both faster than
Dijkstra's algorithm and able to handle negative edge weights, as long as the graph has no cycles.
The same pass computes longest paths if the relaxation comparison is reversed, which is the standard
way to find the critical path in a schedule of dependent tasks.

- `dag_shortest_path(start)` populates `dist` and `pred` for a global, pre-populated adjacency list
  `adj` whose nodes are numbered from 0 to `adj.size() - 1`. Each edge is stored as
  `(neighbor, weight)` and may have any sign. `dist[v]` is set to `INF` for nodes not reachable from
  `start`, and `pred` stores the shortest-path tree for path reconstruction.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n + m) per call to `dag_shortest_path()`, where $n$ is the number of nodes and $m$ is the number
  of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph.
- O(n) auxiliary heap space for `dag_shortest_path()`.

*/

#include <algorithm>
#include <climits>
#include <utility>
#include <vector>

const long long INF = LLONG_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<long long> dist;
std::vector<int> pred;

void topological_dfs(int u, std::vector<bool> &visit, std::vector<int> &order) {
  visit[u] = true;
  for (auto &[v, w] : adj[u]) {
    if (!visit[v]) {
      topological_dfs(v, visit, order);
    }
  }
  order.push_back(u);
}

void dag_shortest_path(int start) {
  int nodes = adj.size();
  std::vector<bool> visit(nodes, false);
  std::vector<int> order;
  for (int i = 0; i < nodes; i++) {
    if (!visit[i]) {
      topological_dfs(i, visit, order);
    }
  }
  std::reverse(order.begin(), order.end());
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  dist[start] = 0;
  for (int u : order) {
    if (dist[u] == INF) {
      continue;
    }
    for (auto &[v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        pred[v] = u;
      }
    }
  }
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  adj.assign(5, {});
  adj[0] = {{1, 1}, {2, 5}};
  adj[1] = {{2, 1}, {3, -2}};  // A negative edge, which Dijkstra could not handle.
  adj[2] = {{3, 1}};
  adj[3] = {{4, 3}};

  dag_shortest_path(0);
  assert((dist == vector<long long>{0, 1, 2, -1, 2}));
  assert(pred[3] == 1 && pred[4] == 3);
  return 0;
}
