/*

1.1.3 - Topological Sorting (DFS)

Description: Given a directed acyclic graph (DAG), order the nodes
such that for every edge from a to b, a precedes b in the ordering.
Usually, there is more than one possible valid ordering. The
following program uses DFS to produce one possible ordering.
This can also be used to detect whether the graph is a DAG.
Note that the DFS algorithm here produces a reversed topological
ordering, so the output must be printed backwards. The graph is
stored in an adjacency list.

Complexity: O(V) on the number of vertices.

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

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b;
vector<bool> vis(MAXN), done(MAXN);
vector<int> adj[MAXN], sorted;

void dfs(int node) {
  if (vis[node]) {
    cout << "Error: Graph is not a DAG!\n";
    return;
  }
  if (done[node]) return;
  vis[node] = true;
  for (int j = 0; j < adj[node].size(); j++)
    dfs(adj[node][j]);
  vis[node] = false;
  done[node] = true;
  sorted.push_back(node);
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  for (int i = 0; i < nodes; i++) 
    if (!done[i]) dfs(i);
  cout << "The topological order:";
  for (int i = sorted.size() - 1; i >= 0; i--)
    cout << " " << sorted[i];
  return 0;
}
