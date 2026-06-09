/*

Given a tree, determine the lowest common ancestor of any two nodes. The lowest common ancestor of
two nodes $u$ and $v$ is the node that has the longest distance from the root while having both $u$
and $v$ as its descendant. A node is considered to be a descendant of itself.

This reduces LCA to a range-minimum query. An Euler tour of the tree records, at each visit to a
node, that node's depth; the lowest common ancestor of $u$ and $v$ is the shallowest node visited
between their first occurrences in the tour, found by a range-minimum query over the depth sequence.
This version answers those queries with a segment tree.

- `build(root)` builds the structure over a global, bidirectionally pre-populated adjacency list
  `adj` of nodes numbered 0 to `adj.size() - 1`, which must define one connected tree rooted at
  `root` (default 0).
- `lca(u, v)` returns the lowest common ancestor of nodes `u` and `v`.

Time Complexity:
- O(n log n) per call to `build()`, where $n$ is the number of nodes.
- O(log n) per call to `lca()`.

Space Complexity:
- O(n) for storage of the segment tree, where $n$ is the number of nodes.
- O(n) auxiliary stack space for `build()`.
- O(log n) auxiliary stack space for `lca()`.

*/

#include <algorithm>
#include <vector>

std::vector<std::vector<int>> adj;
int len, counter;
std::vector<int> depth, dfs_order, first, minpos;

void dfs(int u, int d) {
  depth[u] = d;
  dfs_order[counter++] = u;
  for (int v : adj[u]) {
    if (depth[v] == -1) {
      dfs(v, d + 1);
      dfs_order[counter++] = u;
    }
  }
}

void build(int n, int lo, int hi) {
  if (lo == hi) {
    minpos[n] = dfs_order[lo];
    return;
  }
  int lchild = 2 * n + 1, rchild = 2 * n + 2, mid = lo + (hi - lo) / 2;
  build(lchild, lo, mid);
  build(rchild, mid + 1, hi);
  minpos[n] = depth[minpos[lchild]] < depth[minpos[rchild]] ? minpos[lchild] : minpos[rchild];
}

void build(int root = 0) {
  int nodes = static_cast<int>(adj.size());
  depth.assign(nodes, -1);
  dfs_order.assign(2 * nodes, 0);
  first.assign(nodes, -1);
  minpos.assign(8 * nodes, 0);
  len = 2 * nodes - 1;
  counter = 0;
  dfs(root, 0);
  build(0, 0, len - 1);
  for (int i = 0; i < len; i++) {
    if (first[dfs_order[i]] == -1) {
      first[dfs_order[i]] = i;
    }
  }
}

int get_minpos(int a, int b, int n, int lo, int hi) {
  if (a == lo && b == hi) {
    return minpos[n];
  }
  int mid = lo + (hi - lo) / 2;
  if (a <= mid && mid < b) {
    int p1 = get_minpos(a, std::min(b, mid), 2 * n + 1, lo, mid);
    int p2 = get_minpos(std::max(a, mid + 1), b, 2 * n + 2, mid + 1, hi);
    return depth[p1] < depth[p2] ? p1 : p2;
  }
  if (a <= mid) {
    return get_minpos(a, std::min(b, mid), 2 * n + 1, lo, mid);
  }
  return get_minpos(std::max(a, mid + 1), b, 2 * n + 2, mid + 1, hi);
}

int lca(int u, int v) {
  return get_minpos(std::min(first[u], first[v]), std::max(first[u], first[v]), 0, 0, len - 1);
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
