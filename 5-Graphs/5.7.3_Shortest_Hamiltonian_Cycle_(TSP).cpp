/*

Given a complete weighted graph, determine a cycle of minimum total distance which visits each node
exactly once and returns to the starting node. This is known as the traveling salesman problem
(TSP). Since this implementation uses bitmasks with signed 32-bit integers, the maximum number of
nodes must be less than 31. `shortest_hamiltonian_cycle()` applies to a global adjacency matrix
`adj`, which must be populated with `add_edge()` before the function call.

Time Complexity:
- O(2^n * n^2) per call to `shortest_hamiltonian_cycle()`, where $n$ is the number of nodes.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(2^n * n) auxiliary heap space for `shortest_hamiltonian_cycle()`.

*/

#include <algorithm>
#include <climits>
#include <vector>

const long long INF = LLONG_MAX / 4;
std::vector<std::vector<long long>> adj, dp;
std::vector<int> order;

void add_edge(int u, int v, long long w) {
  adj[u][v] = w;
  adj[v][u] = w;  // Remove this line if the graph is directed.
}

long long shortest_hamiltonian_cycle() {
  int nodes = adj.size();
  int max_mask = (1 << nodes) - 1;
  dp.assign(max_mask + 1, std::vector<long long>(nodes, INF));
  order.assign(nodes, 0);
  dp[1][0] = 0;
  for (int mask = 1; mask <= max_mask; mask += 2) {
    for (int i = 1; i < nodes; i++) {
      if ((mask & 1 << i) != 0) {
        for (int j = 0; j < nodes; j++) {
          if ((mask & 1 << j) != 0 && dp[mask ^ (1 << i)][j] != INF) {
            dp[mask][i] = std::min(dp[mask][i], dp[mask ^ (1 << i)][j] + adj[j][i]);
          }
        }
      }
    }
  }
  long long res = INF;
  for (int i = 1; i < nodes; i++) {
    res = std::min(res, dp[max_mask][i] + adj[i][0]);
  }
  int mask = max_mask, old = 0;
  for (int i = nodes - 1; i >= 1; i--) {
    int bj = -1;
    for (int j = 1; j < nodes; j++) {
      if ((mask & 1 << j) != 0 &&
          (bj == -1 || dp[mask][bj] + adj[bj][old] > dp[mask][j] + adj[j][old])) {
        bj = j;
      }
    }
    order[i] = bj;
    mask ^= 1 << bj;
    old = bj;
  }
  return res;
}

/*** Example Usage and Output:

The shortest hamiltonian cycle has length 5.
Take the path: 0->3->2->4->1->0.

***/

#include <iostream>
using namespace std;

int main() {
  int nodes = 5;
  adj.assign(nodes, std::vector<long long>(nodes));
  add_edge(0, 1, 1);
  add_edge(0, 2, 10);
  add_edge(0, 3, 1);
  add_edge(0, 4, 10);
  add_edge(1, 2, 10);
  add_edge(1, 3, 10);
  add_edge(1, 4, 1);
  add_edge(2, 3, 1);
  add_edge(2, 4, 1);
  add_edge(3, 4, 10);
  cout << "The shortest hamiltonian cycle has length " << shortest_hamiltonian_cycle() << "."
       << endl
       << "Take the path: ";
  for (int i = 0; i < nodes; i++) {
    cout << order[i] << "->";
  }
  cout << order[0] << "." << endl;
  return 0;
}
