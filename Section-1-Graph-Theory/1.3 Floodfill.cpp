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

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAX_N = 101;
int nodes, edges, a, b, source;
bool visit[MAX_N] = {0};
vector<int> adj[MAX_N];

int DFS(int node) {
  if (visit[node]) return 0;
  visit[node] = 1;
  cout << ", " << node; 
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
  cout << " nodes, starting from " << source;
  return 0;
}

/*

=~=~=~=~= Sample Input =~=~=~=~=
8 8
1 2
1 6
2 3
2 4
4 3
5 1
5 4
7 8

=~=~=~=~= Sample Output =~=~=~=~=
Visiting 1, 2, 3, 4, 6
Visited 5 nodes, starting from 1.

*/
