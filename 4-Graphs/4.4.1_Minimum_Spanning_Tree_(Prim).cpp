/*

Given a connected, undirected, weighted graph with possibly negative weights, its minimum spanning
tree (MST) is a subgraph which is a tree that connects all nodes with a subset of its edges such
that their total weight is minimized. If the input graph is not connected, then this implementation
will find the minimum spanning forest.

Prim's algorithm grows the tree from an arbitrary start node, repeatedly adding the minimum-weight
edge that joins a new node to the current tree, with a priority queue supplying the cheapest such
edge at each step.

- `prim_mst()` populates `mst` with the edges in the minimum spanning forest and returns the total
  MST weight for a global, bidirectionally pre-populated adjacency list `adj` which must consist of
  nodes numbered $[0, `n`)$, where `n` is `adj.size()`. Adjacency entries are stored as (`neighbor`,
  `weight`), while each MST edge is stored as (`from`, `to`, `weight`).

The priority queue stores candidate edges as (`weight`, `from`, `to`) and uses `std::greater` to
make it a min-heap. To find a maximum spanning tree instead, use the default max-heap ordering.
Multigraphs are supported; parallel edges are stored as separate adjacency entries and the algorithm
automatically selects the minimum-weight one to each unvisited node.

Time Complexity:
- O(n + m log m) per call, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(max(n, m)) auxiliary.

*/

#include <cstdint>
#include <functional>
#include <queue>
#include <tuple>
#include <utility>
#include <vector>

std::vector<std::vector<std::pair<int, int>>> adj;  // adj[u] = {(v, weight), ...}
std::vector<std::tuple<int, int, int>> mst;         // (u, v, weight)

int64_t prim_mst() {
  int n = static_cast<int>(adj.size());
  mst.clear();
  std::vector<char> visit(n);
  int64_t total_dist = 0;
  for (int i = 0; i < n; i++) {
    if (visit[i]) {
      continue;
    }
    visit[i] = true;
    using QueueEntry = std::tuple<int, int, int>;  // (weight, u, v)
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<>> pq;
    for (auto [v, w] : adj[i]) {
      pq.emplace(w, i, v);
    }
    while (!pq.empty()) {
      auto [w, u, v] = pq.top();
      pq.pop();
      if (visit[v]) {
        continue;
      }
      visit[v] = true;
      mst.emplace_back(u, v, w);
      total_dist += w;
      for (auto [to, ew] : adj[v]) {
        pq.emplace(ew, v, to);
      }
    }
  }
  return total_dist;
}

/*** Example Usage and Output:

Total distance: 13
0 <-> 2 (3)
0 <-> 1 (4)
3 <-> 4 (1)
4 <-> 5 (2)
5 <-> 6 (3)

***/

#include <cassert>
#include <iostream>
using namespace std;

void add_edge(int u, int v, int w) {
  adj[u].emplace_back(v, w);
  adj[v].emplace_back(u, w);
}

int main() {
  // Two connected components; MST routine returns a minimum spanning forest.
  //      w=4              w=1
  //   0 ----- 1        3 ----- 4
  //    \     /               / |
  // w=3 \   / w=6      w=2 /   | w=4
  //      \ /             / w=3 |
  //       2            5 ----- 6
  adj.assign(7, {});
  add_edge(0, 1, 4);
  add_edge(1, 2, 6);
  add_edge(2, 0, 3);
  add_edge(3, 4, 1);
  add_edge(4, 5, 2);
  add_edge(5, 6, 3);
  add_edge(6, 4, 4);
  int64_t total = prim_mst();
  assert(total == 13);
  assert(mst.size() == 5);
  cout << "Total distance: " << total << endl;
  for (auto [u, v, w] : mst) {
    cout << u << " <-> " << v << " (" << w << ")" << endl;
  }
  return 0;
}
