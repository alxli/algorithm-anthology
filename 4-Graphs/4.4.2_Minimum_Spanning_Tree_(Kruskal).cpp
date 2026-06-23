/*

Given a connected, undirected, weighted graph with possibly negative weights, its minimum spanning
tree (MST) is a subgraph which is a tree that connects all nodes with a subset of its edges such
that their total weight is minimized. If the input graph is not connected, then this implementation
will find the minimum spanning forest.

Kruskal's algorithm scans the edges in nondecreasing weight order, adding each edge whose endpoints
lie in different components (tracked with a disjoint-set structure) and skipping any edge that would
form a cycle.

- `kruskal_mst(n)` populates `mst` with the edge IDs in the minimum spanning tree (returning the
  total MST weight) for a global, pre-populated edge list `edges` whose endpoints must be numbered
  [$0$, `n`). Each edge is stored as (`weight`, `u`, `v`).

Multigraphs are supported; parallel edges appear as separate entries in `edges` and any redundant
ones are skipped once the cheaper edge has already connected the two components.

Time Complexity:
- O(m log n) per call, where $m$ is the number of edges and $n$ is the number of nodes. The internal
  DSU uses path compression and union by size, making each DSU operation O(alpha(n)) amortized.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges
- O(n) auxiliary stack space for `kruskal_mst()`.

*/

#include <algorithm>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

std::vector<std::tuple<int, int, int>> edges;  // (weight, u, v)
std::vector<int> dsu_root, dsu_size, mst;

int find_root(int u) {
  if (dsu_root[u] != u) {
    dsu_root[u] = find_root(dsu_root[u]);
  }
  return dsu_root[u];
}

int kruskal_mst(int n) {
  mst.clear();
  std::vector<int> order(edges.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [](int a, int b) {
    return std::get<0>(edges[a]) < std::get<0>(edges[b]);
  });
  int total_dist = 0;
  dsu_root.assign(n, 0);
  dsu_size.assign(n, 1);
  std::iota(dsu_root.begin(), dsu_root.end(), 0);
  for (int id : order) {
    auto &[w, a, b] = edges[id];
    int u = find_root(a), v = find_root(b);
    if (u != v) {
      if (dsu_size[u] < dsu_size[v]) {
        std::swap(u, v);
      }
      dsu_root[v] = u;
      dsu_size[u] += dsu_size[v];
      mst.push_back(id);
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
  cout << "Total distance: " << kruskal_mst(7) << endl;
  for (int id : mst) {
    auto &[w, u, v] = edges[id];
    cout << u << " <-> " << v << endl;
  }
  return 0;
}
