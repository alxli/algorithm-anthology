/*

1.3 - Floodfill

Description: Given a directed graph and a source node,
traverse to all reachable nodes from the source and determine
the total area traveled. Logically, the order that nodes are
visited in a floodfill should resemble a BFS. However, if the
objective is simply to visit all reachable nodes without
regard for the order (as is the case with most applications
of floodfill in contests), it is much simpler to DFS because
an extra queue is not needed. The graph is stored in an
adjacency list.

Complexity: O(V) on the number of vertices.

=~=~=~=~= Sample Input =~=~=~=~=
8 8
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
int nodes, edges, a, b, source;
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

int DFS(int node) {
  if (vis[node]) return 0;
  vis[node] = true;
  int area = 1;
  for (int j = 0; j < adj[node].size(); j++)
    area += DFS(adj[node][j]);
  return area;
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cin >> source;
  cout << "Visited " << DFS(source);
  cout << " nodes starting from " << source;
  return 0;
}
