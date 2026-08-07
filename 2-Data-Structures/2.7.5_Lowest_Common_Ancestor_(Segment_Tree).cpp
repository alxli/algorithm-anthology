/*

Given a rooted tree or forest, determine the lowest common ancestor of any two nodes in the same
tree. The lowest common ancestor of two nodes $u$ and $v$ is the node that has the greatest depth
while having both $u$ and $v$ as descendants. A node is considered to be a descendant of itself.

This reduces LCA to a range-minimum query. An Euler tour of the forest records, at each visit to a
node, that node's depth; the lowest common ancestor of $u$ and $v$ in the same tree is the
shallowest node visited between their first occurrences in the tour, found by a range-minimum query
over the depth sequence. This version answers those queries with a segment tree.

- `SegTreeLCA(adj)` builds the structure over a forest represented by the bidirectional adjacency
  list `adj`, whose indices represent the nodes.
- `lca(u, v)` returns the lowest common ancestor of nodes `u` and `v`, or $-1$ if they are in
  different trees.
- `dist(u, v)` returns the number of edges on the path between nodes `u` and `v`, or $-1$ if they
  are in different trees.

Time Complexity:
- O(n) for construction, where $n$ is the number of nodes.
- O(log n) per call to `lca()` and `dist()`.

Space Complexity:
- O(n) for storage of the segment tree.
- O(n) auxiliary stack space for construction.
- O(log n) auxiliary stack space for `lca()`.

*/

#include <algorithm>
#include <cassert>
#include <vector>

class SegTreeLCA {
  std::vector<int> root, depth, first, order, minpos;
  int len;

  void dfs(const std::vector<std::vector<int>> &adj, int u, int r, int d) {
    root[u] = r;
    depth[u] = d;
    first[u] = static_cast<int>(order.size());
    order.push_back(u);
    for (int v : adj[u]) {
      if (depth[v] == -1) {
        dfs(adj, v, r, d + 1);
        order.push_back(u);
      }
    }
  }

  int better(int a, int b) const { return depth[a] < depth[b] ? a : b; }

  void build(int i, int lo, int hi) {
    if (lo == hi) {
      minpos[i] = order[lo];
      return;
    }
    int mid = lo + (hi - lo) / 2;
    int l = i * 2 + 1, r = i * 2 + 2;
    build(l, lo, mid);
    build(r, mid + 1, hi);
    minpos[i] = better(minpos[l], minpos[r]);
  }

  int get_minpos(int a, int b, int i, int lo, int hi) const {
    if (a == lo && b == hi) {
      return minpos[i];
    }
    int mid = lo + (hi - lo) / 2;
    int l = i * 2 + 1, r = i * 2 + 2;
    if (b <= mid) {
      return get_minpos(a, b, l, lo, mid);
    }
    if (mid < a) {
      return get_minpos(a, b, r, mid + 1, hi);
    }
    return better(get_minpos(a, mid, l, lo, mid), get_minpos(mid + 1, b, r, mid + 1, hi));
  }

 public:
  explicit SegTreeLCA(const std::vector<std::vector<int>> &adj)
      : root(adj.size(), -1), depth(adj.size(), -1), first(adj.size(), -1) {
    int n = static_cast<int>(adj.size());
    for (int u = 0; u < n; u++) {
      if (depth[u] == -1) {
        dfs(adj, u, u, 0);
      }
    }
    len = static_cast<int>(order.size());
    minpos.assign(std::max(1, 4 * len), 0);
    if (len > 0) {
      build(0, 0, len - 1);
    }
  }

  int lca(int u, int v) const {
    assert(0 <= u && u < static_cast<int>(root.size()));
    assert(0 <= v && v < static_cast<int>(root.size()));
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
