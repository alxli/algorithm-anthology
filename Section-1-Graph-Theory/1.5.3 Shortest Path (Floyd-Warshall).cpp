/*

1.5.3 - Floyd-Warshall Algorithm (All-Pairs Shortest Path)

Description: Given a directed graph with positive or negative weights
but no negative cycles, find the shortest distance between all pairs
of nodes. The input graph is stored using an adjacency matrix.

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

const int MAXN = 100, INF = 0x3F3F3F3F;
int nodes, edges, a, b, weight, start, dest;
int dist[MAXN][MAXN], next[MAXN][MAXN];

void print_path(int i, int j) {
  if (next[i][j] != -1) {
    print_path(i, next[i][j]);
    cout << next[i][j];
    print_path(next[i][j], j);
  } else cout << "->";
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < nodes; i++)
    for (int j = 0; j < nodes; j++) {
      dist[i][j] = (i == j) ? 0 : INF;
      next[i][j] = -1;
    }
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> weight;
    dist[a][b] = weight;
  }
  cin >> start >> dest;
  for (int k = 0; k < nodes; k++)
   for (int i = 0; i < nodes; i++)
    for (int j = 0; j < nodes; j++)
      if (dist[i][j] > dist[i][k] + dist[k][j]) {
          dist[i][j] = dist[i][k] + dist[k][j];
          next[i][j] = k;
      }
  cout << "The shortest distance from " << start;
  cout << " to " << dest << " is ";
  cout << dist[start][dest] << ".\n";

  //Use next[][] to recursively print the path
  cout << "Take the path " << start;
  print_path(start, dest);
  cout << dest << ".\n";
  return 0;
}
