/*

Given a starting node in a weighted, directed acyclic graph (DAG), determine the minimum distance to
every reachable node. Because the graph is acyclic, its nodes admit a topological ordering in which
every edge points forward. Relaxing edges in this order settles each node's distance in a single
pass, with no priority queue and no restriction on edge signs. This makes it both faster than
Dijkstra's algorithm and able to handle negative edge weights, as long as the graph has no cycles.
The same pass computes longest paths if the relaxation comparison is reversed, which is the standard
way to find the critical path in a schedule of dependent tasks.

- `dag_shortest_path(start)` populates `dist` and `pred` for a global, pre-populated adjacency list
  `adj` whose nodes are numbered $[0, `n`)$, where `n` is `adj.size()`. Each edge is stored as
  $(`neighbor`, `weight`)$ and may have any sign. `dist[v]` is set to `INF` for nodes not reachable
  from `start`, and `pred` stores the shortest-path tree for path reconstruction.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n + m) per call, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph.
- O(n) auxiliary heap space for `dag_shortest_path()`.

*/

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int64_t> dist;
std::vector<int> pred;

void topological_dfs(int u, std::vector<char> &visit, std::vector<int> &order) {
  visit[u] = true;
  for (auto &[v, w] : adj[u]) {
    if (!visit[v]) {
      topological_dfs(v, visit, order);
    }
  }
  order.push_back(u);
}

void dag_shortest_path(int start) {
  int n = static_cast<int>(adj.size());
  std::vector<char> visit(n, false);
  std::vector<int> order;
  for (int i = 0; i < n; i++) {
    if (!visit[i]) {
      topological_dfs(i, visit, order);
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
    for (auto &[v, w] : adj[u]) {
      if (dist[u] + w < dist[v]) {
        dist[v] = dist[u] + w;
        pred[v] = u;
      }
    }
  }
}

/*** Example Usage and Output:

The shortest distance from 0 to 4 is 2.
Take the path: 0->1->3->4.

***/

#include <iostream>
using namespace std;

void print_path(int dest) {
  vector<int> path;
  for (int j = dest; pred[j] != -1; j = pred[j]) {
    path.push_back(pred[j]);
  }
  cout << "Take the path: ";
  while (!path.empty()) {
    cout << path.back() << "->";
    path.pop_back();
  }
  cout << dest << "." << endl;
}

int main() {
  int start = 0, dest = 4;
  adj.assign(5, {});
  adj[0] = {{1, 1}, {2, 5}};
  adj[1] = {{2, 1}, {3, -2}};  // A negative edge, which Dijkstra could not handle.
  adj[2] = {{3, 1}};
  adj[3] = {{4, 3}};
  dag_shortest_path(start);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
