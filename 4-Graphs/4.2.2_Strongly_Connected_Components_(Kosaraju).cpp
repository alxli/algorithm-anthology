/*

Given a directed graph, determine the strongly connected components (SCCs) using the Kosaraju-Sharir
algorithm. A strongly connected component is a maximal set of nodes where every node can reach every
other node. Condensing each SCC into one node produces a directed acyclic graph. The algorithm runs
two passes of depth-first search: the first records the order in which nodes finish, and the second
explores the transposed graph in reverse finish order, with each search collecting exactly one
component.

- `KosarajuSCC(n = 0)` constructs a directed graph of `n` nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds the directed edge from `u` to `v`.
- `build_sccs()` computes the strongly connected components.
- `sccs()` returns the strongly connected components from the last `build_sccs()` call.
- `comp_id(v)` returns the component ID containing node `v`. Component IDs are in topological order:
  for every edge from component $a$ to a different component $b$, $a < b$.

Time Complexity:
- O(max(n, m)) per call to `build_sccs()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, reverse graph, SCCs, and component IDs.
- O(n) auxiliary stack space.

*/

#include <algorithm>
#include <vector>

class KosarajuSCC {
  std::vector<std::vector<int>> adj, rev, scc;
  std::vector<int> comp;
  std::vector<char> visit;

  void dfs_order(int u, std::vector<int> &order) {
    visit[u] = true;
    for (int v : adj[u]) {
      if (!visit[v]) {
        dfs_order(v, order);
      }
    }
    order.push_back(u);
  }

  void dfs_component(int u) {
    visit[u] = true;
    comp[u] = static_cast<int>(scc.size()) - 1;
    scc.back().push_back(u);
    for (int v : rev[u]) {
      if (!visit[v]) {
        dfs_component(v);
      }
    }
  }

 public:
  explicit KosarajuSCC(int n = 0) : adj(n), rev(n) {}

  void add_edge(int u, int v) {
    adj[u].push_back(v);
    rev[v].push_back(u);
  }

  void build_sccs() {
    int n = static_cast<int>(adj.size());
    visit.assign(n, false);
    std::vector<int> order;
    for (int i = 0; i < n; i++) {
      if (!visit[i]) {
        dfs_order(i, order);
      }
    }
    std::reverse(order.begin(), order.end());
    visit.assign(n, false);
    comp.assign(n, -1);
    scc.clear();
    for (int u : order) {
      if (!visit[u]) {
        scc.emplace_back();
        dfs_component(u);
      }
    }
  }

  const std::vector<std::vector<int>> &sccs() const { return scc; }
  int comp_id(int v) const { return comp[v]; }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // 0 ---> 1 ----> 2 <---> 3
  // ^    / |       |       ^
  // |   /  |       |       |
  // |  /   |       |       |
  // | v    v       v       v
  // 4 ---> 5 <---> 6 <---- 7
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
  g.build_sccs();
  // SCC condensation DAG:
  // {0,1,4} -> {2,3,7} -> {5,6}
  //     \-------------------^
  vector<vector<int>> sccs = g.sccs();
  for (auto &scc : sccs) {
    sort(scc.begin(), scc.end());
  }
  sort(sccs.begin(), sccs.end());
  assert((sccs == vector<vector<int>>{{0, 1, 4}, {2, 3, 7}, {5, 6}}));
  assert(g.comp_id(0) == g.comp_id(1) && g.comp_id(1) == g.comp_id(4));
  assert(g.comp_id(2) == g.comp_id(3) && g.comp_id(3) == g.comp_id(7));
  assert(g.comp_id(5) == g.comp_id(6));
  assert(g.comp_id(0) != g.comp_id(2) && g.comp_id(2) != g.comp_id(5));
  return 0;
}
