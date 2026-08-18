/*

Given a directed graph and a start node, find the immediate dominator of every node reachable from
the start. A node `u` dominates node `v` if every path from the start to `v` passes through `u`. The
immediate dominator `idom[v]` is the closest strict dominator of `v`; these parent links form the
dominator tree rooted at the start. Dominator trees are useful in control-flow graphs, program
analysis, and reachability problems with unavoidable checkpoints.

Lengauer-Tarjan's algorithm numbers nodes by DFS order, computes each reachable node's semidominator
using a disjoint-set structure with path compression, and then resolves immediate dominators from
semidominator buckets.

- `immediate_dominators(start)` uses the directed graph in the global adjacency list `adj` and
  returns a vector `idom` where `idom[start]` = `start`, `idom[v]` is the immediate dominator of
  reachable node `v`, and `idom[v]` $= -1$ if `v` is unreachable.

Time Complexity:
- O(max(n, m) log n) per call to `immediate_dominators()` in this path-compressed implementation,
  where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for the graph, result, and auxiliary heap arrays.
- O(n) auxiliary stack space for the DFS.

*/

#include <algorithm>
#include <vector>

std::vector<std::vector<int>> adj;

std::vector<int> immediate_dominators(int start) {
  int n = static_cast<int>(adj.size());
  std::vector<std::vector<int>> pred(n + 1), bucket(n + 1);
  std::vector<int> time(n), node_at_time(n + 1), parent(n + 1), sdom(n + 1);
  std::vector<int> idom_index(n + 1), dsu_root(n + 1), best(n + 1);
  int timer = 0;
  auto dfs = [&](auto &&dfs, int u) -> void {
    time[u] = ++timer;
    node_at_time[timer] = u;
    sdom[timer] = dsu_root[timer] = best[timer] = timer;
    for (int v : adj[u]) {
      if (time[v] == 0) {
        dfs(dfs, v);
        parent[time[v]] = time[u];
      }
      if (time[v] != 0) {
        pred[time[v]].push_back(time[u]);
      }
    }
  };
  dfs(dfs, start);
  auto find_best = [&](auto &&find_best, int u, int depth) -> int {
    if (u == dsu_root[u]) {
      return depth == 0 ? u : -1;
    }
    int root = find_best(find_best, dsu_root[u], depth + 1);
    if (root == -1) {
      return u;
    }
    if (sdom[best[dsu_root[u]]] < sdom[best[u]]) {
      best[u] = best[dsu_root[u]];
    }
    dsu_root[u] = root;
    return depth == 0 ? best[u] : root;  // The outer call returns the label, recursion the root.
  };
  for (int i = timer; i >= 1; i--) {
    for (int v : pred[i]) {
      sdom[i] = std::min(sdom[i], sdom[find_best(find_best, v, 0)]);
    }
    if (i > 1) {
      bucket[sdom[i]].push_back(i);
    }
    for (int v : bucket[i]) {
      int u = find_best(find_best, v, 0);
      idom_index[v] = (sdom[u] == sdom[v]) ? sdom[v] : u;
    }
    if (i > 1) {
      dsu_root[i] = parent[i];
    }
  }
  std::vector<int> idom(n, -1);
  idom[start] = start;
  for (int i = 2; i <= timer; i++) {
    if (idom_index[i] != sdom[i]) {
      idom_index[i] = idom_index[idom_index[i]];
    }
    idom[node_at_time[i]] = node_at_time[idom_index[i]];
  }
  return idom;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // 0 ---> 1 ---+   +-------> 4
  // |           |  /          |
  // v           v /           v
  // 2 --------> 3 ---> 5 ---> 6
  adj = {{1, 2}, {3}, {3}, {4, 5}, {6}, {6}, {}};
  assert((immediate_dominators(0) == vector<int>{0, 0, 0, 0, 3, 3, 3}));

  // Node 3 is reachable through either 1 or 2, so neither one dominates it.
  // 4 ---> 1 -----+
  // |     ^|      |
  // |   /  |      |
  // v /    v      v
  // 0 ---> 2 ---> 3
  adj = {{1, 2}, {2, 3}, {3}, {}, {0, 1}};
  assert((immediate_dominators(4) == vector<int>{4, 4, 4, 4, 4}));
  return 0;
}
