/*

Given a rooted tree or forest, determine the lowest common ancestor of any two nodes in the same
tree. The lowest common ancestor of two nodes $u$ and $v$ is the node that has the greatest depth
while having both $u$ and $v$ as descendants. A node is considered to be a descendant of itself.

This implementation preprocesses binary ancestor jumps. During depth-first search, it records entry
and exit times so ancestry can be tested in O(1), records each node's depth and root, and stores
`up[u][i]`, the $2^i$-th ancestor of each node `u`. To answer `lca(u, v)`, it first handles the
case where one node is already an ancestor of the other, then jumps `u` upward by decreasing powers
of two until its parent is the lowest common ancestor.

- `SparseTableLCA(adj)` builds the structure over a forest represented by a bidirectional adjacency
  list `adj` of nodes numbered $[0, `n`)$, where `n` is `adj.size()`.
- `go_up(u, k)` returns the $k$-th ancestor of node `u`, stopping at that tree's root if `k` is
  larger than `u`'s depth.
- `lca(u, v)` returns the lowest common ancestor of nodes `u` and `v`, or $-1$ if they are in
  different trees.
- `is_ancestor(parent, child)` returns whether `parent` is an ancestor of `child`.
- `dist(u, v)` returns the number of edges on the path between nodes `u` and `v`, or $-1$ if they
  are in different trees.

Time Complexity:
- O(n log n) for construction, where $n$ is the number of nodes.
- O(log n) per call to `go_up()`, `lca()`, and `dist()`.
- O(1) per call to `is_ancestor()`.

Space Complexity:
- O(n log n) to store the binary ancestor table.
- O(n) auxiliary stack space for construction.
- O(1) auxiliary for `go_up()`, `lca()`, `is_ancestor()`, and `dist()`.

*/

#include <algorithm>
#include <vector>

class SparseTableLCA {
  std::vector<std::vector<int>> adj, up;
  std::vector<int> tin, tout, depth, root;
  int len, timer;

  void dfs(int u, int p, int r, int d) {
    tin[u] = timer++;
    depth[u] = d;
    root[u] = r;
    up[u][0] = p;
    for (int i = 1; i < len; i++) {
      up[u][i] = up[up[u][i - 1]][i - 1];
    }
    for (int v : adj[u]) {
      if (v != p) {
        dfs(v, u, r, d + 1);
      }
    }
    tout[u] = timer++;
  }

 public:
  explicit SparseTableLCA(const std::vector<std::vector<int>> &adj) : adj(adj), timer(0) {
    int n = static_cast<int>(adj.size());
    len = 1;
    while ((1 << len) <= std::max(1, n)) {
      len++;
    }
    up.assign(n, std::vector<int>(len));
    tin.assign(n, 0);
    tout.assign(n, 0);
    depth.assign(n, 0);
    root.assign(n, -1);
    for (int u = 0; u < n; u++) {
      if (root[u] == -1) {
        dfs(u, u, u, 0);
      }
    }
  }

  bool is_ancestor(int parent, int child) const {
    return root[parent] == root[child] && tin[parent] <= tin[child] && tout[child] <= tout[parent];
  }

  int go_up(int u, int k) const {
    k = std::min(k, depth[u]);
    for (int i = 0; i < len; i++) {
      if ((k & (1 << i)) != 0) {
        u = up[u][i];
      }
    }
    return u;
  }

  int lca(int u, int v) const {
    if (root[u] != root[v]) {
      return -1;
    }
    if (is_ancestor(u, v)) {
      return u;
    }
    if (is_ancestor(v, u)) {
      return v;
    }
    for (int i = len - 1; i >= 0; i--) {
      if (!is_ancestor(up[u][i], v)) {
        u = up[u][i];
      }
    }
    return up[u][0];
  }

  int dist(int u, int v) const {
    int l = lca(u, v);
    return l == -1 ? -1 : depth[u] + depth[v] - 2 * depth[l];
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(vector<vector<int>> &adj, int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  vector<vector<int>> adj(7);
  add_edge(adj, 0, 1);
  add_edge(adj, 0, 4);
  add_edge(adj, 1, 2);
  add_edge(adj, 1, 3);
  add_edge(adj, 5, 6);

  SparseTableLCA tree(adj);
  assert(tree.go_up(3, 1) == 1);
  assert(tree.go_up(3, 2) == 0);
  assert(tree.go_up(3, 10) == 0);
  assert(tree.lca(3, 2) == 1);
  assert(tree.lca(2, 4) == 0);
  assert(tree.lca(2, 6) == -1);
  assert(tree.lca(5, 6) == 5);
  assert(tree.dist(3, 2) == 2);   // 3-1-2.
  assert(tree.dist(2, 4) == 3);   // 2-1-0-4.
  assert(tree.dist(5, 6) == 1);   // 5-6.
  assert(tree.dist(2, 6) == -1);  // Different trees.
  return 0;
}
