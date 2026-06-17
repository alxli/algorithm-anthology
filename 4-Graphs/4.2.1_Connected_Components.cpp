/*

Given an undirected graph, label every node with its connected component and collect the nodes in
each component. This is the standard graph reachability primitive behind many larger routines: run
one depth-first search from every unseen node, and all nodes visited by that search belong to the
same component. For directed graphs, use a strongly connected components algorithm instead.

- `connected_components()` populates `comp_id[]` (component ID for each node) and `components[]`
  (nodes for each component ID) given a global, bidirectionally pre-populated adjacency list `adj`
  which must consist of nodes numbered [0, `n`), where `n` is `adj.size()`.

Time Complexity:
- O(max(n, m)) per call to `connected_components()`, where $n$ is the number of nodes and $m$ is the
  number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary stack space for `dfs()`.

*/

#include <algorithm>
#include <vector>

std::vector<std::vector<int>> adj;
std::vector<std::vector<int>> components;
std::vector<int> comp_id;

void dfs(int u, int id) {
  comp_id[u] = id;
  components[id].push_back(u);
  for (int v : adj[u]) {
    if (comp_id[v] == -1) {
      dfs(v, id);
    }
  }
}

void connected_components() {
  int n = static_cast<int>(adj.size());
  components.clear();
  comp_id.assign(n, -1);
  for (int u = 0; u < n; u++) {
    if (comp_id[u] == -1) {
      components.push_back({});
      dfs(u, static_cast<int>(components.size()) - 1);
    }
  }
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  adj.assign(6, {});
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(3, 4);
  connected_components();
  assert(components.size() == 3);
  assert(comp_id[0] == comp_id[2]);
  assert(comp_id[0] != comp_id[3]);
  assert(comp_id[5] == 2);
  return 0;
}
