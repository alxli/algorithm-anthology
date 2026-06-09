/*

Given a weighted, directed graph with possibly negative weights, determine the minimum distance
between all pairs of start and destination nodes in the graph. Optionally, output the shortest path
between two nodes using the shortest-path tree precomputed into the `pred` matrix.

- `initialize(nodes)` initializes `dist` and `pred` for nodes numbered from 0 to `nodes - 1`.
- `floyd_warshall()` updates the global adjacency matrix `dist` so `dist[u][v]` stores the shortest
  path from $u$ to $v$, and updates `pred` for path reconstruction. If the graph contains
  negative-weighted cycles, there is no shortest path and an error will be thrown.

Time Complexity:
- O(n^2) per call to `initialize()`, where $n$ is the number of nodes.
- O(n^3) per call to `floyd_warshall()`.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(n^2) auxiliary heap space for `initialize()` and `floyd_warshall()`.

*/

#include <climits>
#include <stdexcept>
#include <vector>

const long long INF = LLONG_MAX / 4;
std::vector<std::vector<long long>> dist;
std::vector<std::vector<int>> pred;

void initialize(int nodes) {
  dist.assign(nodes, std::vector<long long>(nodes));
  pred.assign(nodes, std::vector<int>(nodes));
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < nodes; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      pred[i][j] = j;
    }
  }
}

void floyd_warshall() {
  int nodes = dist.size();
  for (int k = 0; k < nodes; k++) {
    for (int i = 0; i < nodes; i++) {
      for (int j = 0; j < nodes; j++) {
        // The INF guards avoid relaxing through an unreachable intermediate: with a negative edge,
        // INF + w < INF would otherwise give an unreachable pair a bogus finite distance.
        if (dist[i][k] != INF && dist[k][j] != INF && dist[i][j] > dist[i][k] + dist[k][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          pred[i][j] = pred[i][k];
        }
      }
    }
  }
  // Optional: Report negative-weighted cycles.
  for (int i = 0; i < nodes; i++) {
    if (dist[i][i] < 0) {
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

void print_path(int u, int v) {
  cout << "Take the path: " << u;
  while (u != v) {
    u = pred[u][v];
    cout << "->" << u;
  }
  cout << "." << endl;
}

int main() {
  initialize(3);
  int start = 0, dest = 2;
  dist[0][1] = 1;
  dist[1][2] = 2;
  dist[0][2] = 5;
  floyd_warshall();
  cout << "The shortest distance from " << start << " to " << dest << " is " << dist[start][dest]
       << "." << endl;
  print_path(start, dest);
  return 0;
}
