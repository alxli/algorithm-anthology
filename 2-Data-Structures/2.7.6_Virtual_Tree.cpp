/*

Given a rooted tree and a subset of its nodes, build the virtual tree (also called the auxiliary or
compressed tree): the smallest tree that contains every node of the subset together with the lowest
common ancestor of each pair, with chains of irrelevant degree-two ancestors compressed into single
edges. Many problems ask repeated queries each restricted to a handful of "important" nodes; the
virtual tree has O(k) nodes for a subset of size $k$, so an algorithm can run over it instead of the
whole tree, turning a per-query cost proportional to $n$ into one proportional to the subset size.

The construction sorts the subset by depth-first entry time, which lists the nodes in the order a
walk of the tree would meet them. The lowest common ancestor of any contiguous span of the subset is
the minimum-depth node among the pairwise LCAs of adjacent entries, so it suffices to add the LCA of
each adjacent pair, re-sort, and deduplicate. A single stack pass over the resulting nodes then
links each one to its parent in the virtual tree: ancestors that are no longer on the path to the
current node are popped, and the node on top becomes the parent. This implementation carries its own
binary lifting so it is self-contained; given an existing lowest common ancestor structure, the same
sort-and-stack logic applies directly.

- `VirtualTree(adj, root)` builds the structure over a tree rooted at `root`, given a bidirectional
  adjacency list `adj` of nodes numbered $[0, `n`)$, where `n` is `adj.size()`.
- `lca(u, v)` returns the lowest common ancestor of nodes `u` and `v`.
- `depth_of(u)` returns the depth of `u` in the original tree (the root has depth $0$). The original
  path length of a virtual-tree edge $(`p`, `c`)$ is `depth_of(c)` $-$ `depth_of(p)`.
- `build(nodes)` returns the virtual tree induced by `nodes`. The result has `root` set to the
  topmost node (the common ancestor of `nodes`), `vertices` listing every original node ID in the
  virtual tree sorted by entry time, and `edges` listing each $(`p`, `c`)$ pair in original IDs.
  Both the vertex set and the edges use the node numbering of the original tree. An empty input
  yields an empty result with `root` $= -1$.

Time Complexity:
- O(n log n) for construction, where $n$ is the number of nodes.
- O(log n) per call to `lca()`.
- O(k log k) per call to `build()`, where $k$ is the size of the input subset.

Space Complexity:
- O(n log n) to store the binary ancestor table.
- O(k) auxiliary plus result size per call to `build()`.

*/

#include <algorithm>
#include <utility>
#include <vector>

class VirtualTree {
  std::vector<std::vector<int>> up;
  std::vector<int> tin, tout, depth;
  int len, timer;

  void dfs(const std::vector<std::vector<int>> &adj, int u, int p, int d) {
    tin[u] = timer++;
    depth[u] = d;
    up[u][0] = p;
    for (int i = 1; i < len; i++) {
      up[u][i] = up[up[u][i - 1]][i - 1];
    }
    for (int v : adj[u]) {
      if (v != p) {
        dfs(adj, v, u, d + 1);
      }
    }
    tout[u] = timer++;
  }

 public:
  struct Tree {
    int root = -1;
    std::vector<int> vertices;
    std::vector<std::pair<int, int>> edges;  // (parent, child) in original node ids.
  };

  VirtualTree(const std::vector<std::vector<int>> &adj, int root) {
    int n = static_cast<int>(adj.size());
    len = 1;
    while ((1 << len) <= std::max(1, n)) {
      len++;
    }
    up.assign(n, std::vector<int>(len));
    tin.assign(n, 0);
    tout.assign(n, 0);
    depth.assign(n, 0);
    timer = 0;
    dfs(adj, root, root, 0);
  }

  bool is_ancestor(int parent, int child) const {
    return tin[parent] <= tin[child] && tout[child] <= tout[parent];
  }

  // Depth of u in the original tree; depth(child) - depth(parent) is the length of the original
  // path that a compressed virtual-tree edge stands for.
  int depth_of(int u) const { return depth[u]; }

  int lca(int u, int v) const {
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

  Tree build(std::vector<int> nodes) const {
    Tree res;
    if (nodes.empty()) {
      return res;
    }
    auto by_tin = [&](int a, int b) { return tin[a] < tin[b]; };
    std::sort(nodes.begin(), nodes.end(), by_tin);
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());
    int k = static_cast<int>(nodes.size());
    for (int i = 0; i + 1 < k; i++) {
      nodes.push_back(lca(nodes[i], nodes[i + 1]));
    }
    std::sort(nodes.begin(), nodes.end(), by_tin);
    nodes.erase(std::unique(nodes.begin(), nodes.end()), nodes.end());

    std::vector<int> stack;
    for (int v : nodes) {
      while (!stack.empty() && !is_ancestor(stack.back(), v)) {
        stack.pop_back();
      }
      if (!stack.empty()) {
        res.edges.emplace_back(stack.back(), v);
      }
      stack.push_back(v);
    }
    res.root = nodes.front();
    res.vertices = std::move(nodes);
    return res;
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
  //          0
  //        / |
  //       1  2
  //      /|  |
  //  6--3 4  5--7
  vector<vector<int>> adj(8);
  add_edge(adj, 0, 1);
  add_edge(adj, 0, 2);
  add_edge(adj, 1, 3);
  add_edge(adj, 1, 4);
  add_edge(adj, 2, 5);
  add_edge(adj, 3, 6);
  add_edge(adj, 5, 7);

  VirtualTree vt(adj, 0);
  assert(vt.lca(6, 4) == 1);
  assert(vt.lca(6, 7) == 0);
  assert(vt.depth_of(0) == 0 && vt.depth_of(6) == 3);
  // The compressed edge 0 -> 7 stands for an original path of length 3 (0-2-5-7).
  assert(vt.depth_of(7) - vt.depth_of(0) == 3);

  // Virtual tree of {6, 7}: their LCA is 0, so the tree is 0 -> {6, 7} with edges compressed.
  //
  //      0
  //   +--+--+
  //   6     7
  VirtualTree::Tree t = vt.build({6, 7});
  assert(t.root == 0);
  assert((t.vertices == vector<int>{0, 6, 7}));
  assert(t.edges.size() == 2);
  for (auto &e : t.edges) {
    assert(e.first == 0 && (e.second == 6 || e.second == 7));
  }

  // Virtual tree of {6, 4, 7}: node 1 becomes a branch point (LCA of 6 and 4).
  //
  //      0
  //   +--+--+
  //   1     7
  // +-+-+
  // 6   4
  VirtualTree::Tree u = vt.build({6, 4, 7});
  assert(u.root == 0);
  assert((u.vertices == vector<int>{0, 1, 6, 4, 7}));
  // Expected edges: 0->1, 1->6, 1->4, 0->7.
  vector<pair<int, int>> want{{0, 1}, {1, 6}, {1, 4}, {0, 7}};
  auto got = u.edges;
  sort(got.begin(), got.end());
  sort(want.begin(), want.end());
  assert(got == want);

  // A single node is its own virtual tree with no edges.
  VirtualTree::Tree s = vt.build({4});
  assert(s.root == 4 && s.edges.empty() && s.vertices == vector<int>{4});
  return 0;
}
