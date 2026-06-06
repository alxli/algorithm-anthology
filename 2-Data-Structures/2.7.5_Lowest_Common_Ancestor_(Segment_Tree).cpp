/*

Given a tree, determine the lowest common ancestor of any two nodes in the tree. The lowest common
ancestor of two nodes $u$ and $v$ is the node that has the longest distance from the root while
having both $u$ and $v$ as its descendant. A node is considered to be a descendant of itself.
`build()` applies to a global, pre-populated adjacency list `adj` which must only consist of nodes
numbered with integers between 0 (inclusive) and the total number of nodes (exclusive), as passed in
the function argument. The adjacency list must define one connected tree rooted at `root`.

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

void build(int nodes, int root) {
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

int main() {
  int nodes = 5;
  adj.resize(nodes);
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[3].push_back(1);
  adj[1].push_back(3);
  adj[0].push_back(4);
  adj[4].push_back(0);
  build(nodes, 0);
  assert(lca(3, 2) == 1);
  assert(lca(2, 4) == 0);
  return 0;
}
