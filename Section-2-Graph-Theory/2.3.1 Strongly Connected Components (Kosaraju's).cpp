/*

Given a directed graph, determine the strongly connected components. The
strongly connected components of a graph is the set of all strongly (maximally)
connected subgraphs. A subgraph is strongly connected if there is a path between
each pair of nodes. Condensing the strongly connected components of a graph into
single nodes will result in a directed acyclic graph. kosaraju() applies to a
global, pre-populated adjacency list adj[] which must only consist of nodes
numbered with integers between 0 (inclusive) and the total number of nodes
(exclusive), as passed in the function argument.

Time Complexity: O(max(n, m)) on the number of nodes and edges.
Space Complexity: O(max(n, m)) auxiliary on the number of nodes and edges.

*/

#include <algorithm>  // std::fill(), std::reverse()
#include <vector>

const int MAXN = 100;
std::vector<int> adj[MAXN], rev[MAXN];
std::vector<bool> vis(MAXN);
std::vector<std::vector<int> > scc;

void dfs(std::vector<int> g[], std::vector<int> &res, int u) {
  vis[u] = true;
  for (int j = 0; j < (int)g[u].size(); j++) {
    if (!vis[g[u][j]])
      dfs(g, res, g[u][j]);
  }
  res.push_back(u);
}

void kosaraju(int nodes) {
  std::fill(vis.begin(), vis.end(), false);
  std::vector<int> order;
  for (int i = 0; i < nodes; i++) {
    rev[i].clear();
    if (!vis[i])
      dfs(adj, order, i);
  }
  std::reverse(order.begin(), order.end());
  std::fill(vis.begin(), vis.end(), false);
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < (int)adj[i].size(); j++)
      rev[adj[i][j]].push_back(i);
  }
  scc.clear();
  for (int i = 0; i < (int)order.size(); i++) {
    if (vis[order[i]])
      continue;
    std::vector<int> component;
    dfs(rev, component, order[i]);
    scc.push_back(component);
  }
}

/*** Example Usage and Output:

1 4 0
7 3 2
5 6

***/

#include <iostream>
using namespace std;

int main() {
  adj[0].push_back(1);
  adj[1].push_back(2);
  adj[1].push_back(4);
  adj[1].push_back(5);
  adj[2].push_back(3);
  adj[2].push_back(6);
  adj[3].push_back(2);
  adj[3].push_back(7);
  adj[4].push_back(0);
  adj[4].push_back(5);
  adj[5].push_back(6);
  adj[6].push_back(5);
  adj[7].push_back(3);
  adj[7].push_back(6);
  kosaraju(8);
  cout << "Components:" << endl;
  for (int i = 0; i < (int)scc.size(); i++) {
    for (int j = 0; j < (int)scc[i].size(); j++)
      cout << scc[i][j] << " ";
    cout << endl;
  }
  return 0;
}
