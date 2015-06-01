/*

1.8.1 - Shortest Hamiltonian Cycle (TSP)

Description: Given a weighted, undirected graph, the shortest
hamiltonian cycle is a cycle of minimum distance that visits
each vertex exactly once and returns to the original vertex.
This is also known as the traveling salesman problem (TSP).

Complexity: O(2^N * N^2)

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

#include <iostream>
using namespace std;

const int MAXN = 20, INF = 0x3F3F3F3F;
int nodes, edges, a, b, weight;
int adj[MAXN][MAXN], order[MAXN];

int shortest_hamiltonian_cycle() {
  int dp[1 << nodes][nodes];
  for (int i = 0; i < (1 << nodes); i++) fill(dp[i], dp[i] + nodes, INF);
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
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> weight;
    adj[a][b] = adj[b][a] = weight;
  }
  int len = shortest_hamiltonian_cycle();
  cout << "The shortest hamiltonian cycle has length ";
  cout << len << ".\n" << "Take the path: ";
  for (int i = 0; i < nodes; i++) cout << order[i] << "->";
  cout << order[0] << "\n";
  return 0;
}
