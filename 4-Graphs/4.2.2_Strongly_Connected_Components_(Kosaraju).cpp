/*

Given a directed graph, determine the strongly connected components (SCCs) using the Kosaraju-Sharir
algorithm. A strongly connected component is a maximal set of vertices where every vertex can reach
every other vertex. Condensing each SCC into one node produces a directed acyclic graph.

- `KosarajuSCC(n)` constructs a directed graph on nodes numbered from 0 to `n - 1`.
- `add_edge(u, v)` adds the directed edge from `u` to `v`.
- `build_scc()` populates `scc` with the strongly connected components.

Time Complexity:
- O(max(n, m)) per call to `build_scc()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, reverse graph, and SCCs.
- O(n) auxiliary stack space.

*/

#include <algorithm>
#include <vector>

struct KosarajuSCC {
  std::vector<std::vector<int>> adj, rev, scc;
  std::vector<bool> visited;

  KosarajuSCC(int nodes = 0) : adj(nodes), rev(nodes) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    rev[v].push_back(u);
  }

  void dfs(const std::vector<std::vector<int>> &g, std::vector<int> &res, int u) {
    visited[u] = true;
    for (int v : g[u]) {
      if (!visited[v]) {
        dfs(g, res, v);
      }
    }
    res.push_back(u);
  }

  void build_scc() {
    int nodes = adj.size();
    visited.assign(nodes, false);
    std::vector<int> order;
    for (int i = 0; i < nodes; i++) {
      if (!visited[i]) {
        dfs(adj, order, i);
      }
    }
    std::reverse(order.begin(), order.end());
    visited.assign(nodes, false);
    scc.clear();
    for (int u : order) {
      if (visited[u]) {
        continue;
      }
      std::vector<int> component;
      dfs(rev, component, u);
      scc.push_back(component);
    }
  }
};

/*** Example Usage and Output:

Components:
1 4 0
7 3 2
5 6

***/

#include <iostream>
using namespace std;

int main() {
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
  cout << "Components:" << endl;
  for (auto &component : g.scc) {
    for (int v : component) {
      cout << v << " ";
    }
    cout << endl;
  }
  return 0;
}
