/*

Given an undirected graph, determine a maximum clique: a largest subset of nodes in which every pair
is connected by an edge. A weighted variant instead seeks the clique of maximum total node weight,
given a weight for each node.

The Bron-Kerbosch algorithm recursively extends a growing clique, tracking the set of candidate
nodes that may still be added and the set of nodes already excluded. Choosing a pivot node to avoid
branching on its neighbors prunes large parts of the search, keeping it efficient on most graphs.

- `max_clique()` returns the maximum clique size for a global, bidirectionally pre-populated
  adjacency matrix `adj` which must consist of nodes numbered $[0, `n`)$, where `n` is `adj.size()`.
- `max_clique_weighted()` additionally uses global `w` and returns the maximum clique weight.

These implementations use bitmasks of unsigned 64-bit integers, so the number of nodes must be less
than 64.

Time Complexity:
- O(3^{n/3}) per call to `max_clique()` and `max_clique_weighted()`, where $n$ is the number of
  nodes.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `max_clique()` and `max_clique_weighted()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

std::vector<std::vector<char>> adj;
std::vector<int> w;

std::vector<uint64_t> build_mask_graph() {
  int n = static_cast<int>(adj.size());
  assert(n < 64);
  for (const auto &row : adj) {
    assert(static_cast<int>(row.size()) == n);
  }
  std::vector<uint64_t> g(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (adj[i][j]) {
        g[i] |= 1ULL << j;
      }
    }
  }
  return g;
}

int max_clique_rec(const std::vector<uint64_t> &g, uint64_t curr, uint64_t pool, uint64_t excl) {
  if (pool == 0) {
    return __builtin_popcountll(curr);
  }
  int res = 0;
  int pivot = __builtin_ctzll(pool | excl);
  uint64_t candidates = pool & ~g[pivot];
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
  int n = static_cast<int>(adj.size());
  std::vector<uint64_t> g = build_mask_graph();
  return max_clique_rec(g, 0, (1ULL << n) - 1, 0);
}

int max_clique_weighted_rec(
    const std::vector<uint64_t> &g, uint64_t curr, uint64_t pool, uint64_t excl
) {
  if (pool == 0) {
    int res = 0;
    while (curr != 0) {
      int u = __builtin_ctzll(curr);
      res += w[u];
      curr &= curr - 1;
    }
    return res;
  }
  int res = -1, pivot = __builtin_ctzll(pool | excl);
  uint64_t z = pool & ~g[pivot];
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
  int n = static_cast<int>(adj.size());
  std::vector<uint64_t> g = build_mask_graph();
  return max_clique_weighted_rec(g, 0, (1ULL << n) - 1, 0);
}

/*** Example Usage ***/

void add_edge(int u, int v) {
  adj[u][v] = true;
  adj[v][u] = true;
}

int main() {
  int nodes = 5;
  adj.assign(nodes, std::vector<char>(nodes));
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
