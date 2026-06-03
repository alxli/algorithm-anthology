/*

Given a tree, build its centroid decomposition. A centroid is a node whose
removal splits the current subtree into connected components of size at most
half of that subtree. Recursively choosing centroids creates a decomposition
tree of height O(log n), which is useful for queries based on distances to marked
nodes, nearest special node, and other "path through a centroid" problems.

`build_centroid_tree()` applies to a global, pre-populated adjacency list `adj[]`
which satisfies the precondition that for every node $v$ in `adj[u]`, node $u$
also exists in `adj[v]`.

- `centroid_parent[u]` stores the parent of node $u$ in the centroid tree.
- The root centroid has parent `-1`.

Time Complexity:
- O(n log n) per call to `build_centroid_tree()`, where $n$ is the number of
  nodes.

Space Complexity:
- O(n) for `subtree_size[]`, `removed[]`, and `centroid_parent[]`.
- O(n) auxiliary stack space for the recursive searches.

*/

#include <algorithm>
#include <vector>

const int MAXN = 100;
std::vector<int> adj[MAXN];
int subtree_size[MAXN], centroid_parent[MAXN];
bool removed[MAXN];

int get_size(int u, int p) {
  subtree_size[u] = 1;
  for (int i = 0; i < (int)adj[u].size(); i++) {
    int v = adj[u][i];
    if (v != p && !removed[v]) {
      subtree_size[u] += get_size(v, u);
    }
  }
  return subtree_size[u];
}

int get_centroid(int u, int p, int total) {
  for (int i = 0; i < (int)adj[u].size(); i++) {
    int v = adj[u][i];
    if (v != p && !removed[v] && subtree_size[v] > total / 2) {
      return get_centroid(v, u, total);
    }
  }
  return u;
}

void decompose(int entry, int parent) {
  int total = get_size(entry, -1);
  int centroid = get_centroid(entry, -1, total);
  centroid_parent[centroid] = parent;
  removed[centroid] = true;
  for (int i = 0; i < (int)adj[centroid].size(); i++) {
    int v = adj[centroid][i];
    if (!removed[v]) {
      decompose(v, centroid);
    }
  }
}

void build_centroid_tree(int nodes) {
  std::fill(removed, removed + nodes, false);
  std::fill(centroid_parent, centroid_parent + nodes, -1);
  decompose(0, -1);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(1, 3);
  add_edge(3, 4);
  add_edge(3, 5);
  add_edge(5, 6);
  build_centroid_tree(7);
  assert(centroid_parent[3] == -1);
  assert(centroid_parent[1] == 3);
  assert(centroid_parent[5] == 3);
  return 0;
}
