/*

1.8.2 - Shortest Hamiltonian Path

Description: Given a weighted, directed graph, the shortest
hamiltonian path is a path of minimum distance that visits
each vertex exactly once. Unlike the travelling salesman
problem, we don't have to return to the starting vertex.

Complexity: O(2^N * N^2)

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

#include <iostream>
using namespace std;

const int MAXN = 20, INF = 0x3F3F3F3F;
int nodes, edges, a, b, weight;
int adj[MAXN][MAXN], order[MAXN];

int shortest_hamiltonian_path() {
  int dp[1 << nodes][nodes];
  for (int i = 0; i < (1 << nodes); i++) fill(dp[i], dp[i] + nodes, INF);
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
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> weight;
    adj[a][b] = weight;
  }
  int len = shortest_hamiltonian_path();
  cout << "The shortest hamiltonian path has length ";
  cout << len << ".\n" << "Take the path: " << order[0];
  for (int i = 1; i < nodes; i++) cout << "->" << order[i];
  return 0;
}
