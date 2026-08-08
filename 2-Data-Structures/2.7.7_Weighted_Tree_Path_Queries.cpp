/*

Given a tree or forest with weighted edges, aggregate the edge weights along the path between any
two nodes in the same tree. The input is undirected; construction roots each component at the first
unvisited node in index order. This builds on the same binary lifting table as LCA: for every valid
$2^i$-edge jump from `u`, it stores `agg[u][i]`, the combined weight of the edges traversed by that
jump. A path query splits the path `u`-`v` at their lowest common ancestor and merges the two upward
climbs.

The aggregate is defined by a commutative, associative `combine()`. The default computes the maximum
edge weight on the path. For minimum queries, use `std::min`; for sums, use `a + b`. Commutativity
is required because the two half-paths are merged without regard to orientation. For an invertible
aggregate such as the sum, an alternative that avoids the `agg` table is to store each node's
weighted depth `dw[u]` and return `dw[u] + dw[v] - 2*dw[lca]`.

- `TreePathQueries<W>(adj)` builds the structure over a forest given by a weighted, bidirectional
  adjacency list `adj`, whose indices represent the nodes and where `adj[u]` holds pairs (`v`, `w`)
  for each edge `u`-`v` of weight `w`. Each connected component is rooted at its first node reached
  by the constructor's outer loop.
- `lca(u, v)` returns the lowest common ancestor of `u` and `v`, or $-1$ if they lie in different
  trees.
- `kth_ancestor(u, k)` returns the $k$-th ancestor of `u`, stopping at that tree's root if `k`
  exceeds `u`'s depth.
- `is_ancestor(parent, child)` returns whether `parent` is an ancestor of `child`.
- `path_query(u, v)` returns the combined weight of the edges on the path from `u` to `v`. The nodes
  must be distinct and lie in the same tree.

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
#include <cassert>
#include <optional>
#include <utility>
#include <vector>

template<typename W>
class TreePathQueries {
  static W combine(const W &a, const W &b) { return std::max(a, b); }

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
      if ((1 << i) <= d) {
        agg[u].push_back(combine(agg[u][i - 1], agg[up[u][i - 1]][i - 1]));
      }
    }
    for (const auto &[v, w] : adj[u]) {
      if (v != p) {
        agg[v].push_back(w);  // The edge into a child is stored on the child.
        dfs(v, u, r, d + 1, adj);
      }
    }
    tout[u] = timer++;
  }

  // Combined weight of the k edges on the upward path of k steps from u.
  std::optional<W> climb(int u, int k) const {
    std::optional<W> res;
    for (int i = 0; i < len; i++) {
      if ((k & (1 << i)) != 0) {
        res = res ? combine(*res, agg[u][i]) : agg[u][i];
        u = up[u][i];
      }
    }
    return res;
  }

 public:
  explicit TreePathQueries(const std::vector<std::vector<std::pair<int, W>>> &adj)
      : agg(adj.size()),
        tin(adj.size()),
        tout(adj.size()),
        depth(adj.size()),
        root(adj.size(), -1),
        timer(0) {
    int n = static_cast<int>(adj.size());
    len = 1;
    while ((1 << len) <= std::max(1, n)) {
      len++;
    }
    up.assign(n, std::vector<int>(len));
    for (int u = 0; u < n; u++) {
      if (root[u] == -1) {
        dfs(u, u, u, 0, adj);
      }
    }
  }

  bool is_ancestor(int parent, int child) const {
    assert(0 <= parent && parent < static_cast<int>(root.size()));
    assert(0 <= child && child < static_cast<int>(root.size()));
    return root[parent] == root[child] && tin[parent] <= tin[child] && tout[child] <= tout[parent];
  }

  int kth_ancestor(int u, int k) const {
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

  W path_query(int u, int v) const {
    int l = lca(u, v);
    assert(l != -1 && u != v);
    std::optional<W> left = climb(u, depth[u] - depth[l]);
    std::optional<W> right = climb(v, depth[v] - depth[l]);
    if (!left) {
      return *right;
    }
    if (!right) {
      return *left;
    }
    return combine(*left, *right);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //             0
  //     w=4 /       \ w=2
  //       1           2
  //  w=7 / \w=1   w=5/ \ w=3
  //     3   4       5   6
  vector<vector<pair<int, int>>> adj(7);
  auto add_edge = [&](int u, int v, int w) {
    adj[u].emplace_back(v, w);
    adj[v].emplace_back(u, w);
  };
  add_edge(0, 1, 4);
  add_edge(0, 2, 2);
  add_edge(1, 3, 7);
  add_edge(1, 4, 1);
  add_edge(2, 5, 5);
  add_edge(2, 6, 3);
  TreePathQueries<int> t(adj);
  assert(t.lca(3, 4) == 1);
  assert(t.lca(5, 6) == 2);
  assert(t.lca(3, 6) == 0);
  assert(t.kth_ancestor(3, 1) == 1);
  assert(t.kth_ancestor(3, 2) == 0);
  assert(t.kth_ancestor(3, 5) == 0);  // Clamped to the root.
  // Default combine() reports the maximum edge weight on the path.
  assert(t.path_query(3, 4) == 7);  // 3-1 (7), 1-4 (1).
  assert(t.path_query(5, 6) == 5);  // 5-2 (5), 2-6 (3).
  assert(t.path_query(4, 5) == 5);  // 4-1 (1), 1-0 (4), 0-2 (2), 2-5 (5).
  assert(t.path_query(0, 3) == 7);  // 0-1 (4), 1-3 (7).

  vector<vector<pair<int, int>>> forest(2);
  TreePathQueries<int> disconnected(forest);
  assert(disconnected.lca(0, 1) == -1);
  return 0;
}
