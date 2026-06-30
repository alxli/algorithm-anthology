/*

Given a starting node in a weighted, directed graph with possibly negative weights, visit every
connected node and determine the minimum distance to each such node. Optionally, output the shortest
path to a specific destination node using the shortest-path tree from the predecessor array
`pred`.

Bellman-Ford relaxes every edge in the graph $n - 1$ times. Since any shortest path uses at most
$n - 1$ edges, all distances are correct after these passes unless a negative-weight cycle keeps
reducing them, which a further pass detects. Bellman-Ford will also detect whether the graph
contains a negative-weight cycle reachable from the start node, in which case the affected shortest
paths are undefined and an error will be thrown. (To detect a negative cycle anywhere in the graph,
add a virtual source with zero-weight edges to every node and start from it.)

- `bellman_ford(n, start)` populates `dist` and `pred` for a global, pre-populated edge list `edges`
  whose endpoints must be numbered $[0, `n`)$.

For path reconstruction, `pred[v]` stores the node immediately before `v` on the shortest path from
`start` to `v`, or $-1$ if `v` is `start` or unreachable. Follow `pred` backward from the
destination to `start`, then reverse that sequence to recover the path.

Time Complexity:
- O(n*m) per call, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary.

*/

#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::tuple<int, int, int>> edges;  // (u, v, w)
std::vector<int64_t> dist;
std::vector<int> pred;

void bellman_ford(int n, int start) {
  dist.assign(n, INF);
  pred.assign(n, -1);
  dist[start] = 0;
  for (int i = 0; i < n - 1; i++) {
    for (auto &[u, v, w] : edges) {
      // The dist[u] != INF guard avoids relaxing out of unreachable nodes: a negative edge from an
      // unreachable u would otherwise give v a bogus finite distance (INF + w < INF).
      if (dist[u] != INF && dist[v] > dist[u] + w) {
        dist[v] = dist[u] + w;
        pred[v] = u;
      }
    }
  }
  // Optional: report a negative-weight cycle reachable from the start node.
  for (auto &[u, v, w] : edges) {
    if (dist[u] != INF && dist[v] > dist[u] + w) {
      throw std::runtime_error("Negative-weight cycle found.");
    }
  }
}

/*** Example Usage and Output:

The shortest distance from 0 to 2 is 3.
Take the path: 0->1->2.

***/

#include <cassert>
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
  //    w=1      w=2
  // 0 -----> 1 -----> 2
  // |                 ^
  // +-----------------+
  //         w=5
  edges.emplace_back(0, 1, 1);
  edges.emplace_back(1, 2, 2);
  edges.emplace_back(0, 2, 5);
  int start = 0, dest = 2;
  bellman_ford(3, start);
  assert(dist[dest] == 3);
  assert(pred[dest] == 1);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << ".\n";
  print_path(dest);
  return 0;
}
