/*

An unweighted tree possesses a center, centroid, and diameter. The centers are found by repeatedly
peeling away the leaves of the tree until one or two nodes remain. The diameter relies on the fact
that the farthest node from any start is always one endpoint of a diameter, so a second traversal
from that endpoint measures the full diameter. The following functions apply to a global,
bidirectionally pre-populated adjacency list `adj` which must form a valid tree with nodes numbered
$[0, `n`)$, where `n` is `adj.size()`.

- `find_centers()` returns a vector of either one or two tree Jordan centers. The Jordan center of a
  tree is the set of all nodes with minimum eccentricity, that is, the set of all nodes where the
  maximum distance to all other nodes in the tree is minimal.
- `find_centroid()` returns the node where all of its subtrees have a size less than or equal to
  $n / 2$, where $n$ is the number of nodes in the tree.
- `diameter()` returns the maximum distance between any two nodes in the tree, using a well-known
  double depth-first search technique.

Time Complexity:
- O(max(n, m)) per call to `find_centers()`, `find_centroid()`, and `diameter()`, where $n$ is the
  number of nodes and $m$ is the number of edges.

Space Complexity:
- O(n) auxiliary stack space for `find_centers()`, `find_centroid()`, and `diameter()`, where $n$ is
  the number of nodes.

*/

#include <algorithm>
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

// Returns the centroid node index if found in this subtree, or -(subtree size) to propagate
// the size up to the parent so it can check the complementary component's size.
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

std::pair<int, int> dfs(int u, int p, int depth) {
  std::pair<int, int> res{depth, u};
  for (int v : adj[u]) {
    if (v != p) {
      res = std::max(res, dfs(v, u, depth + 1));
    }
  }
  return res;
}

int diameter() {
  int furthest_node = dfs(0, -1, 0).second;
  return dfs(furthest_node, -1, 0).first;
}

/*** Example Usage and Output:

Centers: 1 4
Centroid: 4
Diameter length: 3

***/

#include <cassert>
#include <iostream>
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
  vector<int> centers = find_centers();
  assert(centers.size() == 2 && centers[0] == 1 && centers[1] == 4);
  assert(find_centroid() == 4);
  assert(diameter() == 3);
  cout << "Centers:";
  for (int u : centers) {
    cout << " " << u;
  }
  cout << "\nCentroid: " << find_centroid() << "\n";
  cout << "Diameter length: " << diameter() << "\n";
  return 0;
}
