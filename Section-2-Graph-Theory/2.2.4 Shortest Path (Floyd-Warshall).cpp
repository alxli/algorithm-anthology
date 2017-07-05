/*

Given a weighted, directed graph with possibly negative weights, determine the
minimum distance between all pairs of start and destination nodes in the graph.
Optionally, output the shortest path between two nodes using the shortest-path
tree precomputed into the parent[][] array. floyd_warshall() applies to a global
adjacency matrix dist[][], which must be initialized using initialize() and
subsequently populated with weights. After the function call, dist[u][v] will
have been modified to contain the shortest path from u to v, for all pairs of
valid nodes u and v.

This function will also detect whether the graph contains negative-weighted
cycles, in which case there is no shortest path and an error will be thrown.

Time Complexity:
- O(n^2) per call to initialize(), where n is the number of nodes.
- O(n^3) per call to floyd_warshall().

Space Complexity:
- O(n^2) for storage of the graph, where n is the number of nodes.
- O(n^2) auxiliary heap space for initialize() and floyd_warshall().

*/

#include <stdexcept>

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN][MAXN], parent[MAXN][MAXN];

void initialize(int nodes) {
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < nodes; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      parent[i][j] = j;
    }
  }
}

void floyd_warshall(int nodes) {
  for (int k = 0; k < nodes; k++) {
    for (int i = 0; i < nodes; i++) {
      for (int j = 0; j < nodes; j++) {
        if (dist[i][j] > dist[i][k] + dist[k][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          parent[i][j] = parent[i][k];
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
  cout << "Take the path " << u;
  while (u != v) {
    u = parent[u][v];
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
  floyd_warshall(3);
  cout << "The shortest distance from " << start << " to " << dest << " is "
       << dist[start][dest] << "." << endl;
  print_path(start, dest);
  return 0;
}
