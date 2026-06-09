/*

Given a tree, determine the lowest common ancestor of any two nodes. The lowest common ancestor of
two nodes $u$ and $v$ is the node that has the longest distance from the root while having both $u$
and $v$ as its descendant. A node is considered to be a descendant of itself.

This implementation preprocesses binary ancestor jumps. During a depth-first search, it records
entry and exit times so ancestry can be tested in O(1), and stores `dp[u][i]`, the $2^i$-th ancestor
of each node `u`. To answer `lca(u, v)`, it first handles the case where one node is already an
ancestor of the other, then jumps `u` upward by decreasing powers of two until its parent is the
lowest common ancestor.

- `build(root)` builds the structure over a global, bidirectionally pre-populated adjacency list
  `adj` of nodes numbered 0 to `adj.size() - 1`, which must define one connected tree rooted at
  `root` (default 0).
- `lca(u, v)` returns the lowest common ancestor of nodes `u` and `v`.

Time Complexity:
- O(n log n) per call to `build()`, where $n$ is the number of nodes.
- O(log n) per call to `lca()`.

Space Complexity:
- O(n log n) to store the binary ancestor table, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `build()`.
- O(1) auxiliary for `lca()`.

*/

#include <vector>

std::vector<std::vector<int>> adj, dp;
int len, timer;
std::vector<int> tin, tout;

void dfs(int u, int p) {
  tin[u] = timer++;
  dp[u][0] = p;
  for (int i = 1; i < len; i++) {
    dp[u][i] = dp[dp[u][i - 1]][i - 1];
  }
  for (int v : adj[u]) {
    if (v != p) {
      dfs(v, u);
    }
  }
  tout[u] = timer++;
}

void build(int root = 0) {
  int nodes = static_cast<int>(adj.size());
  len = 1;
  while ((1 << len) <= nodes) {
    len++;
  }
  dp.assign(nodes, std::vector<int>(len));
  tin.assign(nodes, 0);
  tout.assign(nodes, 0);
  timer = 0;
  dfs(root, root);
}

bool is_ancestor(int parent, int child) {
  return (tin[parent] <= tin[child]) && (tout[child] <= tout[parent]);
}

int lca(int u, int v) {
  if (is_ancestor(u, v)) {
    return u;
  }
  if (is_ancestor(v, u)) {
    return v;
  }
  for (int i = len - 1; i >= 0; i--) {
    if (!is_ancestor(dp[u][i], v)) {
      u = dp[u][i];
    }
  }
  return dp[u][0];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  adj.assign(5, {});
  add_edge(0, 1);
  add_edge(0, 4);
  add_edge(1, 2);
  add_edge(1, 3);
  build(0);
  assert(lca(3, 2) == 1);
  assert(lca(2, 4) == 0);
  return 0;
}
