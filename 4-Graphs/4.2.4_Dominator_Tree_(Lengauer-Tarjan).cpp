/*

Given a directed graph and a start node, find the immediate dominator of every node reachable from
the start. A node `u` dominates node `v` if every path from the start to `v` passes through `u`.
The immediate dominator `idom[v]` is the closest strict dominator of `v`; these parent links form
the dominator tree rooted at the start. Dominator trees are useful in control-flow graphs, program
analysis, and reachability problems with unavoidable checkpoints.

Lengauer-Tarjan's algorithm numbers vertices by DFS order, computes each reachable node's
semidominator using a disjoint-set structure with path compression, and then resolves immediate
dominators from semidominator buckets.

- `Dominators(n)` constructs a directed graph on nodes numbered from 0 to `n - 1`.
- `add_edge(u, v)` adds a directed edge from `u` to `v`.
- `find_dominators(start)` returns a vector `idom` where `idom[start]` = `start`, `idom[v]` is the
  immediate dominator of reachable node `v`, and `idom[v]` $= -1$ if `v` is unreachable.

Time Complexity:
- O(max(n, m) log n) per call to `find_dominators()` in this path-compressed implementation, where
  $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for the graph and auxiliary arrays.
- O(n) auxiliary stack space for the DFS.

*/

#include <algorithm>
#include <vector>

class Dominators {
  std::vector<std::vector<int>> adj, pred, bucket;
  std::vector<int> time, vertex, parent, sdom, idom_index, dsu, best;
  int timer;

  void dfs(int u) {
    time[u] = ++timer;
    vertex[timer] = u;
    sdom[timer] = dsu[timer] = best[timer] = timer;
    for (int v : adj[u]) {
      if (time[v] == 0) {
        dfs(v);
        parent[time[v]] = time[u];
      }
      if (time[v] != 0) {
        pred[time[v]].push_back(time[u]);
      }
    }
  }

  int find_best(int u, bool compress) {
    if (dsu[u] != u) {
      int v = find_best(dsu[u], true);
      if (sdom[v] < sdom[best[u]]) {
        best[u] = v;
      }
      if (compress) {
        dsu[u] = dsu[dsu[u]];
      }
    }
    return best[u];
  }

 public:
  explicit Dominators(int nodes) : adj(nodes), timer(0) {}

  void add_edge(int u, int v) { adj[u].push_back(v); }

  std::vector<int> find_dominators(int start) {
    int nodes = static_cast<int>(adj.size());
    timer = 0;
    time.assign(nodes, 0);
    vertex.assign(nodes + 1, 0);
    parent.assign(nodes + 1, 0);
    sdom.assign(nodes + 1, 0);
    idom_index.assign(nodes + 1, 0);
    dsu.assign(nodes + 1, 0);
    best.assign(nodes + 1, 0);
    pred.assign(nodes + 1, {});
    bucket.assign(nodes + 1, {});
    dfs(start);
    for (int i = timer; i >= 1; i--) {
      for (int v : pred[i]) {
        sdom[i] = std::min(sdom[i], sdom[find_best(v, false)]);
      }
      if (i > 1) {
        bucket[sdom[i]].push_back(i);
      }
      for (int v : bucket[i]) {
        int u = find_best(v, false);
        idom_index[v] = (sdom[u] == sdom[v]) ? sdom[v] : u;
      }
      if (i > 1) {
        dsu[i] = parent[i];
      }
    }
    std::vector<int> idom(adj.size(), -1);
    idom[start] = start;
    for (int i = 2; i <= timer; i++) {
      if (idom_index[i] != sdom[i]) {
        idom_index[i] = idom_index[idom_index[i]];
      }
      idom[vertex[i]] = vertex[idom_index[i]];
    }
    return idom;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  Dominators g(7);
  // 0 branches through 1 and 2, then reconverges at 3 before reaching 4 and 5.
  g.add_edge(0, 1);
  g.add_edge(0, 2);
  g.add_edge(1, 3);
  g.add_edge(2, 3);
  g.add_edge(3, 4);
  g.add_edge(3, 5);
  g.add_edge(4, 6);
  g.add_edge(5, 6);
  vector<int> idom = g.find_dominators(0);
  assert(idom[0] == 0);
  assert(idom[1] == 0);
  assert(idom[2] == 0);
  assert(idom[3] == 0);
  assert(idom[4] == 3);
  assert(idom[5] == 3);
  assert(idom[6] == 3);
  assert(g.find_dominators(0) == idom);
  return 0;
}
