/*

Given a starting node in a weighted, directed graph with nonnegative weights only, visit every
connected node and determine the minimum distance to each such node. Optionally, output the shortest
path to a specific destination node using the shortest-path tree from the predecessor array
`pred`.

Dijkstra's algorithm repeatedly selects the unvisited node of smallest tentative distance using a
priority queue and relaxes its outgoing edges. Because the weights are nonnegative, a node's
distance is final the first time it is removed from the queue.

- `dijkstra(start)` populates `dist` and `pred` for a global, pre-populated adjacency list `adj`
  which must consist of nodes numbered from 0 to `adj.size() - 1`. Each edge is stored as
  `(neighbor, weight)`, where `weight` is nonnegative.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Dijkstra's algorithm requires nonnegative edge weights. Use Bellman-Ford or SPFA instead when
negative edges are present.

Time Complexity:
- O(m log n) for `dijkstra()`, where $m$ is the number of edges and $n$ is the number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(m) auxiliary heap space for `dijkstra()`.

*/

#include <climits>
#include <functional>
#include <queue>
#include <utility>
#include <vector>

const long long INF = LLONG_MAX / 4;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<long long> dist;
std::vector<int> pred;

void dijkstra(int start) {
  int nodes = adj.size();
  std::vector<bool> visit(nodes, false);
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  dist[start] = 0;
  std::priority_queue<
      std::pair<long long, int>, std::vector<std::pair<long long, int>>,
      std::greater<std::pair<long long, int>>>
      pq;
  pq.emplace(0, start);
  while (!pq.empty()) {
    auto [du, u] = pq.top();
    pq.pop();
    if (visit[u] || du != dist[u]) {
      continue;
    }
    visit[u] = true;
    for (auto &[v, w] : adj[u]) {
      if (visit[v]) {
        continue;
      }
      if (dist[v] > dist[u] + w) {
        dist[v] = dist[u] + w;
        pred[v] = u;
        pq.emplace(dist[v], v);
      }
    }
  }
}

/*** Example Usage and Output:

The shortest distance from 0 to 3 is 5.
Take the path: 0->1->2->3.

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
  int start = 0, dest = 3;
  adj.assign(4, {});
  adj[0].push_back({1, 2});
  adj[0].push_back({3, 8});
  adj[1].push_back({2, 2});
  adj[1].push_back({3, 4});
  adj[2].push_back({3, 1});
  dijkstra(start);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << "."
       << endl;
  print_path(dest);
  return 0;
}
