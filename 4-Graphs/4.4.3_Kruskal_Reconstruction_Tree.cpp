/*

Given a connected, undirected, weighted graph, build the Kruskal reconstruction tree of its minimum
spanning tree. Kruskal's algorithm adds MST edges in nondecreasing weight order; whenever it joins
two components, create a new internal node whose children are the two component roots and whose
value is the joining edge weight. The original graph nodes become leaves.

The lowest common ancestor of two leaves is exactly the merge step that first connects their MST
components, so its value is the maximum edge weight on the MST path between those two nodes. This
turns bottleneck path queries into ordinary LCA queries on the reconstruction tree.

- `KruskalReconstructionTree(n, edges)` builds the tree from weighted edges stored as
  `(weight, u, v)`.
- `max_edge_on_path(u, v)` returns the maximum edge weight on the MST path between original nodes
  `u` and `v`.
- `root()` returns the reconstruction-tree root.

The input graph must be connected. Parallel edges are supported and are handled naturally by
Kruskal's algorithm.

Time Complexity:
- O(m log m + n log n) for construction, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(log n) per call to `max_edge_on_path()`.

Space Complexity:
- O(n log n + m) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <numeric>
#include <tuple>
#include <vector>

class KruskalReconstructionTree {
  std::vector<int> dsu_root, dsu_tree_root, depth, tin, tout;
  std::vector<std::vector<int>> up;
  std::vector<std::vector<int>> tree;
  std::vector<int64_t> value;
  int timer = 0, root_node = -1;

  int find_root(int u) {
    if (dsu_root[u] != u) {
      dsu_root[u] = find_root(dsu_root[u]);
    }
    return dsu_root[u];
  }

  void dfs_lca(int u, int p) {
    tin[u] = timer++;
    up[0][u] = p;
    for (int k = 1; k < static_cast<int>(up.size()); k++) {
      up[k][u] = up[k - 1][up[k - 1][u]];
    }
    for (int v : tree[u]) {
      depth[v] = depth[u] + 1;
      dfs_lca(v, u);
    }
    tout[u] = timer;
  }

  bool is_ancestor(int u, int v) const { return tin[u] <= tin[v] && tout[v] <= tout[u]; }

  int lca(int u, int v) const {
    if (is_ancestor(u, v)) {
      return u;
    }
    if (is_ancestor(v, u)) {
      return v;
    }
    for (int k = static_cast<int>(up.size()) - 1; k >= 0; k--) {
      if (!is_ancestor(up[k][u], v)) {
        u = up[k][u];
      }
    }
    return up[0][u];
  }

 public:
  KruskalReconstructionTree(int n, std::vector<std::tuple<int64_t, int, int>> edges)
      : dsu_root(2 * n - 1),
        dsu_tree_root(2 * n - 1),
        depth(2 * n - 1),
        tin(2 * n - 1),
        tout(2 * n - 1),
        tree(2 * n - 1),
        value(2 * n - 1) {
    assert(n > 0);
    std::iota(dsu_root.begin(), dsu_root.begin() + n, 0);
    std::iota(dsu_tree_root.begin(), dsu_tree_root.begin() + n, 0);
    std::sort(edges.begin(), edges.end());
    int nodes = n;
    for (const auto &[w, a, b] : edges) {
      int u = find_root(a), v = find_root(b);
      if (u == v) {
        continue;
      }
      value[nodes] = w;
      tree[nodes].push_back(dsu_tree_root[u]);
      tree[nodes].push_back(dsu_tree_root[v]);
      dsu_root[u] = nodes;
      dsu_root[v] = nodes;
      dsu_root[nodes] = nodes;
      dsu_tree_root[nodes] = nodes;
      nodes++;
    }
    assert(nodes == 2 * n - 1);
    root_node = nodes - 1;
    int lg = 1;
    while ((1 << lg) <= nodes) {
      lg++;
    }
    up.assign(lg, std::vector<int>(nodes));
    dfs_lca(root_node, root_node);
  }

  int root() const { return root_node; }
  int64_t max_edge_on_path(int u, int v) const { return value[lca(u, v)]; }
};

/*** Example Usage ***/

int main() {
  // MST edges are 0-1 (2), 1-2 (4), and 1-3 (5).
  std::vector<std::tuple<int64_t, int, int>> edges{
      {2, 0, 1}, {4, 1, 2}, {5, 1, 3}, {8, 0, 2}, {9, 2, 3}
  };
  KruskalReconstructionTree tree(4, edges);
  assert(tree.root() == 6);
  assert(tree.max_edge_on_path(0, 2) == 4);
  assert(tree.max_edge_on_path(0, 3) == 5);
  assert(tree.max_edge_on_path(2, 3) == 5);
  return 0;
}
