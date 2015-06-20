/*

2.8.1 - Shortest Hamiltonian Cycle (TSP)

Description: Given a weighted, directed graph, the shortest
hamiltonian cycle is a cycle of minimum distance that visits
each vertex exactly once and returns to the original vertex.
This is also known as the traveling salesman problem (TSP).
Since this is a bitmasking solution with 32-bit integers,
the number of vertices must be less than 32.

Complexity: O(2^V * V^2) on the number of vertices.

=~=~=~=~= Sample Input =~=~=~=~=
5 10
0 1 1
0 2 10
0 3 1
0 4 10
1 2 10
1 3 10
1 4 1
2 3 1
2 4 1
3 4 10

=~=~=~=~= Sample Output =~=~=~=~=
The shortest hamiltonian cycle has length 5.
Take the path: 0->3->2->4->1->0

*/

#include <algorithm> /* std::fill(), std::min() */
#include <iostream>
using namespace std;

const int MAXN = 20, INF = 0x3f3f3f3f;
int adj[MAXN][MAXN], order[MAXN];

int shortest_hamiltonian_cycle(int nodes) {
  int dp[1 << nodes][nodes];
  for (int i = 0; i < (1 << nodes); i++)
    fill(dp[i], dp[i] + nodes, INF);
  dp[1][0] = 0;
  for (int mask = 1; mask < (1 << nodes); mask += 2) {
    for (int i = 1; i < nodes; i++)
      if ((mask & 1 << i) != 0)
        for (int j = 0; j < nodes; j++)
          if ((mask & 1 << j) != 0)
            dp[mask][i] = min(dp[mask][i], dp[mask ^ (1 << i)][j] + adj[j][i]);
  }
  int res = INF + INF;
  for (int i = 1; i < nodes; i++)
    res = min(res, dp[(1 << nodes) - 1][i] + adj[i][0]);
  int cur = (1 << nodes) - 1, last = 0;
  for (int i = nodes - 1; i >= 1; i--) {
    int bj = -1;
    for (int j = 1; j < nodes; j++) {
      if ((cur & 1 << j) != 0 && (bj == -1 ||
            dp[cur][bj] + adj[bj][last] > dp[cur][j] + adj[j][last])) {
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
    adj[u][v] = adj[v][u] = w; //only set adj[u][v] if directed edges
  }
  cout << "The shortest hamiltonian cycle has length ";
  cout << shortest_hamiltonian_cycle(nodes) << ".\n";
  cout << "Take the path: ";
  for (int i = 0; i < nodes; i++) cout << order[i] << "->";
  cout << order[0] << "\n";
  return 0;
}
