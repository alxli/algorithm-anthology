/*

Given a connected, undirected graph with nonnegative edge weights, the global minimum cut is the
partition of the vertices into two nonempty sets that minimizes the total weight of edges crossing
between them. Unlike a minimum s-t cut, no source or sink is fixed; the goal is the cheapest way to
split the graph in two. The Stoer-Wagner algorithm finds it without any maximum flow computation by
repeatedly running a "minimum cut phase".

A phase grows a set, starting from an arbitrary vertex, by repeatedly adding the most tightly
connected outside vertex (the one with the greatest total weight to the set so far), exactly as in
Prim's algorithm. The last vertex added, together with its connection weight, yields one candidate
cut, the "cut of the phase". The last two vertices added are then merged into a single supernode and
the next phase begins on the smaller graph. After $n - 1$ phases every candidate has been considered,
and the smallest is the global minimum cut.

The graph is given as a symmetric adjacency matrix `adj`, where `adj[i][j]` is the weight of the
edge between `i` and `j` (0 if absent), and the diagonal is 0. The graph must have at least two
vertices.

- `stoer_wagner(adj)` returns a pair `(weight, side)`, where `weight` is the total weight of the
  global minimum cut and `side` lists the vertices on one side of that cut.

Time Complexity:
- O(n^3) per call, where $n$ is the number of vertices.

Space Complexity:
- O(n^2) auxiliary heap space.

*/

#include <algorithm>
#include <climits>
#include <utility>
#include <vector>

std::pair<long long, std::vector<int>> stoer_wagner(std::vector<std::vector<long long>> adj) {
  int n = static_cast<int>(adj.size());
  const long long NEG = LLONG_MIN / 4;  // A "merged away" sentinel that additions cannot revive.
  std::pair<long long, std::vector<int>> best = {LLONG_MAX, {}};
  std::vector<std::vector<int>> groups(n);
  for (int i = 0; i < n; i++) {
    groups[i] = {i};
  }
  for (int phase = 1; phase < n; phase++) {
    std::vector<long long> weight = adj[0];
    int s = 0, t = 0;
    for (int added = 0; added < n - phase; added++) {
      weight[t] = NEG;
      s = t;
      t = std::max_element(weight.begin(), weight.end()) - weight.begin();
      for (int i = 0; i < n; i++) {
        weight[i] += adj[t][i];
      }
    }
    if (weight[t] - adj[t][t] < best.first) {
      best = {weight[t] - adj[t][t], groups[t]};
    }
    groups[s].insert(groups[s].end(), groups[t].begin(), groups[t].end());
    for (int i = 0; i < n; i++) {
      adj[s][i] += adj[t][i];
      adj[i][s] = adj[s][i];
    }
    adj[0][t] = NEG;
  }
  return best;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Triangle with edge weights 0-1: 2, 1-2: 3, 0-2: 1.
  // Cuts: {0}|{1,2} = 3, {1}|{0,2} = 5, {2}|{0,1} = 4. Minimum is 3.
  vector<vector<long long>> adj{
      {0, 2, 1},
      {2, 0, 3},
      {1, 3, 0},
  };
  auto [weight, side] = stoer_wagner(adj);
  assert(weight == 3);
  assert((side == vector<int>{1, 2}));  // The shore {1, 2}, opposite the single vertex 0.
  return 0;
}
