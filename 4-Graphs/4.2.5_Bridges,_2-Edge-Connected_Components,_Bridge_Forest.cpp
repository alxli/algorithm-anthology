/*

Given an undirected graph, compute bridges, 2-edge-connected components, and the bridge forest using
Tarjan's algorithm. A bridge is an edge whose removal increases the number of connected components
in the graph. A 2-edge-connected component is a maximal set of vertices connected without crossing
any bridge.

After each 2-edge-connected component is condensed into one node, the original bridges connect those
nodes into a bridge tree, or a bridge forest when the original graph is disconnected. This differs
from a block-cut forest: the bridge forest describes edge connectivity, while the block-cut forest
describes vertex connectivity using articulation points and vertex-biconnected components.

- `BridgeDecomposition(n)` constructs an undirected graph on nodes numbered from 0 to `n - 1`.
- `add_edge(u, v)` adds the undirected edge `u`-`v`.
- `build_bridges()` populates `bridges`.
- `build_bridge_forest()` populates `component`, `two_edge_components`, and `bridge_forest` using
  the results of the previous `build_bridges()` call.
- `component[u]` stores the 2-edge-connected component ID containing vertex `u`.

Limitation: the DFS skips the parent by node ID (`v == p`), so parallel edges (multigraphs) are not
supported -- a doubled edge `u`-`v` would be misreported as a bridge even though it lies on a cycle.
To support multigraphs, skip the specific edge used to reach a node by its edge ID instead.

Time Complexity:
- O((n + m) log m) per call to `build_bridges()` followed by `build_bridge_forest()`, where $n$ is
  the number of nodes and $m$ is the number of edges. The logarithmic factor comes from bridge
  lookups.

Space Complexity:
- O(max(n, m)) for storage of the graph, bridges, 2-edge-connected components, and bridge forest.
- O(n) auxiliary stack space for the DFS calls.

*/

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

struct BridgeDecomposition {
  std::vector<std::vector<int>> adj, two_edge_components, bridge_forest;
  std::vector<int> lowlink, tin, component;
  std::vector<bool> visited;
  std::vector<std::pair<int, int>> bridges;
  std::set<std::pair<int, int>> bridge_edges;
  int timer;

  BridgeDecomposition(int nodes = 0) : adj(nodes) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  std::pair<int, int> edge_key(int u, int v) { return std::minmax(u, v); }

  bool is_bridge(int u, int v) { return bridge_edges.count(edge_key(u, v)) > 0; }

  void dfs_bridges(int u, int p) {
    visited[u] = true;
    lowlink[u] = tin[u] = timer++;
    for (int v : adj[u]) {
      if (v == p) {
        continue;
      }
      if (visited[v]) {
        lowlink[u] = std::min(lowlink[u], tin[v]);
      } else {
        dfs_bridges(v, u);
        lowlink[u] = std::min(lowlink[u], lowlink[v]);
        if (lowlink[v] > tin[u]) {
          bridges.emplace_back(u, v);
          bridge_edges.insert(edge_key(u, v));
        }
      }
    }
  }

  void build_bridges() {
    int nodes = adj.size();
    bridges.clear();
    bridge_edges.clear();
    lowlink.assign(nodes, 0);
    tin.assign(nodes, 0);
    visited.assign(nodes, false);
    timer = 0;
    for (int i = 0; i < nodes; i++) {
      if (!visited[i]) {
        dfs_bridges(i, -1);
      }
    }
  }

  void dfs_component(int u, int id) {
    component[u] = id;
    two_edge_components[id].push_back(u);
    for (int v : adj[u]) {
      if (component[v] == -1 && !is_bridge(u, v)) {
        dfs_component(v, id);
      }
    }
  }

  void build_bridge_forest() {
    int nodes = adj.size();
    component.assign(nodes, -1);
    two_edge_components.clear();
    for (int i = 0; i < nodes; i++) {
      if (component[i] == -1) {
        two_edge_components.push_back(std::vector<int>());
        dfs_component(i, static_cast<int>(two_edge_components.size()) - 1);
      }
    }
    bridge_forest.assign(two_edge_components.size(), std::vector<int>());
    for (auto &[u, v] : bridges) {
      int cu = component[u], cv = component[v];
      bridge_forest[cu].push_back(cv);
      bridge_forest[cv].push_back(cu);
    }
  }
};

/*** Example Usage and Output:

Bridges:
1 2
5 4
3 7
2-edge-connected components:
0 1 5 
2 
3 
4 
6 
7 
Adjacency List for Bridge Forest:
0 => 1 3
1 => 0
2 => 5
3 => 0
4 =>
5 => 2

***/

#include <iostream>
using namespace std;

int main() {
  BridgeDecomposition g(8);
  g.add_edge(0, 1);
  g.add_edge(0, 5);
  g.add_edge(1, 2);
  g.add_edge(1, 5);
  g.add_edge(3, 7);
  g.add_edge(4, 5);
  g.build_bridges();
  g.build_bridge_forest();
  cout << "Bridges:" << endl;
  for (auto &[u, v] : g.bridges) {
    cout << u << " " << v << endl;
  }
  cout << "2-edge-connected components:" << endl;
  for (auto &component : g.two_edge_components) {
    for (int v : component) {
      cout << v << " ";
    }
    cout << endl;
  }
  cout << "Adjacency List for Bridge Forest:" << endl;
  for (int i = 0, n = static_cast<int>(g.bridge_forest.size()); i < n; i++) {
    cout << i << " =>";
    for (int v : g.bridge_forest[i]) {
      cout << " " << v;
    }
    cout << endl;
  }
  return 0;
}
