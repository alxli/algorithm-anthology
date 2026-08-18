/*

For a connected, undirected, weighted graph with possibly negative weights, a minimum spanning tree
(MST) connects all nodes using a subset of edges with minimum possible total weight. On disconnected
input, this implementation instead finds the minimum spanning forest.

Boruvka's algorithm grows all components at once. In each round, every current component selects the
cheapest edge leaving it, and all selected edges are added to the forest. The cut property makes
each such edge safe, because it is the minimum-weight edge crossing the cut that separates its
component from the rest of the graph. Every component merges with at least one other per round, so
the number of components at least halves and only O(log n) rounds are needed. Ties are broken by
edge ID, giving each component a unique cheapest outgoing edge and making the forest deterministic;
selecting one edge from both endpoints, or closing a cycle among mutually chosen components, is
rejected by the disjoint-set check before the edge joins.

- `boruvka_mst(n)` populates `mst` with the edge IDs in the minimum spanning forest and returns the
  total MST weight for a global, pre-populated edge list `edges` whose endpoints must be numbered
  $[0, `n`)$. Each edge is stored as a tuple (`weight`, `u`, `v`). Multigraphs are supported, with
  redundant parallel edges skipped once the cheaper one has connected the two components.

Unlike Prim's algorithm in section 4.4.1 and Kruskal's in section 4.4.2, no global ordering or
priority queue is needed, since each round only compares edges within a component. That independence
makes this the starting point for parallel MST computation, for the randomized linear-time algorithm
of Karger, Klein, and Tarjan, and for geometric spanning trees where a spatial query replaces the
edge scan, as in the Manhattan MST of section 7.3.11.

Time Complexity:
- O(n + m log n) per call, where $n$ is the number of nodes and $m$ is the number of edges. Each of
  the O(log n) rounds scans every edge once. The internal DSU uses path compression and union by
  size, making each DSU operation O(alpha(n)) amortized.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary.

*/

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

int64_t boruvka_mst(int n) {
  mst.clear();
  dsu_root.assign(n, 0);
  dsu_size.assign(n, 1);
  std::iota(dsu_root.begin(), dsu_root.end(), 0);
  int m = static_cast<int>(edges.size());
  std::vector<int> cheapest(n);
  int64_t total_weight = 0;
  for (int components = n; components > 1;) {
    cheapest.assign(n, -1);
    for (int id = 0; id < m; id++) {
      auto [w, u, v] = edges[id];
      int ru = find(u), rv = find(v);
      if (ru == rv) {
        continue;
      }
      // Edges are scanned by increasing ID, so a strict comparison keeps the lowest ID on ties.
      if (cheapest[ru] == -1 || w < std::get<0>(edges[cheapest[ru]])) {
        cheapest[ru] = id;
      }
      if (cheapest[rv] == -1 || w < std::get<0>(edges[cheapest[rv]])) {
        cheapest[rv] = id;
      }
    }
    int merges = 0;
    for (int r = 0; r < n; r++) {
      if (cheapest[r] == -1) {
        continue;
      }
      auto [w, u, v] = edges[cheapest[r]];
      int ru = find(u), rv = find(v);
      if (ru == rv) {
        continue;  // Already merged earlier in this round, by this edge or a cycle of selections.
      }
      if (dsu_size[ru] < dsu_size[rv]) {
        std::swap(ru, rv);
      }
      dsu_root[rv] = ru;
      dsu_size[ru] += dsu_size[rv];
      mst.push_back(cheapest[r]);
      total_weight += w;  // Overflow warning.
      merges++;
    }
    if (merges == 0) {
      break;  // Disconnected: no edge crosses between the remaining components.
    }
    components -= merges;
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
  assert(boruvka_mst(7) == 13);
  // The same forest that section 4.4.2 finds, discovered by component instead of by weight.
  assert((mst == vector<int>{2, 0, 3, 4, 5}));
  return 0;
}
