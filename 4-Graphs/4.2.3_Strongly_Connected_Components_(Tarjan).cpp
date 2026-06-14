/*

Given a directed graph, determine the strongly connected components (SCCs) using Tarjan's algorithm.
A strongly connected component is a maximal set of vertices where every vertex can reach every other
vertex. Condensing each SCC into one node produces a directed acyclic graph. A single depth-first
search keeps visited nodes on a stack and tracks each node's low-link, the smallest entry time
reachable from its subtree; a node whose low-link equals its own entry time roots a component, which
is popped off the stack in one piece.

- `TarjanSCC(n)` constructs a directed graph on nodes numbered from 0 to `n - 1`.
- `add_edge(u, v)` adds the directed edge from `u` to `v`.
- `build_scc()` populates `scc` with the strongly connected components and `component[v]` with
  the component ID containing vertex `v`. Component IDs are in reverse topological order: for every
  edge from component `a` to a different component `b`, `a > b`.

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

struct TarjanSCC {
  static const int INF = INT_MAX / 2;
  std::vector<std::vector<int>> adj, scc;
  std::vector<int> component, curstack, lowlink;
  std::vector<bool> visited;
  int timer;

  TarjanSCC(int nodes = 0) : adj(nodes) {}

  void add_edge(int u, int v) { adj[u].push_back(v); }

  void dfs(int u) {
    lowlink[u] = timer++;
    visited[u] = true;
    curstack.push_back(u);
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
      v = curstack.back();
      curstack.pop_back();
      lowlink[v] = INF;  // marks v as removed from the stack
      component[v] = id;
      comp_nodes.push_back(v);
    } while (u != v);
    scc.push_back(comp_nodes);
  }

  void build_scc() {
    int nodes = static_cast<int>(adj.size());
    scc.clear();
    component.assign(nodes, -1);
    curstack.clear();
    lowlink.assign(nodes, 0);
    visited.assign(nodes, false);
    timer = 0;
    for (int i = 0; i < nodes; i++) {
      if (!visited[i]) {
        dfs(i);
      }
    }
  }
};

/*** Example Usage and Output:

Components:
5 6
7 3 2
4 1 0

***/

#include <iostream>
using namespace std;

int main() {
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
  cout << "Components:" << endl;
  for (auto &component : g.scc) {
    for (int v : component) {
      cout << v << " ";
    }
    cout << endl;
  }
  return 0;
}
