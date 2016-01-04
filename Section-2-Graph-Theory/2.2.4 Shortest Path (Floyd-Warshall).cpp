/*

Description: Given a directed graph with positive or negative
weights but no negative cycles, find the shortest distance
between all pairs of nodes. The input graph is stored using
an adjacency matrix. Note that the input adjacency matrix
is converted to the distance matrix afterwards. If you still
need the adjacencies afterwards, back it up at the beginning.

Complexity: O(V^3) on the number of vertices.

=~=~=~=~= Sample Input =~=~=~=~=
3 3
0 1 1
1 2 2
0 2 5
0 2

=~=~=~=~= Sample Output =~=~=~=~=
The shortest distance from 0 to 2 is 3.
Take the path: 0->1->2.

*/

#include <iostream>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN][MAXN], next[MAXN][MAXN];

void initialize(int nodes) {
  for (int i = 0; i < nodes; i++)
    for (int j = 0; j < nodes; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      next[i][j] = -1;
    }
}

void floyd_warshall(int nodes) {
  for (int k = 0; k < nodes; k++)
   for (int i = 0; i < nodes; i++)
    for (int j = 0; j < nodes; j++)
      if (dist[i][j] > dist[i][k] + dist[k][j]) {
        dist[i][j] = dist[i][k] + dist[k][j];
        next[i][j] = k;
      }
}

void print_path(int u, int v) {
  if (next[u][v] != -1) {
    print_path(u, next[u][v]);
    cout << next[u][v];
    print_path(next[u][v], v);
  } else cout << "->";
}

int main() {
  int nodes, edges, u, v, w, start, dest;
  cin >> nodes >> edges;
  initialize(nodes);
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> w;
    dist[u][v] = w;
  }
  cin >> start >> dest;
  floyd_warshall(nodes);
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is ";
  cout << dist[start][dest] << ".\n";

  //Use next[][] to recursively print the path
  cout << "Take the path " << start;
  print_path(start, dest);
  cout << dest << ".\n";
  return 0;
}
