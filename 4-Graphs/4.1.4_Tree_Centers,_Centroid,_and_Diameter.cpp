/*

An unweighted tree has two useful notions of its middle and one of its extent. Jordan centers
optimize distances, centroids balance component sizes, and a diameter identifies the farthest pair
of nodes; a tree's centers and centroids need not coincide.

Repeatedly removing all leaves reveals the centers. Subtree sizes locate a centroid. A diameter is
found by first finding the farthest node from any start, then traversing again from that endpoint to
find the other.

- `find_centers()` returns the one or two Jordan centers: the nodes minimizing the maximum distance
  to every other node.
- `find_centroid()` returns a centroid: a node whose removal leaves every connected component with
  at most $n / 2$ nodes.
- `diameter()` returns (`length`, `u`, `v`), where `length` is the maximum distance between any two
  nodes and `u` and `v` are endpoints attaining it.

The functions use a global, bidirectionally pre-populated adjacency list `adj`, which must form a
nonempty tree with nodes numbered $[0, n)$, where $n$ is `adj.size()`.

Time Complexity:
- O(n) per call to `find_centers()`, `find_centroid()`, and `diameter()`.

Space Complexity:
- O(n) auxiliary heap space for `find_centers()`.
- O(n) auxiliary stack space for `find_centroid()` and `diameter()`.

*/

#include <algorithm>
#include <tuple>
#include <utility>
#include <vector>

std::vector<std::vector<int>> adj;

std::vector<int> find_centers() {
  int n = static_cast<int>(adj.size());
  std::vector<int> leaves, degree(n);
  for (int i = 0; i < n; i++) {
    degree[i] = static_cast<int>(adj[i].size());
    if (degree[i] <= 1) {
      leaves.push_back(i);
    }
  }
  int removed = static_cast<int>(leaves.size());
  while (removed < n) {
    std::vector<int> nleaves;
    for (int u : leaves) {
      for (int v : adj[u]) {
        if (--degree[v] == 1) {
          nleaves.push_back(v);
        }
      }
    }
    leaves = nleaves;
    removed += static_cast<int>(leaves.size());
  }
  return leaves;
}

// Returns the centroid node index if found in this subtree, or -(subtree size) to propagate the
// size up to the parent so it can check the complementary component's size.
int find_centroid(int u = 0, int p = -1) {
  int n = static_cast<int>(adj.size());
  int count = 1;
  bool good_center = true;
  for (int v : adj[u]) {
    if (v == p) {
      continue;
    }
    int res = find_centroid(v, u);
    if (res >= 0) {
      return res;
    }
    int size = -res;
    good_center &= (size <= n / 2);
    count += size;
  }
  good_center &= (n - count <= n / 2);
  return good_center ? u : -count;
}

std::tuple<int, int, int> diameter() {
  auto dfs = [&](auto &&dfs, int u, int p = -1, int depth = 0) -> std::pair<int, int> {
    std::pair<int, int> res{depth, u};
    for (int v : adj[u]) {
      if (v != p) {
        res = std::max(res, dfs(dfs, v, u, depth + 1));
      }
    }
    return res;
  };
  int u = dfs(dfs, 0).second;
  auto [length, v] = dfs(dfs, u);
  return {length, u, v};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  // 0---1---4---3
  //     |   |
  //     2   5
  adj.assign(6, {});
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(1, 4);
  add_edge(3, 4);
  add_edge(4, 5);
  assert((find_centers() == vector<int>{1, 4}));
  assert(find_centroid() == 4);
  auto [length, u, v] = diameter();
  assert(length == 3 && u == 5 && v == 2);
  return 0;
}
