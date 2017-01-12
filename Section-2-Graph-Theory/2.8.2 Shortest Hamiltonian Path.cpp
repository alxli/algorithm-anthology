/*

Description: Given a weighted, directed graph, the shortest
hamiltonian path is a path of minimum distance that visits
each vertex exactly once. Unlike the travelling salesman
problem, we don't have to return to the starting vertex.
Since this is a bitmasking solution with 32-bit integers,
the number of nodes must be less than 32.

Complexity: O(2^n * n^2) on the number of nodes.

=~=~=~=~= Sample Input =~=~=~=~=
3 6
0 1 1
0 2 1
1 0 7
1 2 2
2 0 3
2 1 5

=~=~=~=~= Sample Output =~=~=~=~=
The shortest hamiltonian path has length 3.
Take the path: 0->1->2

*/

#include <algorithm> /* std::fill(), std::min() */
#include <iostream>
using namespace std;

const int MAXN = 20, INF = 0x3f3f3f3f;

int adj[MAXN][MAXN], order[MAXN];

int shortest_hamiltonian_path(int nodes) {
  int dp[1 << nodes][nodes];
  for (int i = 0; i < (1 << nodes); i++)
    fill(dp[i], dp[i] + nodes, INF);
  for (int i = 0; i < nodes; i++) dp[1 << i][i] = 0;
  for (int mask = 1; mask < (1 << nodes); mask += 2) {
    for (int i = 0; i < nodes; i++)
      if ((mask & 1 << i) != 0)
        for (int j = 0; j < nodes; j++)
          if ((mask & 1 << j) != 0)
            dp[mask][i] = min(dp[mask][i], dp[mask ^ (1 << i)][j] + adj[j][i]);
  }
  int res = INF + INF;
  for (int i = 1; i < nodes; i++)
    res = min(res, dp[(1 << nodes) - 1][i]);
  int cur = (1 << nodes) - 1, last = -1;
  for (int i = nodes - 1; i >= 0; i--) {
    int bj = -1;
    for (int j = 0; j < nodes; j++) {
      if ((cur & 1 << j) != 0 && (bj == -1 ||
           dp[cur][bj] + (last == -1 ? 0 : adj[bj][last]) >
           dp[cur][j]  + (last == -1 ? 0 : adj[j][last]))) {
        bj = j;
      }
    }
    order[i] = bj;
    cur ^= 1 << bj;
    last = bj;
  }
  return res;
}

int main() {
  int nodes, edges, u, v, w;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> w;
    adj[u][v] = w;
  }
  cout << "The shortest hamiltonian path has length ";
  cout << shortest_hamiltonian_path(nodes) << ".\n";
  cout << "Take the path: " << order[0];
  for (int i = 1; i < nodes; i++) cout << "->" << order[i];
  return 0;
}
