/*

Given a tree, build its centroid decomposition. A centroid is a node whose removal splits the
current subtree into connected components of size at most half of that subtree. Recursively choosing
centroids creates a decomposition tree of height O(log n), which is useful for queries based on
distances to marked nodes, nearest special node, and other "path through a centroid" problems.

- `CentroidDecomposition(adj)` builds the centroid decomposition of the tree given by a
  bidirectional adjacency list `adj`, which must form a valid tree with `n` nodes numbered [0, `n`),
  where `n` is `adj.size()`.
- `parent(u)` returns the parent of node `u` in the centroid tree, or $-1$ if `u` is the root
  centroid.

Time Complexity:
- O(n log n) for construction, where $n$ is the number of nodes.

Space Complexity:
- O(n) for storage of the centroid tree.
- O(n) auxiliary stack space for the recursive searches.

*/

#include <vector>

class CentroidDecomposition {
  std::vector<std::vector<int>> adj;
  std::vector<int> subtree_size, par;
  std::vector<char> removed;

  int get_size(int u, int p) {
    subtree_size[u] = 1;
    for (int v : adj[u]) {
      if (v != p && !removed[v]) {
        subtree_size[u] += get_size(v, u);
      }
    }
    return subtree_size[u];
  }

  int get_centroid(int u, int p, int total) {
    for (int v : adj[u]) {
      if (v != p && !removed[v] && subtree_size[v] > total / 2) {
        return get_centroid(v, u, total);
      }
    }
    return u;
  }

  void decompose(int entry, int parent) {
    int total = get_size(entry, -1);
    int centroid = get_centroid(entry, -1, total);
    par[centroid] = parent;
    removed[centroid] = true;
    for (int v : adj[centroid]) {
      if (!removed[v]) {
        decompose(v, centroid);
      }
    }
  }

 public:
  explicit CentroidDecomposition(const std::vector<std::vector<int>> &adj)
      : adj(adj), subtree_size(adj.size()), par(adj.size(), -1), removed(adj.size()) {
    if (adj.size() > 0) {
      decompose(0, -1);
    }
  }

  int parent(int u) const { return par[u]; }
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
  add_edge(adj, 1, 2);
  add_edge(adj, 1, 3);
  add_edge(adj, 3, 4);
  add_edge(adj, 3, 5);
  add_edge(adj, 5, 6);

  CentroidDecomposition tree(adj);
  assert(tree.parent(3) == -1);
  assert(tree.parent(1) == 3);
  assert(tree.parent(5) == 3);
  return 0;
}
