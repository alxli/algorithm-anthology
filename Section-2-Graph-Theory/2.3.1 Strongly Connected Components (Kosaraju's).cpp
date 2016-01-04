/*

Description: Determines the strongly connected components (SCC)
from a given directed graph. Given a directed graph, its SCCs
are its maximal strongly connected sub-graphs. A graph is
strongly connected if there is a path from each node to every
other node. Condensing the strongly connected components of a
graph into single nodes will result in a directed acyclic graph.
The input is stored in an adjacency list.

Complexity: O(V+E) on the number of vertices and edges.

Comparison with other SCC algorithms:
The strongly connected components of a graph can be efficiently
computed using Kosaraju's algorithm, Tarjan's algorithm, or the
path-based strong component algorithm. Tarjan's algorithm can
be seen as an improved version of Kosaraju's because it performs
a single DFS rather than two. Though they both have the same
complexity, Tarjan's algorithm is much more efficient in
practice. However, Kosaraju's algorithm is conceptually simpler.

=~=~=~=~= Sample Input =~=~=~=~=
8 14
0 1
1 2
1 4
1 5
2 3
2 6
3 2
3 7
4 0
4 5
5 6
6 5
7 3
7 6

=~=~=~=~= Sample Output =~=~=~=~=
Component: 1 4 0
Component: 7 3 2
Component: 5 6

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
  for (int j = 0; j < (int)graph[u].size(); j++)
    if (!vis[graph[u][j]])
      dfs(graph, res, graph[u][j]);
  res.push_back(u);
}

void kosaraju(int nodes) {
  scc.clear();
  order.clear();
  vector<int> rev[nodes];
  fill(vis.begin(), vis.end(), false);
  for (int i = 0; i < nodes; i++)
    if (!vis[i]) dfs(adj, order, i);
  for (int i = 0; i < nodes; i++)
    for (int j = 0; j < (int)adj[i].size(); j++)
      rev[adj[i][j]].push_back(i);
  fill(vis.begin(), vis.end(), false);
  reverse(order.begin(), order.end());
  for (int i = 0; i < (int)order.size(); i++) {
    if (vis[order[i]]) continue;
    vector<int> component;
    dfs(rev, component, order[i]);
    scc.push_back(component);
  }
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  kosaraju(nodes);
  for (int i = 0; i < (int)scc.size(); i++) {
    cout << "Component:";
    for (int j = 0; j < (int)scc[i].size(); j++)
      cout << " " << scc[i][j];
    cout << "\n";
  }
  return 0;
}
