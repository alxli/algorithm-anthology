/*

2.1.2 - Floodfill (DFS)

Description: Given a directed graph and a source node,
traverse to all reachable nodes from the source and determine
the total area traveled. Conceptually, the order that nodes
are visited in a "flooding" of the graph should resemble a
BFS ordering. However, if the objective is simply to visit
all reachable nodes without regard for the order (as is the
case with most applications of floodfill in contests), it is
much simpler to DFS because an extra queue is not needed.
The input graph is stored in an adjacency list.

Complexity: O(V+E) on the number of vertices and edges.

=~=~=~=~= Sample Input =~=~=~=~=
8 8 0
0 1
0 5
1 2
1 3
3 2
4 0
4 3
6 7

=~=~=~=~= Sample Output =~=~=~=~=
Visited 5 nodes starting from 0

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

int dfs(int u) {
  vis[u] = true;
  int area = 1;
  for (int j = 0; j < adj[u].size(); j++)
    if (!vis[adj[u][j]])
      area += dfs(adj[u][j]);
  return area;
}

int main() {
  int nodes, edges, start, u, v;
  cin >> nodes >> edges >> start;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  cout << "Visited " << dfs(start) << " nodes";
  cout << " starting from " << start << "\n";
  return 0;
}
