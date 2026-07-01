/*

Given a connected, undirected graph with nonnegative edge weights, the global minimum cut is the
partition of the vertices into two nonempty sets that minimizes the total weight of edges crossing
between them. Unlike a minimum $s$-$t$ cut, no source or sink is fixed; the goal is the cheapest way
to split the graph in two. The Stoer-Wagner algorithm finds it without any maximum flow computation
by repeatedly running a "minimum cut phase".

A phase grows a set, starting from an arbitrary vertex, by repeatedly adding the most tightly
connected outside vertex (the one with the greatest total weight to the set so far), exactly as in
Prim's algorithm. The last vertex added, together with its connection weight, yields one candidate
cut, the "cut of the phase". The last two vertices added are then merged into a single supernode and
the next phase begins on the smaller graph. After $n - 1$ phases every candidate has been
considered, and the smallest is the global minimum cut.

- `stoer_wagner(adj)` returns a pair $(`weight`, `side`)$ for a graph as a symmetric adjacency
  matrix `adj` (with $0$ for the diagonal and absent edges), where `weight` is the total weight of
  the global minimum cut and `side` lists the vertices on one side of that cut. The graph must have
  at least two vertices.

Parallel edges should be pre-summed into the adjacency matrix before calling: `adj[u][v]` holds a
single combined weight, so the sum of all parallel edge weights between `u` and `v` should be
stored there.

Time Complexity:
- O(n^3) per call, where $n$ is the number of vertices.

Space Complexity:
- O(n^2) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> stoer_wagner(std::vector<std::vector<int64_t>> adj) {
  int n = static_cast<int>(adj.size());
  assert(n >= 2);
  for (int i = 0; i < n; i++) {
    assert(static_cast<int>(adj[i].size()) == n);
    for (int j = i + 1; j < n; j++) {
      assert(adj[i][j] == adj[j][i]);
    }
  }
  std::pair<int64_t, std::vector<int>> best = {INT64_MAX, {}};
  std::vector<std::vector<int>> groups(n);
  std::vector<int64_t> weight(n);
  std::vector<char> exists(n, true), added(n);
  for (int i = 0; i < n; i++) {
    groups[i] = {i};
  }
  for (int phase = 1; phase < n; phase++) {
    std::fill(weight.begin(), weight.end(), 0);
    std::fill(added.begin(), added.end(), false);
    int s = -1;
    for (int it = 0; it <= n - phase; it++) {
      int sel = -1;
      for (int i = 0; i < n; i++) {
        if (exists[i] && !added[i] && (sel == -1 || weight[i] > weight[sel])) {
          sel = i;
        }
      }
      if (it == n - phase) {
        if (weight[sel] < best.first) {
          best = {weight[sel], groups[sel]};
        }
        groups[s].insert(groups[s].end(), groups[sel].begin(), groups[sel].end());
        for (int i = 0; i < n; i++) {
          adj[s][i] += adj[sel][i];
          adj[i][s] = adj[s][i];
        }
        exists[sel] = false;
        break;
      }
      added[sel] = true;
      for (int i = 0; i < n; i++) {
        weight[i] += adj[sel][i];
      }
      s = sel;
    }
  }
  return best;
}

/*** Example Usage and Output:

Global min cut weight: 3
One side: 1 2

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  //      w=2
  //   0 ----- 1
  //    \     /
  // w=1 \   / w=3
  //       2
  vector<vector<int64_t>> adj{
      {0, 2, 1},
      {2, 0, 3},
      {1, 3, 0},
  };
  // Cuts: {0}|{1,2} = 3, {1}|{0,2} = 5, {2}|{0,1} = 4. Minimum is 3.
  auto [weight, side] = stoer_wagner(adj);
  assert(weight == 3);
  assert((side == vector<int>{1, 2}));  // The shore {1, 2}, opposite the single vertex 0.
  cout << "Global min cut weight: " << weight << "\n";
  cout << "One side:";
  for (int u : side) {
    cout << " " << u;
  }
  cout << "\n";
  return 0;
}
