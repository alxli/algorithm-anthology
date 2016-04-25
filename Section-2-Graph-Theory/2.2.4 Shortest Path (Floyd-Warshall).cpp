/*

Given a weighted, directed graph with possibly negative weights,
determine the shortest distance between all possible pairs of starting
and destination nodes in the graph. Optionally, detect if there exists
a negative-weighted cycle (in which case the shortest path does not
exist). Optionally, output the shortest path between two nodes using
the shortest-path tree precomputed into the next[][] array.

Time Complexity: initialize() is O(n^2) on the number of nodes.
floyd_warshall() is O(n^3) on the number of nodes. print_path() is O(n)
on the number of nodes in the shortest path to be printed.

Space Complexity: O(n^2) auxiliary on the number of nodes to store the
graph, shortest path distances, and the shortest-path tree.

*/

#include <iostream>
#include <stdexcept>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN][MAXN], next[MAXN][MAXN];

void initialize() {
  for (int i = 0; i < MAXN; i++) {
    for (int j = 0; j < MAXN; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      next[i][j] = j;
    }
  }
}

void floyd_warshall() {
  for (int k = 0; k < MAXN; k++)
    for (int i = 0; i < MAXN; i++)
      for (int j = 0; j < MAXN; j++)
        if (dist[i][j] > dist[i][k] + dist[k][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          next[i][j] = next[i][k];
        }
  //optional: report negative-weight cycles
  for (int i = 0; i < MAXN; i++)
    if (dist[i][i] < 0)
      throw std::runtime_error("Negative-weight cycle found.");
}

void print_path(int u, int v) {
  cout << "Take the path " << u;
  while (u != v) {
    u = next[u][v];
    cout << "->" << u;
  }
  cout << ".\n";
}

/*** Example Usage

Sample Output:
The shortest distance from 0 to 2 is 3.
Take the path: 0->1->2.

***/

int main() {
  initialize();
  int start = 0, dest = 2;
  dist[0][1] = 1;
  dist[1][2] = 2;
  dist[0][2] = 5;
  floyd_warshall();
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is ";
  cout << dist[start][dest] << ".\n";
  print_path(start, dest);
  return 0;
}
