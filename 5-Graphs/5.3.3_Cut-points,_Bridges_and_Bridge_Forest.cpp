/*

Given an undirected graph, compute the following properties of the graph using Tarjan's algorithm.
`tarjan()` applies to a global, pre-populated adjacency list `adj` which satisfies the precondition
that for every node $v$ in `adj[u]`, node $u$ also exists in `adj[v]`. Nodes in `adj`
must be numbered with integers between 0 (inclusive) and `adj.size()` (exclusive).
`get_block_forest()` applies to a global vector `block` which must be already precomputed by a call
to `tarjan()`.

A cut-point (i.e. cut-node, or articulation point) is any node whose removal increases the number of
connected components in the graph.

A bridge is an edge such that when deleted, the number of connected components in the graph is
increased. An edge is a bridge if and only if it is not part of any cycle.

Limitation: the DFS skips the parent by node id (`v == p`), so parallel edges (multigraphs) are not
supported -- a doubled edge `u`-`v` would be misreported as a bridge even though it lies on a cycle.
To support multigraphs, skip the specific edge used to reach a node by its edge id instead.

Condensing each maximal component without a bridge into a single node, we can decompose any
connected graph into a bridge-block tree (a.k.a. bridge-tree or block-tree), with bridges connecting
each block. An unconnected graph will thus decompose into a "bridge-block forest."

Time Complexity:
- O(max(n, m)) per call to `tarjan()` and `get_block_forest()`, where $n$ is the number of nodes and
  $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges
- O(n) auxiliary stack space for `tarjan()`.
- O(1) auxiliary stack space for `get_block_forest()`.

*/

#include <algorithm>
#include <utility>
#include <vector>

int timer;
std::vector<int> lowlink, tin, comp;
std::vector<bool> visit;
std::vector<std::vector<int>> adj, block_forest;
std::vector<int> currstack, cutpoints;
std::vector<std::vector<int>> block;
std::vector<std::pair<int, int>> bridges;

void dfs(int u, int p) {
  visit[u] = true;
  lowlink[u] = tin[u] = timer++;
  currstack.push_back(u);
  int children = 0;
  bool cutpoint = false;
  for (int v : adj[u]) {
    if (v == p) {
      continue;
    }
    if (visit[v]) {
      lowlink[u] = std::min(lowlink[u], tin[v]);
    } else {
      dfs(v, u);
      lowlink[u] = std::min(lowlink[u], lowlink[v]);
      cutpoint |= (lowlink[v] >= tin[u]);
      if (lowlink[v] > tin[u]) {
        bridges.emplace_back(u, v);
      }
      children++;
    }
  }
  if (p == -1) {
    cutpoint = (children >= 2);
  }
  if (cutpoint) {
    cutpoints.push_back(u);
  }
  if (lowlink[u] == tin[u]) {
    std::vector<int> component;
    int v;
    do {
      v = currstack.back();
      currstack.pop_back();
      component.push_back(v);
    } while (u != v);
    block.push_back(component);
  }
}

void tarjan() {
  int nodes = adj.size();
  block.clear();
  bridges.clear();
  cutpoints.clear();
  currstack.clear();
  lowlink.assign(nodes, 0);
  tin.assign(nodes, 0);
  visit.assign(nodes, false);
  timer = 0;
  for (int i = 0; i < nodes; i++) {
    if (!visit[i]) {
      dfs(i, -1);
    }
  }
}

void get_block_forest() {
  int nodes = adj.size();
  comp.assign(nodes, 0);
  block_forest.assign(nodes, std::vector<int>());
  int id = 0;
  for (const auto &component : block) {
    for (int v : component) {
      comp[v] = id;
    }
    id++;
  }
  for (int i = 0; i < nodes; i++) {
    for (int v : adj[i]) {
      if (comp[i] != comp[v]) {
        block_forest[comp[i]].push_back(comp[v]);
      }
    }
  }
}

/*** Example Usage and Output:

Cut-points: 5 1
Bridges:
1 2
5 4
3 7
Blocks, a.k.a. Edge-Biconnected Components:
2 
4 
5 1 0 
7 
3 
6 
Adjacency List for Bridge-Block Forest:
0 => 2
1 => 2
2 => 0 1
3 => 4
4 => 3
5 =>

***/

#include <iostream>
using namespace std;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  int nodes = 8;
  adj.resize(nodes);
  add_edge(0, 1);
  add_edge(0, 5);
  add_edge(1, 2);
  add_edge(1, 5);
  add_edge(3, 7);
  add_edge(4, 5);
  tarjan();
  get_block_forest();
  cout << "Cut-points:";
  for (int v : cutpoints) {
    cout << " " << v;
  }
  cout << endl << "Bridges:" << endl;
  for (auto &[u, v] : bridges) {
    cout << u << " " << v << endl;
  }
  cout << "Blocks, or Edge-Biconnected Components:" << endl;
  for (auto &blk : block) {
    for (int v : blk) {
      cout << v << " ";
    }
    cout << endl;
  }
  cout << "Adjacency List for Bridge-Block Forest:" << endl;
  for (int i = 0, n = static_cast<int>(block.size()); i < n; i++) {
    cout << i << " =>";
    for (int v : block_forest[i]) {
      cout << " " << v;
    }
    cout << endl;
  }
  return 0;
}
