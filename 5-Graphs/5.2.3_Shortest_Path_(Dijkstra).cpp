/*

Given a starting node in a weighted, directed graph with nonnegative weights only, visit every
connected node and determine the minimum distance to each such node. Optionally, output the shortest
path to a specific destination node using the shortest-path tree from the predecessor array
`pred`. `dijkstra()` applies to a global, pre-populated adjacency list `adj`.

Since `std::priority_queue` is by default a max-heap, we store negated node distances so the
closest candidate is popped first.

Dijkstra's algorithm may be modified to support negative edge weights by allowing nodes to be
re-visited (removing the visited array check in the inner for-loop). This is known as the Shortest
Path Faster Algorithm (SPFA), which has a larger running time of O(n*m) on the number of nodes and
edges respectively. While it is as slow in the worst case as the Bellman-Ford algorithm, the SPFA
still tends to outperform in the average case.

Time Complexity:
- O(m log n) for `dijkstra()`, where $m$ is the number of edges and $n$ is the number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(m) auxiliary heap space for `dijkstra()`.

*/

#include <climits>
#include <queue>
#include <utility>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<std::pair<int, int>>> adj;
std::vector<int> dist, pred;

void dijkstra(int start) {
  int nodes = adj.size();
  std::vector<bool> visit(nodes, false);
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  dist[start] = 0;
  std::priority_queue<std::pair<int, int>> pq;
  pq.emplace(0, start);
  while (!pq.empty()) {
    auto [neg_d, u] = pq.top();
    pq.pop();
    if (visit[u]) {
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
        pq.emplace(-dist[v], v);
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
  int nodes = 4, start = 0, dest = 3;
  adj.resize(nodes);
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
