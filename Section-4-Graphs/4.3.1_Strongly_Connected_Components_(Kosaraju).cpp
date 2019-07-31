/*

Given a directed graph, determine the strongly connected components, that is,
the set of all strongly (maximally) connected subgraphs. A subgraph is strongly
connected if there is a path between each pair of nodes. Condensing the strongly
connected components of a graph into single nodes will result in a directed
acyclic graph. kosaraju() applies to a global, pre-populated adjacency list
adj[] which must only consist of nodes numbered with integers between 0
(inclusive) and the total number of nodes (exclusive), as passed in the function
argument.

Time Complexity:
- O(max(n, m)) per call to kosaraju(), where n is the number of nodes and m is
  the number of edges.

Space Complexity:
- O(max(n, m)) auxiliary heap space for storage of the graph, where n the number
  of nodes and m is the number of edges.
- O(n) auxiliary stack space for kosaraju().

*/

#include <algorithm>
#include <vector>

const int MAXN = 100;
std::vector<int> adj[MAXN], rev[MAXN];
std::vector<bool> visit(MAXN);
std::vector<std::vector<int> > scc;

void dfs(std::vector<int> g[], std::vector<int> &res, int u) {
  visit[u] = true;
  for (int j = 0; j < (int)g[u].size(); j++) {
    if (!visit[g[u][j]]) {
      dfs(g, res, g[u][j]);
    }
  }
  res.push_back(u);
}

void kosaraju(int nodes) {
  std::fill(visit.begin(), visit.end(), false);
  std::vector<int> order;
  for (int i = 0; i < nodes; i++) {
    rev[i].clear();
    if (!visit[i]) {
      dfs(adj, order, i);
    }
  }
  std::reverse(order.begin(), order.end());
  std::fill(visit.begin(), visit.end(), false);
  for (int i = 0; i < nodes; i++) {
    for (int j = 0; j < (int)adj[i].size(); j++) {
      rev[adj[i][j]].push_back(i);
    }
  }
  scc.clear();
  for (int i = 0; i < (int)order.size(); i++) {
    if (visit[order[i]]) {
      continue;
    }
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
    for (int j = 0; j < (int)scc[i].size(); j++) {
      cout << scc[i][j] << " ";
    }
    cout << endl;
  }
  return 0;
}
