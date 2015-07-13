/*

2.1.3 - Topological Sorting (DFS)

Description: Given a directed acyclic graph (DAG), order the nodes
such that for every edge from a to b, a precedes b in the ordering.
Usually, there is more than one possible valid ordering. The
following program uses DFS to produce one possible ordering.
This can also be used to detect whether the graph is a DAG.
Note that the DFS algorithm here produces a reversed topological
ordering, so the output must be printed backwards. The graph is
stored in an adjacency list.

Complexity: O(V+E) on the number of vertices and edges.

=~=~=~=~= Sample Input =~=~=~=~=
8 9
0 3
0 4
1 3
2 4
2 7
3 5
3 6
3 7
4 6

=~=~=~=~= Sample Output =~=~=~=~=
The topological order: 2 1 0 4 3 7 6 5

*/

#include <algorithm> /* std::fill(), std::reverse() */
#include <iostream>
#include <stdexcept> /* std::runtime_error() */
#include <vector>
using namespace std;

const int MAXN = 100;
vector<bool> vis(MAXN), done(MAXN);
vector<int> adj[MAXN], sorted;

void dfs(int u) {
  if (vis[u])
    throw std::runtime_error("Not a DAG.");
  if (done[u]) return;
  vis[u] = true;
  for (int j = 0; j < (int)adj[u].size(); j++)
    dfs(adj[u][j]);
  vis[u] = false;
  done[u] = true;
  sorted.push_back(u);
}

void toposort(int nodes) {
  fill(vis.begin(), vis.end(), false);
  fill(done.begin(), done.end(), false);
  sorted.clear();
  for (int i = 0; i < nodes; i++)
    if (!done[i]) dfs(i);
  reverse(sorted.begin(), sorted.end());
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  toposort(nodes);
  cout << "The topological order:";
  for (int i = 0; i < (int)sorted.size(); i++)
    cout << " " << sorted[i];
  cout << "\n";
  return 0;
}
