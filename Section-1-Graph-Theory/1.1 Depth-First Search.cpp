/*

1.1 - Depth First Search

Description: Given an unweighted graph, traverse all reachable
nodes from a source node. Each branch is explored as deep as
possible before more branches are visited. DFS only uses as
much space as the length of the longest branch. When DFS'ing
recursively, the internal call-stack could overflow, so
sometimes it is safer to use an explicit stack data structure.

Complexity: O(number of edges) for explicit graphs traversed
without repetition. O(b^d) for implicit graphs with a branch
factor of b, searched to depth d.

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
int nodes, edges, start, a, b;
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

void dfs(int n) {
  vis[n] = true;
  cout << " " << n;
  for (int j = 0; j < adj[n].size(); j++)
    if (!vis[adj[n][j]])
      dfs(adj[n][j]);
}

int main() {
  cin >> nodes >> edges >> start;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cout << "Nodes visited:";
  dfs(start); cout << "\n";
  return 0;
}
