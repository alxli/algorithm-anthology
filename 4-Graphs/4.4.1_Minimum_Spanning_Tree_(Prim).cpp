/*

Given a connected, undirected, weighted graph with possibly negative weights, its minimum spanning
tree (MST) is a subgraph which is a tree that connects all nodes with a subset of its edges such
that their total weight is minimized. If the input graph is not connected, then this implementation
will find the minimum spanning forest.

Prim's algorithm grows the tree from an arbitrary start node, repeatedly adding the minimum-weight
edge that joins a new node to the current tree, with a priority queue supplying the cheapest such
edge at each step.

- `prim_mst()` populates `mst` with the edge IDs in the minimum spanning tree (returning the total
  MST weight) for a global, bidirectionally pre-populated adjacency list `adj` which must consist of
  nodes numbered [0, `n`), where `n` is `adj.size()`. Edges are stored as (`neighbor`, `weight`,
  `edge_id`).

The priority queue stores candidate edges as (`weight`, `from`, `to`, `edge_id`) and uses
`std::greater` to make it a min-heap. To find a maximum spanning tree instead, use the default
max-heap ordering. Multigraphs are supported; parallel edges are stored as separate adjacency
entries and the algorithm automatically selects the minimum-weight one to each unvisited node.

Time Complexity:
- O(m log n) per call to `prim_mst()`, where $m$ is the number of edges and $n$ is the number of
  nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(m) auxiliary heap space for `prim_mst()`.

*/

#include <functional>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

std::vector<std::vector<std::tuple<int, int, int>>> adj;  // adj[u] = {(v, weight, edge_id), ...}
std::vector<int> mst;

int prim_mst() {
  int n = static_cast<int>(adj.size());
  mst.clear();
  std::vector<bool> visit(n);
  int total_dist = 0;
  for (int i = 0; i < n; i++) {
    if (visit[i]) {
      continue;
    }
    visit[i] = true;
    using qnode = std::tuple<int, int, int, int>;  // (weight, u, v, edge_id)
    std::priority_queue<qnode, std::vector<qnode>, std::greater<qnode>> pq;
    for (auto &[v, w, id] : adj[i]) {
      pq.emplace(w, i, v, id);
    }
    while (!pq.empty()) {
      auto [w, u, v, id] = pq.top();
      pq.pop();
      if (visit[u] && !visit[v]) {
        visit[v] = true;
        if (v != i) {
          mst.push_back(id);
          total_dist += w;
        }
        for (auto &[nb, ew, eid] : adj[v]) {
          pq.emplace(ew, v, nb, eid);
        }
      }
    }
  }
  return total_dist;
}

/*** Example Usage and Output:

Total distance: 13
0 <-> 2
0 <-> 1
3 <-> 4
4 <-> 5
5 <-> 6

***/

#include <iostream>
using namespace std;

vector<pair<int, int>> edge_endpoints;

void add_edge(int u, int v, int w) {
  int id = static_cast<int>(edge_endpoints.size());
  edge_endpoints.emplace_back(u, v);
  adj[u].emplace_back(v, w, id);
  adj[v].emplace_back(u, w, id);
}

int main() {
  int nodes = 7;
  adj.assign(nodes, {});
  add_edge(0, 1, 4);
  add_edge(1, 2, 6);
  add_edge(2, 0, 3);
  add_edge(3, 4, 1);
  add_edge(4, 5, 2);
  add_edge(5, 6, 3);
  add_edge(6, 4, 4);
  cout << "Total distance: " << prim_mst() << endl;
  for (int id : mst) {
    auto &[u, v] = edge_endpoints[id];
    cout << u << " <-> " << v << endl;
  }
  return 0;
}
