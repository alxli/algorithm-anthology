/*

Given a starting node in a weighted, directed graph with possibly negative weights, visit every
connected node and determine the minimum distance to each such node. Optionally, output the shortest
path to a specific destination node using the shortest-path tree from the predecessor array
`pred`. `bellman_ford()` applies to a global, pre-populated edge list which must only consist of
nodes numbered with integers between 0 (inclusive) and the total number of nodes (exclusive), as
passed in the function argument.

This function will also detect whether the graph contains a negative-weight cycle reachable from the
start node, in which case the affected shortest paths are undefined and an error will be thrown. (To
detect a negative cycle anywhere in the graph, add a virtual source with zero-weight edges to every
node and start from it.)

Time Complexity:
- O(n*m) per call to `bellman_ford()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary heap space for `bellman_ford()`, where $n$ is the number of nodes.

*/

#include <climits>
#include <stdexcept>
#include <vector>

struct Edge {
  int u, v, w;
};

const int INF = INT_MAX / 2;
std::vector<Edge> edges;
std::vector<int> dist, pred;

void bellman_ford(int nodes, int start) {
  dist.assign(nodes, INF);
  pred.assign(nodes, -1);
  dist[start] = 0;
  for (int i = 0; i < nodes - 1; i++) {
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
  int start = 0, dest = 2;
  edges.push_back(Edge{0, 1, 1});
  edges.push_back(Edge{1, 2, 2});
  edges.push_back(Edge{0, 2, 5});
  bellman_ford(3, start);
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[dest] << "."
       << endl;
  print_path(dest);
  return 0;
}
