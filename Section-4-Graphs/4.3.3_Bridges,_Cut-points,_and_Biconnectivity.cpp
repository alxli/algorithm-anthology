/*

Given an undirected graph, compute the following properties of the graph using
Tarjan's algorithm. tarjan() applies to a global, pre-populated adjacency list
adj[] which satisfies the precondition that for every node v in adj[u], node u
also exists in adj[v]. Nodes in adj[] must be numbered with integers between 0
(inclusive) and the total number of nodes (exclusive), as passed in the function
arguments. get_block_forest() applies to the global vector of biconnected
components bcc[] which must have already been precomputed by a call to tarjan().

A bridge is an edge such that when deleted, the number of connected components
in the graph is increased. An edge is a bridge if and only if it is not part of
any cycle.

A cut-point (i.e. cut-node, or articulation point) is any node whose removal
increases the number of connected components in the graph.

A biconnected component of a graph is a maximally biconnected subgraph. A
biconnected graph is a connected and "non-separable" graph, meaning that if any
node were to be removed, the graph will remain connected. Thus, a biconnected
graph has no articulation points.

Any connected graph decomposes into a tree of biconnected components called the
"block tree" of the graph. An unconnected graph will thus decompose into a
"block forest."

Time Complexity:
- O(max(n, m)) per call to tarjan() and get_block_forest(), where n is the
  number of nodes and m is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n the number of nodes and m is
  the number of edges
- O(n) auxiliary stack space for tarjan().
- O(1) auxiliary stack space for get_block_forest().

*/

#include <algorithm>
#include <vector>

const int MAXN = 100;
int timer, lowlink[MAXN], tin[MAXN], comp[MAXN];
std::vector<bool> visit(MAXN);
std::vector<int> adj[MAXN], bcc_forest[MAXN];
std::vector<int> stack, cutpoints;
std::vector<std::vector<int> > bcc;
std::vector<std::pair<int, int> > bridges;

void dfs(int u, int p) {
  visit[u] = true;
  lowlink[u] = tin[u] = timer++;
  stack.push_back(u);
  int v, children = 0;
  bool cutpoint = false;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    v = adj[u][j];
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
        bridges.push_back(std::make_pair(u, v));
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
    do {
      v = stack.back();
      stack.pop_back();
      component.push_back(v);
    } while (u != v);
    bcc.push_back(component);
  }
}

void tarjan(int nodes) {
  bcc.clear();
  bridges.clear();
  cutpoints.clear();
  stack.clear();
  std::fill(lowlink, lowlink + nodes, 0);
  std::fill(tin, tin + nodes, 0);
  std::fill(visit.begin(), visit.end(), false);
  timer = 0;
  for (int i = 0; i < nodes; i++) {
    if (!visit[i]) {
      dfs(i, -1);
    }
  }
}

void get_block_forest(int nodes) {
  std::fill(comp, comp + nodes, 0);
  for (int i = 0; i < nodes; i++) {
    bcc_forest[i].clear();
  }
  for (int i = 0; i < (int)bcc.size(); i++) {
    for (int j = 0; j < (int)bcc[i].size(); j++) {
      comp[bcc[i][j]] = i;
    }
  }
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < (int)adj[i].size(); j++) {
      if (comp[i] != comp[adj[i][j]]) {
        bcc_forest[comp[i]].push_back(comp[adj[i][j]]);
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
Edge-Biconnected Components:
2
4
5 1 0
7
3
6
Adjacency List for Block Forest:
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
  add_edge(0, 1);
  add_edge(0, 5);
  add_edge(1, 2);
  add_edge(1, 5);
  add_edge(3, 7);
  add_edge(4, 5);
  tarjan(8);
  get_block_forest(8);
  cout << "Cut-points:";
  for (int i = 0; i < (int)cutpoints.size(); i++) {
    cout << " " << cutpoints[i];
  }
  cout << endl << "Bridges:" << endl;
  for (int i = 0; i < (int)bridges.size(); i++) {
    cout << bridges[i].first << " " << bridges[i].second << endl;
  }
  cout << "Edge-Biconnected Components:" << endl;
  for (int i = 0; i < (int)bcc.size(); i++) {
    for (int j = 0; j < (int)bcc[i].size(); j++) {
      cout << bcc[i][j] << " ";
    }
    cout << endl;
  }
  cout << "Adjacency List for Block Forest:" << endl;
  for (int i = 0; i < (int)bcc.size(); i++) {
    cout << i << " =>";
    for (int j = 0; j < (int)bcc_forest[i].size(); j++) {
      cout << " " << bcc_forest[i][j];
    }
    cout << endl;
  }
  return 0;
}
