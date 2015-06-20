/*

2.1.1 - Depth First Search

Description: Given an unweighted graph, traverse all reachable
nodes from a source node. Each branch is explored as deep as
possible before more branches are visited. DFS only uses as
much space as the length of the longest branch. When DFS'ing
recursively, the internal call-stack could overflow, so
sometimes it is safer to use an explicit stack data structure.

Complexity: O(V+E) on the number of vertices and edges for
explicit graphs traversed without repetition. O(b^d) for
implicit graphs with a branch factor of b, searched to depth d.

=~=~=~=~= Sample Input =~=~=~=~=
12 11 0
0 1
0 6
0 7
1 2
1 5
2 3
2 4
7 8
7 11
8 9
8 10

=~=~=~=~= Sample Output =~=~=~=~=
Nodes visited: 0 1 2 3 4 5 6 7 8 9 10 11

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

void dfs(int u) {
  vis[u] = true;
  cout << " " << u;
  for (int j = 0; j < adj[u].size(); j++)
    if (!vis[adj[u][j]])
      dfs(adj[u][j]);
}

int main() {
  int nodes, edges, start, u, v;
  cin >> nodes >> edges >> start;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  cout << "Nodes visited:";
  dfs(start);
  cout << "\n";
  return 0;
}
