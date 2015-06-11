/*

1.3.2 - Strongly Connected Components (Tarjan's Algorithm)

Description: Determines the strongly connected components (SCC)
from a given directed graph. Given a directed graph, its SCCs
are its maximal strongly connected sub-graphs. A graph is strongly
connected if there is a path from each node to every other node.
Condensing the strongly connected components of a graph into
single nodes will result in a directed acyclic graph. The input is
stored in an adjacency list.

In this implementation, a vector is used to emulate a stack
for the sake of simplicity. One useful property of Tarjan’s
algorithm is that, while there is nothing special about the
ordering of nodes within each component, the resulting DAG
is produced in reverse topological order.

Complexity: O(V + E) on the number of vertices and edges.

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
Component 1: 5 6
Component 2: 7 3 2
Component 3: 4 1 0

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, counter, ncomp;
int num[MAXN], low[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN], stack;

void dfs(int u) {
  int v;
  low[u] = num[u] = ++counter;
  stack.push_back(u);
  for (int j = 0; j < adj[u].size(); j++) {
    v = adj[u][j];
    if (vis[v]) continue;
    if (num[v] == -1) {
      dfs(v);
      low[u] = min(low[u], low[v]);
    } else {
      low[u] = min(low[u], num[v]);
    }
  }
  if (num[u] != low[u]) return;
  cout << "Component " << ++ncomp << ":";
  do {
    vis[v = stack.back()] = true;
    stack.pop_back();
    cout << " " << v;
  } while (u != v);
  cout << "\n";
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  for (int i = 0; i < nodes; i++) {
    num[i] = low[i] = -1;
  }
  counter = ncomp = 0;
  for (int i = 0; i < nodes; i++)
    if (num[i] == -1) dfs(i);
  return 0;
} 
