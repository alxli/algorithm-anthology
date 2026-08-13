/*

Given a rooted tree or forest, determine the lowest common ancestor of any two nodes in the same
tree. The lowest common ancestor of two nodes $u$ and $v$ is the node that has the greatest depth
while having both $u$ and $v$ as descendants. A node is considered to be a descendant of itself.

This implementation preprocesses binary ancestor jumps. During depth-first search, it records entry
and exit times so ancestry can be tested in O(1), records each node's depth and root, and stores in
`up[u][i]` the ancestor $2^i$ edges above node `u`. To answer `lca(u, v)`, it first handles the case
where one node is already an ancestor of the other, then jumps `u` upward by decreasing powers of
two until its parent is the lowest common ancestor.

- `BinaryLiftingLCA(adj)` builds the structure over a forest represented by the bidirectional
  adjacency list `adj`, whose indices represent the nodes.
- `go_up(u, k)` returns the ancestor `k` edges above node `u`. When `k == 0`, it returns `u`; values
  larger than `u`'s depth stop at that tree's root.
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
#include <cassert>
#include <vector>

class BinaryLiftingLCA {
  std::vector<std::vector<int>> up;
  std::vector<int> tin, tout, depth, root;
  int len, timer;

  void dfs(const std::vector<std::vector<int>> &adj, int u, int p, int r, int d) {
    tin[u] = timer++;
    depth[u] = d;
    root[u] = r;
    up[u][0] = p;
    for (int i = 1; i < len; i++) {
      up[u][i] = up[up[u][i - 1]][i - 1];
    }
    for (int v : adj[u]) {
      if (v != p) {
        dfs(adj, v, u, r, d + 1);
      }
    }
    tout[u] = timer++;
  }

 public:
  explicit BinaryLiftingLCA(const std::vector<std::vector<int>> &adj)
      : tin(adj.size()), tout(adj.size()), depth(adj.size()), root(adj.size(), -1), timer(0) {
    int n = static_cast<int>(adj.size());
    len = 1;
    while ((1 << len) <= std::max(1, n)) {
      len++;
    }
    up.assign(n, std::vector<int>(len));
    for (int u = 0; u < n; u++) {
      if (root[u] == -1) {
        dfs(adj, u, u, u, 0);
      }
    }
  }

  bool is_ancestor(int parent, int child) const {
    assert(0 <= parent && parent < static_cast<int>(root.size()));
    assert(0 <= child && child < static_cast<int>(root.size()));
    return root[parent] == root[child] && tin[parent] <= tin[child] && tout[child] <= tout[parent];
  }

  int go_up(int u, int k) const {
    assert(0 <= u && u < static_cast<int>(root.size()) && k >= 0);
    k = std::min(k, depth[u]);
    for (int i = 0; i < len; i++) {
      if ((k & (1 << i)) != 0) {
        u = up[u][i];
      }
    }
    return u;
  }

  int lca(int u, int v) const {
    assert(0 <= u && u < static_cast<int>(root.size()));
    assert(0 <= v && v < static_cast<int>(root.size()));
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

int main() {
  // 0---1---2    5---6
  // |   |
  // 4   3
  vector<vector<int>> adj(7);
  auto add_edge = [&](int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  };
  add_edge(0, 1);
  add_edge(0, 4);
  add_edge(1, 2);
  add_edge(1, 3);
  add_edge(5, 6);
  BinaryLiftingLCA tree(adj);
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
