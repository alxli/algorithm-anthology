/*

For a connected, undirected, weighted graph with possibly negative weights, a minimum spanning tree
(MST) connects all nodes using a subset of edges with minimum possible total weight. On disconnected
input, this implementation instead finds the minimum spanning forest.

Kruskal's algorithm scans the edges in nondecreasing weight order, adding each edge whose endpoints
lie in different components (tracked with a disjoint-set structure) and skipping any edge that would
form a cycle.

- `kruskal_mst(n)` populates `mst` with the edge IDs in the minimum spanning forest and returns the
  total MST weight for a global, pre-populated edge list `edges` whose endpoints must be numbered
  $[0, `n`)$. Each edge is stored as a tuple (`weight`, `u`, `v`).

Multigraphs are supported; parallel edges appear as separate entries in `edges` and any redundant
ones are skipped once the cheaper edge has already connected the two components.

Time Complexity:
- O(n + m log m) per call, where $n$ is the number of nodes and $m$ is the number of edges. The
  internal DSU uses path compression and union by size, making each DSU operation O(alpha(n))
  amortized.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(max(n, m)) auxiliary heap space and O(log m) auxiliary stack space.

*/

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

std::vector<std::tuple<int, int, int>> edges;  // (weight, u, v)
std::vector<int> dsu_root, dsu_size, mst;

int find(int u) {
  return dsu_root[u] == u ? u : dsu_root[u] = find(dsu_root[u]);
}

int64_t kruskal_mst(int n) {
  mst.clear();
  std::vector<int> order(edges.size());
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [](int a, int b) {
    return std::get<0>(edges[a]) < std::get<0>(edges[b]);
  });
  dsu_root.assign(n, 0);
  dsu_size.assign(n, 1);
  std::iota(dsu_root.begin(), dsu_root.end(), 0);
  int64_t total_weight = 0;
  for (int id : order) {
    auto [w, u, v] = edges[id];
    u = find(u);
    v = find(v);
    if (u != v) {
      if (dsu_size[u] < dsu_size[v]) {
        std::swap(u, v);
      }
      dsu_root[v] = u;
      dsu_size[u] += dsu_size[v];
      mst.push_back(id);
      total_weight += w;  // Overflow warning.
    }
  }
  return total_weight;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Two connected components; MST routine returns a minimum spanning forest.
  //      w=4              w=1
  //   0 ----- 1        3 ----- 4
  //    \     /               / |
  // w=3 \   / w=6      w=2 /   | w=4
  //      \ /             / w=3 |
  //       2            5 ----- 6
  edges.emplace_back(4, 0, 1);  // (weight, u, v)
  edges.emplace_back(6, 1, 2);
  edges.emplace_back(3, 2, 0);
  edges.emplace_back(1, 3, 4);
  edges.emplace_back(2, 4, 5);
  edges.emplace_back(3, 5, 6);
  edges.emplace_back(4, 6, 4);
  assert(kruskal_mst(7) == 13);
  assert((mst == vector<int>{3, 4, 2, 5, 0}));
  return 0;
}
