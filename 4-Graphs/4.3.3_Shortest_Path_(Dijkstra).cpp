/*

Given a starting node in a weighted, directed graph with nonnegative weights only, visit every
connected node and determine the minimum distance to each such node. Optionally, output the shortest
path to a specific destination node using the shortest-path tree from the predecessor array
`pred`.

Dijkstra's algorithm repeatedly selects the unvisited node of smallest tentative distance using a
priority queue and relaxes its outgoing edges. Dijkstra's algorithm requires nonnegative edge
weights. Use Bellman-Ford or SPFA instead when negative edges are present. Because the weights are
nonnegative, a node's distance is final the first time it is removed from the queue.

- `dijkstra(start, adj, dist, pred)` returns a pair of vectors `dist` and `pred` for an adjacency
  list `adj` which must consist of nodes numbered $[0, `n`)$, where `n` is `adj.size()`. Each edge
  is stored as $(`neighbor`, `weight`)$, where `weight` is nonnegative.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(m log n) for `dijkstra()`, where $m$ is the number of edges and $n$ is the number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(m) auxiliary heap space for `dijkstra()`.

*/

#include <cstdint>
#include <functional>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

template<typename T>
std::pair<std::vector<T>, std::vector<int>> dijkstra(
    const std::vector<std::vector<std::pair<int, T>>> &adj, int start
) {
  const T INF = std::numeric_limits<T>::max() / 4;
  int n = static_cast<int>(adj.size());
  std::vector<T> dist(n, INF);
  std::vector<int> pred(n, -1);
  dist[start] = 0;
  std::priority_queue<
      std::pair<T, int>, std::vector<std::pair<T, int>>, std::greater<std::pair<T, int>>>
      pq;
  pq.emplace(0, start);
  while (!pq.empty()) {
    auto [du, u] = pq.top();
    pq.pop();
    if (du != dist[u]) {
      continue;
    }
    for (auto &[v, w] : adj[u]) {
      if (dist[v] > dist[u] + w) {
        dist[v] = dist[u] + w;
        pred[v] = u;
        pq.emplace(dist[v], v);
      }
    }
  }
  return {dist, pred};
}

/*** Example Usage and Output:

The shortest distance from 0 to 3 is 5.
Take the path: 0->1->2->3.

***/

#include <cassert>
#include <iostream>
using namespace std;

void print_path(const vector<int> &pred, int dest) {
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
  //     w=2     w=2    
  //  0 ----> 1 ----> 2
  //   \      |      /
  // w=8\  w=4|     /w=1
  //     \    v    /
  //      --> 3 <--
  vector<vector<pair<int, int64_t>>> adj(4);
  adj[0].emplace_back(1, 2);
  adj[0].emplace_back(3, 8);
  adj[1].emplace_back(2, 2);
  adj[1].emplace_back(3, 4);
  adj[2].emplace_back(3, 1);
  int start = 0, dest = 3;
  auto [dist, pred] = dijkstra(adj, start);
  assert(dist[dest] == 5);
  assert(pred[dest] == 2 && pred[2] == 1 && pred[1] == 0);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(pred, dest);
  return 0;
}
