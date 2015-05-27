/*

1.4 - Topological Sorting

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
1 4
1 5
2 4
3 5
3 8
4 6
4 7
4 8
5 7

=~=~=~=~= Sample Output =~=~=~=~=
The topological order:
3 2 1 5 4 8 7 6

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAX_N = 101;
int nodes, edges, a, b;
bool done[MAX_N] = {0}, visit[MAX_N] = {0}; 
vector<int> adj[MAX_N], sorted;

void DFS(int node) {
  if (visit[node]) {
    cout << "Error: Graph is not a DAG!\n";
    return;
  }
  if (done[node]) return;
  visit[node] = 1;
  for (int j = 0; j < adj[node].size(); j++)
    DFS(adj[node][j]);
  visit[node] = 0;
  done[node] = 1;
  sorted.push_back(node);
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  for (int i = 1; i <= nodes; i++) 
    if (!done[i]) DFS(i);
  for (int i = sorted.size() - 1; i >= 0; i--)
    cout << " " << sorted[i];
  return 0;
}
