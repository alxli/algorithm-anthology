/*

Given a starting node in a weighted, directed graph with possibly negative
weights, visit every connected node and determine the minimum distance to each
such node. Optionally, output the shortest path to a specific destination node
using the shortest-path tree from the predecessor array pred[]. bellman_ford()
applies to a global, pre-populated edge list which must only consist of nodes
numbered with integers between 0 (inclusive) and the total number of nodes
(exclusive), as passed in the function argument.

This function will also detect whether the graph contains negative-weighted
cycles, in which case there is no shortest path and an error will be thrown.

Time Complexity:
- O(n*m) per call to bellman_ford(), where n is the number of nodes and m is the
  number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n is the number of nodes and m is
  the number of edges.
- O(n) auxiliary heap space for bellman_ford(), where n is the number of nodes.

*/

#include <stdexcept>
#include <vector>

struct edge { int u, v, w; };  // Edge from u to v with weight w.

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<edge> e;
int dist[MAXN], pred[MAXN];

void bellman_ford(int nodes, int start) {
  for (int i = 0; i < nodes; i++) {
    dist[i] = INF;
    pred[i] = -1;
  }
  dist[start] = 0;
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < (int)e.size(); j++) {
      if (dist[e[j].v] > dist[e[j].u] + e[j].w) {
        dist[e[j].v] = dist[e[j].u] + e[j].w;
        pred[e[j].v] = e[j].u;
      }
    }
  }
  // Optional: Report negative-weighted cycles.
  for (int i = 0; i < (int)e.size(); i++) {
    if (dist[e[i].v] > dist[e[i].u] + e[i].w) {
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
  e.push_back((edge){0, 1, 1});
  e.push_back((edge){1, 2, 2});
  e.push_back((edge){0, 2, 5});
  bellman_ford(3, start);
  cout << "The shortest distance from " << start << " to " << dest << " is "
       << dist[dest] << "." << endl;
  print_path(dest);
  return 0;
}
