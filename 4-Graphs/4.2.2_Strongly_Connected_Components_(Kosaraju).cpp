/*

Given a directed graph, determine the strongly connected components (SCCs) using the Kosaraju-Sharir
algorithm. A strongly connected component is a maximal set of nodes where every node can reach every
other node. Condensing each SCC into one node produces a directed acyclic graph. The algorithm runs
two passes of depth-first search: the first records the order in which nodes finish, and the second
explores the transposed graph in reverse finish order, with each search collecting exactly one
component.

- `KosarajuSCC(n = 0)` constructs a directed graph of `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds the directed edge from `u` to `v`.
- `build_scc()` computes the strongly connected components.
- `components()` returns the strongly connected components from the last `build_scc()` call.
- `component_id(v)` returns the component ID containing node `v`. Component IDs are in topological
  order: for every edge from component $a$ to a different component $b$, $a < b$.

Time Complexity:
- O(max(n, m)) per call to `build_scc()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, reverse graph, SCCs, and component IDs.
- O(n) auxiliary stack space.

*/

#include <algorithm>
#include <vector>

class KosarajuSCC {
  std::vector<std::vector<int>> adj, rev, scc;
  std::vector<int> component;
  std::vector<char> visited;

  void dfs(const std::vector<std::vector<int>> &g, std::vector<int> &res, int u) {
    visited[u] = true;
    for (int v : g[u]) {
      if (!visited[v]) {
        dfs(g, res, v);
      }
    }
    res.push_back(u);
  }

  void dfs_component(int u, int id, std::vector<int> &res) {
    visited[u] = true;
    component[u] = id;
    res.push_back(u);
    for (int v : rev[u]) {
      if (!visited[v]) {
        dfs_component(v, id, res);
      }
    }
  }

 public:
  explicit KosarajuSCC(int n = 0) : adj(n), rev(n) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    rev[v].push_back(u);
  }

  void build_scc() {
    int n = static_cast<int>(adj.size());
    visited.assign(n, false);
    std::vector<int> order;
    for (int i = 0; i < n; i++) {
      if (!visited[i]) {
        dfs(adj, order, i);
      }
    }
    std::reverse(order.begin(), order.end());
    visited.assign(n, false);
    component.assign(n, -1);
    scc.clear();
    for (int u : order) {
      if (visited[u]) {
        continue;
      }
      std::vector<int> comp_nodes;
      dfs_component(u, static_cast<int>(scc.size()), comp_nodes);
      scc.push_back(comp_nodes);
    }
  }

  const std::vector<std::vector<int>> &components() const { return scc; }
  int component_id(int v) const { return component[v]; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // 0 ---> 1 ----> 2 <---> 3
  // ^    / |       |       ^
  // |   /  |       |       |
  // |  /   |       |       v
  // | v    v       v       7
  // 4 ---> 5 <---> 6 <----/
  KosarajuSCC g(8);
  g.add_edge(0, 1);
  g.add_edge(1, 2);
  g.add_edge(1, 4);
  g.add_edge(1, 5);
  g.add_edge(2, 3);
  g.add_edge(2, 6);
  g.add_edge(3, 2);
  g.add_edge(3, 7);
  g.add_edge(4, 0);
  g.add_edge(4, 5);
  g.add_edge(5, 6);
  g.add_edge(6, 5);
  g.add_edge(7, 3);
  g.add_edge(7, 6);
  g.build_scc();
  // SCC condensation DAG:
  // {0,1,4} -> {2,3,7} -> {5,6}
  //     \-------------------^
  assert(g.components().size() == 3);
  vector<vector<int>> components = g.components();
  for (auto &component : components) {
    sort(component.begin(), component.end());
  }
  sort(components.begin(), components.end());
  assert((components == vector<vector<int>>{{0, 1, 4}, {2, 3, 7}, {5, 6}}));
  assert(g.component_id(0) == g.component_id(1) && g.component_id(1) == g.component_id(4));
  assert(g.component_id(2) == g.component_id(3) && g.component_id(3) == g.component_id(7));
  assert(g.component_id(5) == g.component_id(6));
  assert(g.component_id(0) != g.component_id(2) && g.component_id(2) != g.component_id(5));
  return 0;
}
