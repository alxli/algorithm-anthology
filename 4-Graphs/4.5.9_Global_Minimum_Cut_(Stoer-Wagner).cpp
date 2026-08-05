/*

Given an undirected graph with nonnegative edge weights, the global minimum cut is the partition of
the nodes into two nonempty sets that minimizes the total weight of edges crossing between them.
Unlike a minimum $s$-$t$ cut, no source or sink is fixed; the goal is the cheapest way to split the
graph in two. The Stoer-Wagner algorithm finds it without any maximum flow computation by repeatedly
running a "minimum cut phase".

A phase grows a set, starting from an arbitrary node, by repeatedly adding the most tightly
connected outside node (the one with the greatest total weight to the set so far), exactly as in
Prim's algorithm. The last node added, together with its connection weight, yields one candidate
cut, the "cut of the phase". The last two nodes added are then merged into a single supernode and
the next phase begins on the smaller graph. After $n - 1$ phases every candidate has been
considered, and the smallest is the global minimum cut.

- `global_min_cut(cap)` returns a pair (`weight`, `side`) for a graph as a symmetric capacity matrix
  `cap` (with $0$ for the diagonal and absent edges), where `weight` is the total weight of the
  global minimum cut and `side` lists the nodes on one side of that cut. The graph must have at
  least two nodes.

Parallel edges should be pre-summed into the capacity matrix before calling: `cap[u][v]` holds a
single combined weight, so the sum of all parallel edge weights between `u` and `v` should be stored
there.

Time Complexity:
- O(n^3) per call, where $n$ is the number of nodes.

Space Complexity:
- O(n^2) auxiliary.

*/

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> global_min_cut(std::vector<std::vector<int64_t>> cap) {
  int n = static_cast<int>(cap.size());
  assert(n >= 2);
  for (int i = 0; i < n; i++) {
    for (int j = i + 1; j < n; j++) {
      assert(cap[i][j] >= 0);
      assert(cap[i][j] == cap[j][i]);
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
    weight.assign(n, 0);
    added.assign(n, false);
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
          cap[s][i] += cap[sel][i];  // Overflow warning.
          cap[i][s] = cap[s][i];
        }
        exists[sel] = false;
        break;
      }
      added[sel] = true;
      for (int i = 0; i < n; i++) {
        weight[i] += cap[sel][i];  // Overflow warning.
      }
      s = sel;
    }
  }
  return best;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //      w=2
  //   0 ----- 1
  //    \     /
  // w=1 \   / w=3
  //       2
  vector<vector<int64_t>> cap{
      {0, 2, 1},
      {2, 0, 3},
      {1, 3, 0},
  };
  // Cuts: {0}|{1,2} = 3, {1}|{0,2} = 5, {2}|{0,1} = 4. Minimum is 3.
  auto [weight, side] = global_min_cut(cap);
  assert(weight == 3);
  assert((side == vector<int>{1, 2}));  // The shore {1, 2}, opposite the single node 0.

  vector<vector<int64_t>> disconnected{{0, 0}, {0, 0}};
  assert(global_min_cut(disconnected).first == 0);
  return 0;
}
