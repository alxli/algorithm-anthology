/*

Given a directed graph, determine the strongly connected components (SCCs) using Tarjan's algorithm.
A strongly connected component is a maximal set of nodes where every node can reach every other
node. Condensing each SCC into one node produces a directed acyclic graph. A single depth-first
search keeps visited nodes on a stack and tracks each node's low-link, the smallest entry time
reachable from its subtree; a node whose low-link equals its own entry time roots a component, which
is popped off the stack in one piece.

- `TarjanSCC(n = 0)` constructs a directed graph of `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds the directed edge from `u` to `v`.
- `build_scc()` computes the strongly connected components.
- `components()` returns the strongly connected components from the last `build_scc()` call.
- `component_id(v)` returns the component ID containing node `v`. Component IDs are in reverse
  topological order: for every edge from component $a$ to a different component $b$, $a > b$.

Time Complexity:
- O(max(n, m)) per call to `build_scc()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, SCCs, and component IDs.
- O(n) auxiliary stack space.

*/

#include <algorithm>
#include <climits>
#include <vector>

class TarjanSCC {
  static const int INF = INT_MAX / 2;
  std::vector<std::vector<int>> adj, scc;
  std::vector<int> component, active, lowlink;
  std::vector<char> visited;
  int timer;

  void dfs(int u) {
    lowlink[u] = timer++;
    visited[u] = true;
    active.push_back(u);
    bool is_component_root = true;
    for (int v : adj[u]) {
      if (!visited[v]) {
        dfs(v);
      }
      if (lowlink[u] > lowlink[v]) {
        lowlink[u] = lowlink[v];
        is_component_root = false;
      }
    }
    if (!is_component_root) {
      return;
    }
    std::vector<int> comp_nodes;
    int id = static_cast<int>(scc.size());
    int v;
    do {
      v = active.back();
      active.pop_back();
      lowlink[v] = INF;  // marks v as removed from the active stack
      component[v] = id;
      comp_nodes.push_back(v);
    } while (u != v);
    scc.push_back(comp_nodes);
  }

 public:
  explicit TarjanSCC(int n = 0) : adj(n) {}

  void add_edge(int u, int v) { adj[u].push_back(v); }

  void build_scc() {
    int n = static_cast<int>(adj.size());
    scc.clear();
    component.assign(n, -1);
    active.clear();
    lowlink.assign(n, 0);
    visited.assign(n, false);
    timer = 0;
    for (int i = 0; i < n; i++) {
      if (!visited[i]) {
        dfs(i);
      }
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
  TarjanSCC g(8);
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
