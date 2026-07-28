/*

Given an undirected graph, compute bridges, 2-edge-connected components, and the bridge forest using
Tarjan's algorithm. A bridge is an edge whose removal increases the number of connected components
in the graph. A 2-edge-connected component is a maximal set of nodes connected without crossing a
bridge. A single depth-first search tracks each node's low-link, the earliest entry time reachable
from its subtree: a tree edge is a bridge exactly when the child's subtree cannot reach back to the
parent's side by any other route.

After each 2-edge-connected component is condensed into one node, the original bridges connect those
nodes into a bridge tree, or a bridge forest when the original graph is disconnected. This differs
from a block-cut forest: the bridge forest describes edge connectivity, while the block-cut forest
describes vertex connectivity using articulation points and vertex-biconnected components.

- `BridgeDecomposition(n = 0)` constructs an undirected graph of `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds the undirected edge `u`-`v`. Parallel edges are supported.
- `build_bridges()` computes the bridges, returned by `bridges()`.
- `build_bridge_forest()` computes the 2-edge-connected components and bridge forest using the
  results of the previous `build_bridges()` call.
- `components()` and `bridge_forest()` return those results.
- `component_id(u)` returns the 2-edge-connected component ID containing node `u`.

Time Complexity:
- O(max(n, m)) per call to `build_bridges()` followed by `build_bridge_forest()`, where $n$ is the
  number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, bridges, 2-edge-connected components, and bridge forest.
- O(n) auxiliary stack space for the DFS calls.

*/

#include <algorithm>
#include <utility>
#include <vector>

class BridgeDecomposition {
  std::vector<std::vector<int>> adj, two_edge_components, forest;
  std::vector<int> lowlink, tin, component;
  std::vector<char> visited, is_bridge_edge;
  std::vector<std::pair<int, int>> edges, bridge_edges;
  int timer;

  int other(int id, int u) const { return edges[id].first ^ edges[id].second ^ u; }

  void dfs_bridges(int u, int p) {
    visited[u] = true;
    lowlink[u] = tin[u] = timer++;
    for (int id : adj[u]) {
      if (id == p) {
        continue;
      }
      int v = other(id, u);
      if (visited[v]) {
        lowlink[u] = std::min(lowlink[u], tin[v]);
      } else {
        dfs_bridges(v, id);
        lowlink[u] = std::min(lowlink[u], lowlink[v]);
        if (lowlink[v] > tin[u]) {
          bridge_edges.emplace_back(u, v);
          is_bridge_edge[id] = true;
        }
      }
    }
  }

  void dfs_component(int u, int id) {
    component[u] = id;
    two_edge_components[id].push_back(u);
    for (int edge_id : adj[u]) {
      int v = other(edge_id, u);
      if (component[v] == -1 && !is_bridge_edge[edge_id]) {
        dfs_component(v, id);
      }
    }
  }

 public:
  explicit BridgeDecomposition(int n = 0) : adj(n) {}

  void add_edge(int u, int v) {
    int id = static_cast<int>(edges.size());
    adj[u].push_back(id);
    adj[v].push_back(id);
    edges.emplace_back(u, v);
  }

  void build_bridges() {
    int n = static_cast<int>(adj.size());
    bridge_edges.clear();
    lowlink.assign(n, 0);
    tin.assign(n, 0);
    visited.assign(n, false);
    is_bridge_edge.assign(edges.size(), false);
    timer = 0;
    for (int i = 0; i < n; i++) {
      if (!visited[i]) {
        dfs_bridges(i, -1);
      }
    }
  }

  void build_bridge_forest() {
    int n = static_cast<int>(adj.size());
    component.assign(n, -1);
    two_edge_components.clear();
    for (int i = 0; i < n; i++) {
      if (component[i] == -1) {
        two_edge_components.push_back({});
        dfs_component(i, static_cast<int>(two_edge_components.size()) - 1);
      }
    }
    forest.assign(two_edge_components.size(), {});
    for (auto &[u, v] : bridge_edges) {
      int cu = component[u], cv = component[v];
      forest[cu].push_back(cv);
      forest[cv].push_back(cu);
    }
  }

  const std::vector<std::pair<int, int>> &bridges() const { return bridge_edges; }
  const std::vector<std::vector<int>> &components() const { return two_edge_components; }
  const std::vector<std::vector<int>> &bridge_forest() const { return forest; }
  int component_id(int u) const { return component[u]; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // 0---1---2    3---7
  //   \ |
  //     5---4
  BridgeDecomposition g(8);
  g.add_edge(0, 1);
  g.add_edge(0, 5);
  g.add_edge(1, 2);
  g.add_edge(1, 5);
  g.add_edge(3, 7);
  g.add_edge(4, 5);
  g.build_bridges();
  assert((g.bridges() == vector<pair<int, int>>{{1, 2}, {5, 4}, {3, 7}}));
  g.build_bridge_forest();
  vector<vector<int>> components = g.components();
  for (auto &component : components) {
    sort(component.begin(), component.end());
  }
  sort(components.begin(), components.end());
  assert((components == vector<vector<int>>{{0, 1, 5}, {2}, {3}, {4}, {6}, {7}}));
  assert((g.bridge_forest() == vector<vector<int>>{{1, 3}, {0}, {5}, {0}, {}, {2}}));
  return 0;
}
