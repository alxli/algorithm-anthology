/*

Given a complete, weighted, directed graph, find a minimum-length Hamiltonian path or cycle. A
Hamiltonian path visits each node exactly once. A Hamiltonian cycle additionally returns to its
starting node; this cycle version is the traveling salesman problem (TSP).

Both functions use the Held-Karp subset dynamic program, where `dp[S][i]` is the shortest path
visiting exactly the nodes in `S` and ending at node `i`.

- `shortest_hamiltonian_path()` populates `path` and returns the minimum Hamiltonian path length
  for a global, pre-populated adjacency matrix `adj`.
- `shortest_hamiltonian_cycle()` fixes the start at node 0, populates `path`, and returns the
  minimum Hamiltonian cycle length for a global, pre-populated adjacency matrix `adj`.

Since this implementation uses bitmasks with signed 32-bit integers, the maximum number of nodes
must be less than 31.

Time Complexity:
- O(2^n * n^2) per call, where $n$ is the number of nodes.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(2^n * n) auxiliary heap space.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

const int64_t INF = INT64_MAX / 4;
std::vector<std::vector<int64_t>> adj, dp;
std::vector<int> path;

int64_t shortest_hamiltonian_path() {
  int n = static_cast<int>(adj.size());
  assert(1 <= n && n < 31);
  for (const auto &row : adj) {
    assert(static_cast<int>(row.size()) == n);
  }
  int max_mask = (1 << n) - 1;
  dp.assign(max_mask + 1, std::vector<int64_t>(n, INF));
  path.assign(n, 0);
  for (int i = 0; i < n; i++) {
    dp[1 << i][i] = 0;
  }
  for (int mask = 1; mask <= max_mask; mask++) {
    for (int i = 0; i < n; i++) {
      if ((mask & (1 << i)) != 0) {
        for (int j = 0; j < n; j++) {
          if ((mask & (1 << j)) != 0 && dp[mask ^ (1 << i)][j] != INF) {
            dp[mask][i] = std::min(dp[mask][i], dp[mask ^ (1 << i)][j] + adj[j][i]);
          }
        }
      }
    }
  }
  int64_t res = INF;
  for (int i = 0; i < n; i++) {
    res = std::min(res, dp[max_mask][i]);
  }
  int mask = max_mask, old = -1;
  for (int i = n - 1; i >= 0; i--) {
    int best = -1;
    for (int j = 0; j < n; j++) {
      if ((mask & (1 << j)) != 0 &&
          (best == -1 || dp[mask][best] + (old == -1 ? 0 : adj[best][old]) >
                             dp[mask][j] + (old == -1 ? 0 : adj[j][old]))) {
        best = j;
      }
    }
    path[i] = best;
    mask ^= 1 << best;
    old = best;
  }
  return res;
}

int64_t shortest_hamiltonian_cycle() {
  int n = static_cast<int>(adj.size());
  assert(1 <= n && n < 31);
  for (const auto &row : adj) assert(static_cast<int>(row.size()) == n);
  if (n == 1) {
    path = {0};
    return 0;
  }
  int max_mask = (1 << n) - 1;
  dp.assign(max_mask + 1, std::vector<int64_t>(n, INF));
  path.assign(n, 0);
  dp[1][0] = 0;
  for (int mask = 1; mask <= max_mask; mask += 2) {
    for (int i = 1; i < n; i++) {
      if ((mask & (1 << i)) != 0) {
        for (int j = 0; j < n; j++) {
          if ((mask & (1 << j)) != 0 && dp[mask ^ (1 << i)][j] != INF) {
            dp[mask][i] = std::min(dp[mask][i], dp[mask ^ (1 << i)][j] + adj[j][i]);
          }
        }
      }
    }
  }
  int64_t res = INF;
  for (int i = 1; i < n; i++) {
    res = std::min(res, dp[max_mask][i] + adj[i][0]);
  }
  int mask = max_mask, old = 0;
  for (int i = n - 1; i >= 1; i--) {
    int best = -1;
    for (int j = 1; j < n; j++) {
      if ((mask & (1 << j)) != 0 &&
          (best == -1 || dp[mask][best] + adj[best][old] > dp[mask][j] + adj[j][old])) {
        best = j;
      }
    }
    path[i] = best;
    mask ^= 1 << best;
    old = best;
  }
  return res;
}

/*** Example Usage and Output:

Shortest Hamiltonian path length: 3
Path: 0->1->2

Shortest Hamiltonian cycle length: 6
Cycle: 0->1->2->0

***/

#include <iostream>
using namespace std;

int main() {
  int nodes = 3;
  adj.assign(nodes, std::vector<int64_t>(nodes));
  adj[0][1] = 1;
  adj[0][2] = 1;
  adj[1][0] = 7;
  adj[1][2] = 2;
  adj[2][0] = 3;
  adj[2][1] = 5;

  cout << "Shortest Hamiltonian path length: " << shortest_hamiltonian_path() << endl;
  cout << "Path: " << path[0];
  for (int i = 1; i < nodes; i++) {
    cout << "->" << path[i];
  }
  cout << "\n\n";
  cout << "Shortest Hamiltonian cycle length: " << shortest_hamiltonian_cycle() << endl;
  cout << "Cycle: ";
  for (int i = 0; i < nodes; i++) {
    cout << path[i] << "->";
  }
  cout << path[0] << endl;
  return 0;
}
