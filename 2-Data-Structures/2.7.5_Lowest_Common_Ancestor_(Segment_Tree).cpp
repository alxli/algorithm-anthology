/*

Given a rooted tree or forest, determine the lowest common ancestor of any two nodes in the same
tree. The lowest common ancestor of two nodes $u$ and $v$ is the node that has the greatest depth
while having both $u$ and $v$ as descendants. A node is considered to be a descendant of itself.

This reduces LCA to a range-minimum query. An Euler tour of the forest records, at each visit to a
node, that node's depth; the lowest common ancestor of $u$ and $v$ in the same tree is the
shallowest node visited between their first occurrences in the tour, found by a range-minimum query
over the depth sequence. This version answers those queries with a segment tree.

- `SegTreeLCA(adj)` builds the structure over a forest represented by a bidirectional adjacency list
  `adj` of nodes numbered $[0, `n`)$, where `n` is `adj.size()`.
- `lca(u, v)` returns the lowest common ancestor of nodes `u` and `v`, or $-1$ if they are in
  different trees.
- `dist(u, v)` returns the number of edges on the path between nodes `u` and `v`, or $-1$ if they
  are in different trees.

Time Complexity:
- O(n log n) for construction, where $n$ is the number of nodes.
- O(log n) per call to `lca()` and `dist()`.

Space Complexity:
- O(n) for storage of the segment tree.
- O(n) auxiliary stack space for construction.
- O(log n) auxiliary stack space for `lca()`.

*/

#include <algorithm>
#include <vector>

class SegTreeLCA {
  std::vector<std::vector<int>> adj;
  std::vector<int> depth, order, first, root, minpos;
  int len;

  void dfs(int u, int r, int d) {
    root[u] = r;
    depth[u] = d;
    first[u] = static_cast<int>(order.size());
    order.push_back(u);
    for (int v : adj[u]) {
      if (depth[v] == -1) {
        dfs(v, r, d + 1);
        order.push_back(u);
      }
    }
  }

  int better(int a, int b) const { return depth[a] < depth[b] ? a : b; }

  void build(int node, int lo, int hi) {
    if (lo == hi) {
      minpos[node] = order[lo];
      return;
    }
    int mid = lo + (hi - lo) / 2;
    build(2 * node + 1, lo, mid);
    build(2 * node + 2, mid + 1, hi);
    minpos[node] = better(minpos[2 * node + 1], minpos[2 * node + 2]);
  }

  int get_minpos(int a, int b, int node, int lo, int hi) const {
    if (a == lo && b == hi) {
      return minpos[node];
    }
    int mid = lo + (hi - lo) / 2;
    if (b <= mid) {
      return get_minpos(a, b, 2 * node + 1, lo, mid);
    }
    if (mid < a) {
      return get_minpos(a, b, 2 * node + 2, mid + 1, hi);
    }
    return better(
        get_minpos(a, mid, 2 * node + 1, lo, mid), get_minpos(mid + 1, b, 2 * node + 2, mid + 1, hi)
    );
  }

 public:
  explicit SegTreeLCA(const std::vector<std::vector<int>> &adj) : adj(adj) {
    int n = static_cast<int>(adj.size());
    depth.assign(n, -1);
    first.assign(n, -1);
    root.assign(n, -1);
    for (int u = 0; u < n; u++) {
      if (depth[u] == -1) {
        dfs(u, u, 0);
      }
    }
    len = static_cast<int>(order.size());
    minpos.assign(std::max(1, 4 * len), 0);
    if (len > 0) {
      build(0, 0, len - 1);
    }
  }

  int lca(int u, int v) const {
    if (root[u] != root[v]) {
      return -1;
    }
    return get_minpos(std::min(first[u], first[v]), std::max(first[u], first[v]), 0, 0, len - 1);
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

  SegTreeLCA tree(adj);
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
