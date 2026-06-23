/*

Given a rooted tree or forest with weighted edges, aggregate the edge weights along the path between
any two nodes in the same tree. This builds on the same binary lifting table as LCA: alongside
`up[u][i]`, the $2^i$-th ancestor of `u`, it stores `agg[u][i]`, the combined weight of the $2^i$
edges traversed by that jump. A path query splits the path `u`-`v` at their lowest common ancestor
and merges the two upward climbs.

The aggregate is a commutative monoid defined by `combine()` and its identity `identity()`. The two
must be edited together; the defaults below compute the maximum edge weight on the path. For the
minimum, use `std::min` with `std::numeric_limits<W>::max()`; for the sum (the path length), use
`a + b` with `W()`. Commutativity is required because the two half-paths are merged without regard
to orientation. For an invertible aggregate such as the sum, an alternative that avoids the `agg`
table is to store each node's weighted depth `dw[u]` and return `dw[u] + dw[v] - 2*dw[lca]`.

- `WeightedTreePath<W>(adj)` builds the structure over a forest given by a weighted, bidirectional
  adjacency list `adj`, where `adj[u]` holds `{v, w}` pairs for each edge `u`-`v` of weight `w`,
  over nodes numbered [$0$, `n`), where `n` is `adj.size()`.
- `lca(u, v)` returns the lowest common ancestor of `u` and `v`, or $-1$ if they lie in different
  trees.
- `kth_ancestor(u, k)` returns the $k$-th ancestor of `u`, stopping at that tree's root if `k`
  exceeds `u`'s depth.
- `is_ancestor(parent, child)` returns whether `parent` is an ancestor of `child`.
- `path_query(u, v)` returns the combined weight of the edges on the path from `u` to `v`, or
  `identity()` if they lie in different trees. If `u` equals `v` then the path has no edges, so
  `identity()` is returned.

Time Complexity:
- O(n log n) for construction, where $n$ is the number of nodes.
- O(log n) per call to `lca()`, `kth_ancestor()`, and `path_query()`.
- O(1) per call to `is_ancestor()`.

Space Complexity:
- O(n log n) to store the binary ancestor and aggregate tables.
- O(n) auxiliary stack space for construction.
- O(1) auxiliary for `lca()`, `kth_ancestor()`, `is_ancestor()`, and `path_query()`.

*/

#include <algorithm>
#include <limits>
#include <utility>
#include <vector>

template<typename W>
class WeightedTreePath {
  // Edit combine() and identity() together for the desired commutative monoid.
  // The defaults give the maximum edge weight on the path; for the minimum use std::min with
  // std::numeric_limits<W>::max(), and for the sum (path length) use a + b with W().
  static W combine(const W &a, const W &b) { return std::max(a, b); }
  static W identity() { return std::numeric_limits<W>::lowest(); }

  std::vector<std::vector<int>> up;
  std::vector<std::vector<W>> agg;  // agg[u][i] = combined weight of the 2^i edges above u.
  std::vector<int> tin, tout, depth, root;
  int len, timer;

  void dfs(int u, int p, int r, int d, const std::vector<std::vector<std::pair<int, W>>> &adj) {
    tin[u] = timer++;
    depth[u] = d;
    root[u] = r;
    up[u][0] = p;
    for (int i = 1; i < len; i++) {
      up[u][i] = up[up[u][i - 1]][i - 1];
      agg[u][i] = combine(agg[u][i - 1], agg[up[u][i - 1]][i - 1]);
    }
    for (const auto &[v, w] : adj[u]) {
      if (v != p) {
        agg[v][0] = w;  // The edge into a child is stored on the child.
        dfs(v, u, r, d + 1, adj);
      }
    }
    tout[u] = timer++;
  }

  // Combined weight of the k edges on the upward path of k steps from u.
  W climb(int u, int k) const {
    W res = identity();
    for (int i = 0; i < len; i++) {
      if ((k & (1 << i)) != 0) {
        res = combine(res, agg[u][i]);
        u = up[u][i];
      }
    }
    return res;
  }

 public:
  explicit WeightedTreePath(const std::vector<std::vector<std::pair<int, W>>> &adj) : timer(0) {
    int n = static_cast<int>(adj.size());
    len = 1;
    while ((1 << len) <= std::max(1, n)) {
      len++;
    }
    up.assign(n, std::vector<int>(len));
    agg.assign(n, std::vector<W>(len, identity()));
    tin.assign(n, 0);
    tout.assign(n, 0);
    depth.assign(n, 0);
    root.assign(n, -1);
    for (int u = 0; u < n; u++) {
      if (root[u] == -1) {
        dfs(u, u, u, 0, adj);
      }
    }
  }

  bool is_ancestor(int parent, int child) const {
    return root[parent] == root[child] && tin[parent] <= tin[child] && tout[child] <= tout[parent];
  }

  int kth_ancestor(int u, int k) const {
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

  W path_query(int u, int v) const {
    int l = lca(u, v);
    if (l == -1) {
      return identity();
    }
    return combine(climb(u, depth[u] - depth[l]), climb(v, depth[v] - depth[l]));
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(vector<vector<pair<int, int>>> &adj, int u, int v, int w) {
  adj[u].push_back({v, w});
  adj[v].push_back({u, w});
}

int main() {
  //            0
  //      (4) /   \ (2)
  //         1     2
  //   (7) / | (1) | \ (3)
  //      3  4  (5)5  6
  vector<vector<pair<int, int>>> adj(7);
  add_edge(adj, 0, 1, 4);
  add_edge(adj, 0, 2, 2);
  add_edge(adj, 1, 3, 7);
  add_edge(adj, 1, 4, 1);
  add_edge(adj, 2, 5, 5);
  add_edge(adj, 2, 6, 3);
  WeightedTreePath<int> tree(adj);

  assert(tree.lca(3, 4) == 1);
  assert(tree.lca(5, 6) == 2);
  assert(tree.lca(3, 6) == 0);

  assert(tree.kth_ancestor(3, 1) == 1);
  assert(tree.kth_ancestor(3, 2) == 0);
  assert(tree.kth_ancestor(3, 5) == 0);  // Clamped to the root.

  // Default combine() reports the maximum edge weight on the path.
  assert(tree.path_query(3, 4) == 7);  // 3-1 (7), 1-4 (1).
  assert(tree.path_query(5, 6) == 5);  // 5-2 (5), 2-6 (3).
  assert(tree.path_query(4, 5) == 5);  // 4-1 (1), 1-0 (4), 0-2 (2), 2-5 (5).
  assert(tree.path_query(0, 3) == 7);  // 0-1 (4), 1-3 (7).

  vector<vector<pair<int, int>>> forest(2);
  WeightedTreePath<int> disconnected(forest);
  assert(disconnected.lca(0, 1) == -1);
  assert(disconnected.path_query(0, 1) == numeric_limits<int>::lowest());
  return 0;
}
