/*

Given an undirected graph, max_clique() returns the size of the maximum clique,
that is, the largest subset of nodes such that all pairs of nodes in the subset
are connected by an edge. max_clique_weighted() additionally uses a global array
w[] specifying a weight value for each node, returning the clique in the graph
that has maximum total weight.

Both functions apply to a global, pre-populated adjacency matrix adj[] which
must satisfy the condition that adj[u][v] is true if and only if adj[v][u] is
true, for all pairs of nodes u and v respectively between 0 (inclusive) and the
total number of nodes (exclusive) as passed in the function argument. Note that
max_clique_weighted() is an efficient implementation using bitmasks of unsigned
64-bit integers, thus requiring the number of nodes to be less than 64.

Time Complexity:
- O(3^(n/3)) per call to max_clique() and max_clique_weighted(), where n
  is the number of nodes.

Space Complexity:
- O(n^2) for storage of the graph, where n is the number of nodes.
- O(n) auxiliary stack space for max_clique() and max_clique_weighted().

*/

#include <algorithm>
#include <bitset>
#include <vector>

const int MAXN = 35;
typedef std::bitset<MAXN> bits;
typedef unsigned long long uint64;

bool adj[MAXN][MAXN];
int w[MAXN];

int rec(int nodes, bits &curr, bits &pool, bits &excl) {
  if (pool.none() && excl.none()) {
    return curr.count();
  }
  int ans = 0, u = 0;
  for (int v = 0; v < nodes; v++) {
    if (pool[v] || excl[v]) {
      u = v;
    }
  }
  for (int v = 0; v < nodes; v++) {
    if (!pool[v] || adj[u][v]) {
      continue;
    }
    bits ncurr, npool, nexcl;
    for (int i = 0; i < nodes; i++) {
      ncurr[i] = curr[i];
    }
    ncurr[v] = true;
    for (int j = 0; j < nodes; j++) {
      npool[j] = pool[j] && adj[v][j];
      nexcl[j] = excl[j] && adj[v][j];
    }
    ans = std::max(ans, rec(nodes, ncurr, npool, nexcl));
    pool[v] = false;
    excl[v] = true;
  }
  return ans;
}

int max_clique(int nodes) {
  bits curr, excl, pool;
  pool.flip();
  return rec(nodes, curr, pool, excl);
}

int rec(const std::vector<uint64> &g, uint64 curr, uint64 pool, uint64 excl) {
  if (pool == 0 && excl == 0) {
    int res = 0, u = __builtin_ctzll(curr);
    while (u < (int)g.size()) {
      res += w[u];
      u += __builtin_ctzll(curr >> (u + 1)) + 1;
    }
    return res;
  }
  if (pool == 0) {
    return -1;
  }
  int res = -1, pivot = __builtin_ctzll(pool | excl);
  uint64 z = pool & ~g[pivot];
  int u = __builtin_ctzll(z);
  while (u < (int)g.size()) {
    res = std::max(res, rec(g, curr | (1LL << u), pool & g[u], excl & g[u]));
    pool ^= 1LL << u;
    excl |= 1LL << u;
    u += __builtin_ctzll(z >> (u + 1)) + 1;
  }
  return res;
}

int max_clique_weighted(int nodes) {
  std::vector<uint64> g(nodes, 0);
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < nodes; j++) {
      if (adj[i][j]) {
        g[i] |= 1LL << j;
      }
    }
  }
  return rec(g, 0, (1LL << nodes) - 1, 0);
}

/*** Example Usage ***/

#include <cassert>

void add_edge(int u, int v) {
  adj[u][v] = true;
  adj[v][u] = true;
}

int main() {
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
  assert(max_clique(5) == 4);
  assert(max_clique_weighted(5) == 120);
  return 0;
}
