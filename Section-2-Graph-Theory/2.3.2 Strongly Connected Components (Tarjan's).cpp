/*

2.3.2 - Strongly Connected Components (Tarjan's Algorithm)

Description: Determines the strongly connected components (SCC)
from a given directed graph. Given a directed graph, its SCCs
are its maximal strongly connected sub-graphs. A graph is
strongly connected if there is a path from each node to every
other node. Condensing the strongly connected components of a
graph into single nodes will result in a directed acyclic graph.
The input is stored in an adjacency list.

In this implementation, a vector is used to emulate a stack
for the sake of simplicity. One useful property of Tarjan’s
algorithm is that, while there is nothing special about the
ordering of nodes within each component, the resulting DAG
is produced in reverse topological order.

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
Component 1: 5 6
Component 2: 7 3 2
Component 3: 4 1 0

*/

#include <algorithm> /* std::fill() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3f3f3f3f;
int timer, lowlink[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN], stack;
vector<vector<int> > scc;

void dfs(int u) {
  lowlink[u] = timer++;
  vis[u] = true;
  stack.push_back(u);
  bool is_component_root = true;
  int v;
  for (int j = 0; j < adj[u].size(); j++) {
    if (!vis[v = adj[u][j]]) dfs(v);
    if (lowlink[u] > lowlink[v]) {
      lowlink[u] = lowlink[v];
      is_component_root = false;
    }
  }
  if (!is_component_root) return;
  vector<int> component;
  do {
    vis[v = stack.back()] = true;
    stack.pop_back();
    lowlink[v] = INF;
    component.push_back(v);
  } while (u != v);
  scc.push_back(component);
}

void tarjan(int nodes) {
  scc.clear();
  stack.clear();
  fill(lowlink, lowlink + nodes, 0);
  fill(vis.begin(), vis.end(), false);
  timer = 0;
  for (int i = 0; i < nodes; i++)
    if (!vis[i]) dfs(i);
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  tarjan(nodes);
  for (int i = 0; i < scc.size(); i++) {
    cout << "Component:";
    for (int j = 0; j < scc[i].size(); j++)
      cout << " " << scc[i][j];
    cout << "\n";
  }
  return 0;
}
