/*

Given a directed graph, determine the strongly connected components.
The strongly connected components (SCC) of a graph is the set of all
strongly (maximally) connected subgraphs. A subgraph is strongly
connected if there is a path between each pair of nodes. Condensing the
strongly connected components of a graph into single nodes will result
in a directed acyclic graph.

Time Complexity: O(n) on the number of edges.

Space Complexity: O(n) on the number of edges to store the input graph
as an adjacency list and O(n) auxiliary on the number of nodes.

*/

#include <algorithm> /* std::fill(), std::reverse() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
vector<bool> vis(MAXN);
vector<int> adj[MAXN], order;
vector<vector<int> > scc;

void dfs(vector<int> graph[], vector<int> & res, int u) {
  vis[u] = true;
  for (int j = 0; j < (int)graph[u].size(); j++) {
    if (!vis[graph[u][j]])
      dfs(graph, res, graph[u][j]);
  }
  res.push_back(u);
}

void kosaraju(int nodes) {
  scc.clear();
  order.clear();
  vector<int> rev[nodes];
  fill(vis.begin(), vis.end(), false);
  for (int i = 0; i < nodes; i++) {
    if (!vis[i])
      dfs(adj, order, i);
  }
  for (int i = 0; i < nodes; i++)
    for (int j = 0; j < (int)adj[i].size(); j++)
      rev[adj[i][j]].push_back(i);
  fill(vis.begin(), vis.end(), false);
  reverse(order.begin(), order.end());
  for (int i = 0; i < (int)order.size(); i++) {
    if (vis[order[i]])
      continue;
    vector<int> component;
    dfs(rev, component, order[i]);
    scc.push_back(component);
  }
}

/*** Example Usage

Sample Output:
Component: 1 4 0
Component: 7 3 2
Component: 5 6

***/

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
  for (int i = 0; i < (int)scc.size(); i++) {
    cout << "Component:";
    for (int j = 0; j < (int)scc[i].size(); j++)
      cout << " " << scc[i][j];
    cout << "\n";
  }
  return 0;
}
