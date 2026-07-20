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

These implementations use `mask_t`, which is `uint64_t` by default, so the number of nodes must be
less than `MASK_BITS`. Node weights must be nonnegative.

Time Complexity:
- O(3^{n/3}) per call to `max_clique()` and `max_clique_weighted()`, where $n$ is the number of
  nodes.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `max_clique()` and `max_clique_weighted()`.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstdint>
#include <vector>

using mask_t = uint64_t;
const int MASK_BITS = sizeof(mask_t) * CHAR_BIT;

std::vector<std::vector<char>> adj;
std::vector<int64_t> w;

std::vector<mask_t> build_mask_graph() {
  int n = static_cast<int>(adj.size());
  assert(n < MASK_BITS);
  std::vector<mask_t> g(n);
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      if (adj[i][j]) {
        g[i] |= mask_t{1} << j;
      }
    }
  }
  return g;
}

int max_clique_rec(const std::vector<mask_t> &g, mask_t curr, mask_t pool, mask_t excl) {
  if (pool == 0) {
    return __builtin_popcountll(curr);
  }
  int res = 0;
  int pivot = __builtin_ctzll(pool | excl);
  mask_t candidates = pool & ~g[pivot];
  while (candidates != 0) {
    int u = __builtin_ctzll(candidates);
    res = std::max(res, max_clique_rec(g, curr | (mask_t{1} << u), pool & g[u], excl & g[u]));
    pool ^= mask_t{1} << u;
    excl |= mask_t{1} << u;
    candidates &= candidates - 1;
  }
  return res;
}

int max_clique() {
  int n = static_cast<int>(adj.size());
  std::vector<mask_t> g = build_mask_graph();
  return max_clique_rec(g, 0, (mask_t{1} << n) - 1, 0);
}

int64_t max_clique_weighted_rec(
    const std::vector<mask_t> &g, int64_t curr, mask_t pool, mask_t excl
) {
  if (pool == 0) {
    return curr;
  }
  int64_t res = -1;
  int pivot = __builtin_ctzll(pool | excl);
  mask_t z = pool & ~g[pivot];
  while (z != 0) {
    int u = __builtin_ctzll(z);
    int64_t next_weight = curr + w[u];  // Overflow warning!
    int64_t next = max_clique_weighted_rec(g, next_weight, pool & g[u], excl & g[u]);
    res = std::max(res, next);
    pool ^= mask_t{1} << u;
    excl |= mask_t{1} << u;
    z &= z - 1;
  }
  return res;
}

int64_t max_clique_weighted() {
  int n = static_cast<int>(adj.size());
  std::vector<mask_t> g = build_mask_graph();
  return max_clique_weighted_rec(g, 0, (mask_t{1} << n) - 1, 0);
}

/*** Example Usage ***/

void add_edge(int u, int v) {
  adj[u][v] = true;
  adj[v][u] = true;
}

int main() {
  //          0(10)
  //         / / |
  //       /  /  |
  //     /   /   |
  // 1(20)---+---3(40)---4(50)
  //     \   |   /      /
  //      \  |  /      /
  //       \ | /      /
  //        2(30)----+
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
