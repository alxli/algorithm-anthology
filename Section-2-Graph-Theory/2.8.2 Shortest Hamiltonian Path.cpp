/*

Given a weighted, directed graph, determine a path of minimum total distance
which visits each node exactly once. Unlike the travelling salesman problem, we
do not have to return to the starting vertex. Since this implementation uses
bitmasks with 32-bit ints, the maximum number of nodes must be less than 32.

Time Complexity: O(2^n * n^2) on the number of nodes.
Space Complexity: O(2^n * n^2) on the number of nodes.

*/

#include <algorithm>  // std::fill(), std::min()

const int MAXN = 20, INF = 0x3f3f3f3f;
int adj[MAXN][MAXN], dp[1 << MAXN][MAXN], order[MAXN];

int shortest_hamiltonian_path(int nodes) {
  int max_mask = (1 << nodes) - 1;
  for (int i = 0; i <= max_mask; i++) {
    std::fill(dp[i], dp[i] + nodes, INF);
  }
  for (int i = 0; i < nodes; i++) {
    dp[1 << i][i] = 0;
  }
  for (int mask = 1; mask <= max_mask; mask += 2) {
    for (int i = 0; i < nodes; i++) {
      if ((mask & 1 << i) != 0) {
        for (int j = 0; j < nodes; j++) {
          if ((mask & 1 << j) != 0)
            dp[mask][i] = std::min(dp[mask][i],
                                   dp[mask ^ (1 << i)][j] + adj[j][i]);
        }
      }
    }
  }
  int res = INF + INF;
  for (int i = 1; i < nodes; i++) {
    res = std::min(res, dp[max_mask][i]);
  }
  int cur = max_mask, old = -1;
  for (int i = nodes - 1; i >= 0; i--) {
    int bj = -1;
    for (int j = 0; j < nodes; j++) {
      if ((cur & 1 << j) != 0 && (bj == -1 ||
              dp[cur][bj] + (old == -1 ? 0 : adj[bj][old]) >
               dp[cur][j] + (old == -1 ? 0 : adj[j][old]))) {
        bj = j;
      }
    }
    order[i] = bj;
    cur ^= 1 << bj;
    old = bj;
  }
  return res;
}

/*** Example Usage and Output:

The shortest hamiltonian path has length 3.
Take the path: 0->1->2.

***/

#include <iostream>
using namespace std;

int main() {
  int nodes = 3;
  adj[0][1] = 1;
  adj[0][2] = 1;
  adj[1][0] = 7;
  adj[1][2] = 2;
  adj[2][0] = 3;
  adj[2][1] = 5;
  cout << "The shortest hamiltonian path has length "
       << shortest_hamiltonian_path(nodes) << "." << endl
       << "Take the path: " << order[0];
  for (int i = 1; i < nodes; i++) {
    cout << "->" << order[i];
  }
  cout << "." << endl;
  return 0;
}
