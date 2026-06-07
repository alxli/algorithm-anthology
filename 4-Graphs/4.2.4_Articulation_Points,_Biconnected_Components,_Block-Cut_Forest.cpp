/*

Given an undirected graph, compute articulation points, vertex-biconnected components (BCCs), and
the block-cut forest using Tarjan's algorithm. An articulation point (a.k.a. cut vertex) is a vertex
whose removal increases the number of connected components in the graph. A vertex-biconnected
component is a maximal subgraph that cannot be disconnected by removing one vertex, with single-edge
and isolated-vertex components handled as degenerate cases.

The block-cut forest is a bipartite forest with one node for each BCC and one node for each
articulation point, with an edge whenever an articulation point belongs to a BCC. Note that this
differs from a bridge forest: the block-cut forest describes vertex connectivity, while a bridge
forest describes edge connectivity after compressing 2-edge-connected components.

- `BiconnectedComponents(n)` constructs an undirected graph on nodes numbered from 0 to `n - 1`.
- `add_edge(u, v)` adds the undirected edge `u`-`v`.
- `build_bcc()` populates `articulation_points` and `biconnected_components`.
- `build_block_cut_forest()` populates `block_cut_forest` and `block_cut_id` using the results of
  the previous `build_bcc()` call.
- BCC nodes in `block_cut_forest` are numbered `[0, biconnected_components.size())`.
- For an articulation point `v`, `block_cut_id[v]` stores its node ID in the block-cut forest.
- For a non-articulation point `v`, `block_cut_id[v] == -1`.

Limitation: the DFS skips the parent by node ID (`v == p`), so parallel edges (multigraphs) are not
supported. To support multigraphs, skip the specific edge used to reach a node by its edge ID
instead.

Time Complexity:
- O(max(n, m)) per call to `build_bcc()` and `build_block_cut_forest()`, where $n$ is the number of
  nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, edge stack, BCCs, and block-cut forest.
- O(n) auxiliary stack space for `build_bcc()`.

*/

#include <algorithm>
#include <utility>
#include <vector>

struct BiconnectedComponents {
  std::vector<std::vector<int>> adj, biconnected_components, block_cut_forest;
  std::vector<int> lowlink, tin, block_cut_id, articulation_points;
  std::vector<bool> visited, is_articulation;
  std::vector<std::pair<int, int>> edge_stack;
  int timer;

  BiconnectedComponents(int nodes = 0) : adj(nodes) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  }

  void add_component_until(int u, int v) {
    std::vector<int> component;
    for (;;) {
      auto e = edge_stack.back();
      edge_stack.pop_back();
      component.push_back(e.first);
      component.push_back(e.second);
      if (e.first == u && e.second == v) {
        break;
      }
    }
    std::sort(component.begin(), component.end());
    component.erase(std::unique(component.begin(), component.end()), component.end());
    biconnected_components.push_back(component);
  }

  void dfs(int u, int p) {
    visited[u] = true;
    lowlink[u] = tin[u] = timer++;
    int children = 0;
    for (int v : adj[u]) {
      if (v == p) {
        continue;
      }
      if (visited[v]) {
        if (tin[v] < tin[u]) {
          edge_stack.emplace_back(u, v);
          lowlink[u] = std::min(lowlink[u], tin[v]);
        }
      } else {
        edge_stack.emplace_back(u, v);
        dfs(v, u);
        lowlink[u] = std::min(lowlink[u], lowlink[v]);
        children++;
        if (lowlink[v] >= tin[u]) {
          if (p != -1 || children > 1) {
            is_articulation[u] = true;
          }
          add_component_until(u, v);
        }
      }
    }
    if (p == -1 && children == 0) {
      biconnected_components.push_back({u});
    }
  }

  void build_bcc() {
    int nodes = adj.size();
    articulation_points.clear();
    biconnected_components.clear();
    edge_stack.clear();
    lowlink.assign(nodes, 0);
    tin.assign(nodes, 0);
    visited.assign(nodes, false);
    is_articulation.assign(nodes, false);
    timer = 0;
    for (int i = 0; i < nodes; i++) {
      if (!visited[i]) {
        dfs(i, -1);
      }
    }
    for (int i = 0; i < nodes; i++) {
      if (is_articulation[i]) {
        articulation_points.push_back(i);
      }
    }
  }

  void build_block_cut_forest() {
    int nodes = adj.size();
    int blocks = biconnected_components.size();
    block_cut_id.assign(nodes, -1);
    int total = blocks;
    for (int v : articulation_points) {
      block_cut_id[v] = total++;
    }
    block_cut_forest.assign(total, std::vector<int>());
    for (int b = 0; b < blocks; b++) {
      for (int v : biconnected_components[b]) {
        if (block_cut_id[v] != -1) {
          block_cut_forest[b].push_back(block_cut_id[v]);
          block_cut_forest[block_cut_id[v]].push_back(b);
        }
      }
    }
  }
};

/*** Example Usage and Output:

Articulation points: 1 5
Biconnected components:
1 2 
4 5 
0 1 5 
3 7 
6 
Adjacency List for Block-Cut Forest:
0 => 5
1 => 6
2 => 5 6
3 =>
4 =>
5 => 0 2
6 => 1 2

***/

#include <iostream>
using namespace std;

int main() {
  BiconnectedComponents g(8);
  g.add_edge(0, 1);
  g.add_edge(0, 5);
  g.add_edge(1, 2);
  g.add_edge(1, 5);
  g.add_edge(3, 7);
  g.add_edge(4, 5);
  g.build_bcc();
  g.build_block_cut_forest();
  cout << "Articulation points:";
  for (int v : g.articulation_points) {
    cout << " " << v;
  }
  cout << endl << "Biconnected components:" << endl;
  for (auto &component : g.biconnected_components) {
    for (int v : component) {
      cout << v << " ";
    }
    cout << endl;
  }
  cout << "Adjacency List for Block-Cut Forest:" << endl;
  for (int i = 0, n = static_cast<int>(g.block_cut_forest.size()); i < n; i++) {
    cout << i << " =>";
    for (int v : g.block_cut_forest[i]) {
      cout << " " << v;
    }
    cout << endl;
  }
  return 0;
}
