/*

Given a tree, build its centroid decomposition. A centroid is a node whose removal splits the
current subtree into connected components of size at most half of that subtree. Recursively choosing
centroids creates a decomposition tree of height O(log n), which is useful for queries based on
distances to marked nodes, nearest special node, and other "path through a centroid" problems.

- `centroid_decomposition()` uses the tree in the global bidirectional adjacency list `adj` and
  returns a vector `parent`, where `parent[u]` is the parent of node `u` in the centroid tree, or
  $-1$ if `u` is the root centroid.

Time Complexity:
- O(n log n) for construction, where $n$ is the number of nodes.

Space Complexity:
- O(n) for storage of the centroid tree.
- O(n) auxiliary stack space for the recursive searches.

*/

#include <vector>

std::vector<std::vector<int>> adj;

std::vector<int> centroid_decomposition() {
  int n = static_cast<int>(adj.size());
  std::vector<int> subtree_size(n), parent(n, -1);
  std::vector<char> removed(n);
  auto get_size = [&](auto &&get_size, int u, int p) -> int {
    subtree_size[u] = 1;
    for (int v : adj[u]) {
      if (v != p && !removed[v]) {
        subtree_size[u] += get_size(get_size, v, u);
      }
    }
    return subtree_size[u];
  };
  auto get_centroid = [&](auto &&get_centroid, int u, int p, int total) -> int {
    for (int v : adj[u]) {
      if (v != p && !removed[v] && subtree_size[v] > total / 2) {
        return get_centroid(get_centroid, v, u, total);
      }
    }
    return u;
  };
  auto decompose = [&](auto &&decompose, int entry, int p) -> void {
    int total = get_size(get_size, entry, -1);
    int centroid = get_centroid(get_centroid, entry, -1, total);
    parent[centroid] = p;
    removed[centroid] = true;
    for (int v : adj[centroid]) {
      if (!removed[v]) {
        decompose(decompose, v, centroid);
      }
    }
  };
  if (n > 0) {
    decompose(decompose, 0, -1);
  }
  return parent;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
#include <climits>
using namespace std;

// Example-only distance helper; use LCA preprocessing for O(1) distances in a full solution.
int tree_dist(int u, int target, int p = -1) {
  if (u == target) {
    return 0;
  }
  for (int v : adj[u]) {
    if (v != p) {
      int d = tree_dist(v, target, u);
      if (d != -1) {
        return d + 1;
      }
    }
  }
  return -1;
}

int main() {
  // 0---1---3---5---6
  //     |   |
  //     2   4
  adj = {{1}, {0, 2, 3}, {1}, {1, 4, 5}, {3}, {3, 6}, {5}};
  vector<int> parent = centroid_decomposition();
  assert((parent == vector<int>{1, 3, 1, -1, 3, 3, 5}));

  const int INF = INT_MAX / 2;
  vector<int> best(7, INF);
  auto mark = [&](int u) {
    for (int c = u; c != -1; c = parent[c]) {
      best[c] = min(best[c], tree_dist(u, c));
    }
  };
  auto nearest_marked = [&](int u) {
    int answer = INF;
    for (int c = u; c != -1; c = parent[c]) {
      answer = min(answer, best[c] + tree_dist(u, c));
    }
    return answer;
  };
  mark(0);
  mark(6);
  assert(nearest_marked(2) == 2);
  assert(nearest_marked(4) == 3);
  assert(nearest_marked(5) == 1);
  return 0;
}
