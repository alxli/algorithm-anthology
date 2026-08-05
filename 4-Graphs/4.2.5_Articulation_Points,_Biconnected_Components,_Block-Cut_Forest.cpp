/*

Given an undirected graph, compute articulation points, vertex-biconnected components (BCCs), and
the block-cut forest using Tarjan's algorithm. An articulation point (a.k.a. cut vertex) is a node
whose removal increases the number of connected components in the graph. A vertex-biconnected
component is a maximal subgraph that cannot be disconnected by removing one node, with single-edge
and isolated-node components handled as degenerate cases. A single depth-first search tracks each
node's low-link, the earliest entry time reachable from its subtree: a node is an articulation point
exactly when some child's subtree cannot reach above it, and the edges of each BCC are collected on
a stack popped at that moment.

The block-cut forest is a bipartite forest with one node for each BCC and one node for each
articulation point, with an edge whenever an articulation point belongs to a BCC. Note that this
differs from a bridge forest: the block-cut forest describes vertex connectivity, while a bridge
forest describes edge connectivity after compressing 2-edge-connected components.

- `BiconnectedComponents(n = 0)` constructs an undirected graph of `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds the undirected edge `u`-`v`. Parallel edges are supported.
- `build_bcc()` computes the articulation points and vertex-biconnected components.
- `articulation_points()` and `components()` return those results.
- `build_block_cut_forest()` computes the block-cut forest using the results of the previous
  `build_bcc()` call.
- `block_cut_forest()` returns that forest. Its component nodes are numbered in the range
  $[0, `components().size()`)$.
- `block_cut_id(v)` returns the block-cut forest node representing articulation point `v`, or $-1$
  if `v` is not an articulation point.

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

class BiconnectedComponents {
  std::vector<std::vector<int>> adj, bccs, forest;
  std::vector<int> lowlink, tin, forest_id, articulation;
  std::vector<char> visit, is_articulation;
  std::vector<std::pair<int, int>> edges;
  std::vector<int> edge_stack;
  int timer;

  int other(int id, int u) const { return edges[id].first ^ edges[id].second ^ u; }

  void add_component_until(int stop_id) {
    std::vector<int> component;
    while (true) {
      int id = edge_stack.back();
      edge_stack.pop_back();
      component.push_back(edges[id].first);
      component.push_back(edges[id].second);
      if (id == stop_id) {
        break;
      }
    }
    std::sort(component.begin(), component.end());
    component.erase(std::unique(component.begin(), component.end()), component.end());
    bccs.push_back(component);
  }

  void dfs(int u, int p) {
    visit[u] = true;
    lowlink[u] = tin[u] = timer++;
    int children = 0;
    for (int id : adj[u]) {
      if (id == p) {
        continue;
      }
      int v = other(id, u);
      if (visit[v]) {
        if (tin[v] < tin[u]) {
          edge_stack.push_back(id);
          lowlink[u] = std::min(lowlink[u], tin[v]);
        }
      } else {
        edge_stack.push_back(id);
        dfs(v, id);
        lowlink[u] = std::min(lowlink[u], lowlink[v]);
        children++;
        if (lowlink[v] >= tin[u]) {
          if (p != -1 || children > 1) {
            is_articulation[u] = true;
          }
          add_component_until(id);
        }
      }
    }
    if (p == -1 && children == 0) {
      bccs.push_back({u});
    }
  }

 public:
  explicit BiconnectedComponents(int n = 0) : adj(n) {}

  void add_edge(int u, int v) {
    int id = static_cast<int>(edges.size());
    adj[u].push_back(id);
    adj[v].push_back(id);
    edges.emplace_back(u, v);
  }

  void build_bcc() {
    int n = static_cast<int>(adj.size());
    articulation.clear();
    bccs.clear();
    edge_stack.clear();
    lowlink.assign(n, 0);
    tin.assign(n, 0);
    visit.assign(n, false);
    is_articulation.assign(n, false);
    timer = 0;
    for (int i = 0; i < n; i++) {
      if (!visit[i]) {
        dfs(i, -1);
      }
    }
    for (int i = 0; i < n; i++) {
      if (is_articulation[i]) {
        articulation.push_back(i);
      }
    }
  }

  void build_block_cut_forest() {
    int n = static_cast<int>(adj.size());
    int blocks = static_cast<int>(bccs.size());
    forest_id.assign(n, -1);
    int total = blocks;
    for (int v : articulation) {
      forest_id[v] = total++;
    }
    forest.assign(total, {});
    for (int b = 0; b < blocks; b++) {
      for (int v : bccs[b]) {
        if (forest_id[v] != -1) {
          forest[b].push_back(forest_id[v]);
          forest[forest_id[v]].push_back(b);
        }
      }
    }
  }

  const std::vector<int> &articulation_points() const { return articulation; }
  const std::vector<std::vector<int>> &components() const { return bccs; }
  const std::vector<std::vector<int>> &block_cut_forest() const { return forest; }
  int block_cut_id(int v) const { return forest_id[v]; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // 0---1---2    3---7
  //   \ |
  //     5---4
  BiconnectedComponents g(8);
  g.add_edge(0, 1);
  g.add_edge(0, 5);
  g.add_edge(1, 2);
  g.add_edge(1, 5);
  g.add_edge(3, 7);
  g.add_edge(4, 5);
  g.build_bcc();
  assert((g.articulation_points() == vector<int>{1, 5}));
  assert((g.components() == vector<vector<int>>{{1, 2}, {4, 5}, {0, 1, 5}, {3, 7}, {6}}));
  g.build_block_cut_forest();
  assert((g.block_cut_forest() == vector<vector<int>>{{5}, {6}, {5, 6}, {}, {}, {0, 2}, {1, 2}}));
  return 0;
}
