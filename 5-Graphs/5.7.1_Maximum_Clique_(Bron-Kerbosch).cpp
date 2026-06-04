/*

Given an undirected graph, `max_clique()` returns the size of the maximum clique, that is, the
largest subset of nodes such that all pairs of nodes in the subset are connected by an edge.
`max_clique_weighted()` additionally uses a global array `w` specifying a weight value for each
node, returning the clique in the graph that has maximum total weight.

Both functions apply to a global, pre-populated adjacency matrix `adj` which must satisfy the
condition that `adj[u][v]` is `true` if and only if `adj[v][u]` is `true`, for all pairs of nodes
$u$ and $v$ respectively between 0 (inclusive) and `adj.size()` (exclusive). These implementations
use bitmasks of unsigned 64-bit integers, so the number of nodes must be less than 64.

Time Complexity:
- O(3^(n/3)) per call to `max_clique()` and `max_clique_weighted()`, where $n$ is the number of
  nodes.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `max_clique()` and `max_clique_weighted()`.

*/

#include <algorithm>
#include <vector>

using uint64 = unsigned long long;

std::vector<std::vector<bool>> adj;
std::vector<int> w;

std::vector<uint64> build_mask_graph() {
  int nodes = adj.size();
  std::vector<uint64> g(nodes);
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < nodes; j++) {
      if (adj[i][j]) {
        g[i] |= 1ULL << j;
      }
    }
  }
  return g;
}

int max_clique_rec(const std::vector<uint64> &g, uint64 curr, uint64 pool, uint64 excl) {
  if (pool == 0 && excl == 0) {
    return __builtin_popcountll(curr);
  }
  int res = 0;
  int pivot = __builtin_ctzll(pool | excl);
  uint64 candidates = pool & ~g[pivot];
  while (candidates != 0) {
    int u = __builtin_ctzll(candidates);
    res = std::max(res, max_clique_rec(g, curr | (1ULL << u), pool & g[u], excl & g[u]));
    pool ^= 1ULL << u;
    excl |= 1ULL << u;
    candidates &= candidates - 1;
  }
  return res;
}

int max_clique() {
  int nodes = adj.size();
  std::vector<uint64> g = build_mask_graph();
  return max_clique_rec(g, 0, (1ULL << nodes) - 1, 0);
}

int max_clique_weighted_rec(const std::vector<uint64> &g, uint64 curr, uint64 pool, uint64 excl) {
  if (pool == 0 && excl == 0) {
    int res = 0;
    while (curr != 0) {
      int u = __builtin_ctzll(curr);
      res += w[u];
      curr &= curr - 1;
    }
    return res;
  }
  if (pool == 0) {
    return -1;
  }
  int res = -1, pivot = __builtin_ctzll(pool | excl);
  uint64 z = pool & ~g[pivot];
  while (z != 0) {
    int u = __builtin_ctzll(z);
    res = std::max(res, max_clique_weighted_rec(g, curr | (1ULL << u), pool & g[u], excl & g[u]));
    pool ^= 1ULL << u;
    excl |= 1ULL << u;
    z &= z - 1;
  }
  return res;
}

int max_clique_weighted() {
  int nodes = adj.size();
  std::vector<uint64> g = build_mask_graph();
  return max_clique_weighted_rec(g, 0, (1ULL << nodes) - 1, 0);
}

/*** Example Usage ***/

#include <cassert>

void add_edge(int u, int v) {
  adj[u][v] = true;
  adj[v][u] = true;
}

int main() {
  int nodes = 5;
  adj.assign(nodes, std::vector<bool>(nodes));
  w.assign(nodes, 0);
  add_edge(0, 1);
  add_edge(0, 2);
  add_edge(0, 3);
  add_edge(1, 2);
  add_edge(1, 3);
  add_edge(2, 3);
  add_edge(3, 4);
  add_edge(4, 2);
  w[0] = 10;
  w[1] = 20;
  w[2] = 30;
  w[3] = 40;
  w[4] = 50;
  assert(max_clique() == 4);
  assert(max_clique_weighted() == 120);
  return 0;
}
