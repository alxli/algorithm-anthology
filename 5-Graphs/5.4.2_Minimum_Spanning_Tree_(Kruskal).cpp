/*

Given a connected, undirected, weighted graph with possibly negative weights, its minimum spanning
tree is a subgraph which is a tree that connects all nodes with a subset of its edges such that
their total weight is minimized. `kruskal()` applies to a global, pre-populated edge list `edges`.
If the input graph is not connected, then this implementation will find the minimum spanning forest.

Time Complexity:
- O(m log n) per call to `kruskal()`, where $m$ is the number of edges and $n$ is the number of
  nodes. The internal DSU uses path compression but not union-by-rank (simplified for brevity);
  `find_root()` is O(log n) amortized rather than O(alpha(n)).

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges
- O(n) auxiliary stack space for `kruskal()`.

*/

#include <algorithm>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

std::vector<std::tuple<int, int, int>> edges;
std::vector<int> root;
std::vector<std::pair<int, int>> mst;

int find_root(int u) {
  if (root[u] != u) {
    root[u] = find_root(root[u]);
  }
  return root[u];
}

int kruskal(int nodes) {
  mst.clear();
  std::sort(edges.begin(), edges.end());
  int total_dist = 0;
  root.resize(nodes);
  std::iota(root.begin(), root.end(), 0);
  for (auto &[w, a, b] : edges) {
    int u = find_root(a);
    int v = find_root(b);
    if (u != v) {
      root[u] = root[v];
      mst.emplace_back(a, b);
      total_dist += w;
    }
  }
  return total_dist;
}

/*** Example Usage and Output:

Total distance: 13
3 <-> 4
4 <-> 5
2 <-> 0
5 <-> 6
0 <-> 1

***/

#include <iostream>
using namespace std;

void add_edge(int u, int v, int w) {
  edges.emplace_back(w, u, v);
}

int main() {
  add_edge(0, 1, 4);
  add_edge(1, 2, 6);
  add_edge(2, 0, 3);
  add_edge(3, 4, 1);
  add_edge(4, 5, 2);
  add_edge(5, 6, 3);
  add_edge(6, 4, 4);
  cout << "Total distance: " << kruskal(7) << endl;
  for (auto &[u, v] : mst) {
    cout << u << " <-> " << v << endl;
  }
  return 0;
}
