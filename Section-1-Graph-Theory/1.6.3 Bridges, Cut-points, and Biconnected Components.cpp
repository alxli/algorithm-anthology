/*

1.6.3 - Bridges, Cut-points, and Biconnected Components (Tarjan's)

Description: The following terms apply to *undirected* graphs.

A bridge is an edge, when deleted, increases the number of
connected components. An edge is a bridge if and only ifit is not
contained in any cycle.

A cut-point (i.e. cut-vertex or articulation point) is any vertex
whose removal increases the number of connected components.

A biconnected component of a graph is a maximally biconnected
subgraph. A biconnected graph is a connected and "nonseparable"
graph, meaning that if any vertex were to be removed, the graph
will remain connected. Therefore, a biconnected graph has no
articulation vertices.

Complexity: O(V + E) on the number of vertices and edges.

=~=~=~=~= Sample Input =~=~=~=~=
8 6
0 1
0 5
1 2
1 5
3 7
4 5

=~=~=~=~= Sample Output =~=~=~=~=
Cut Points: 5 1
Bridges:
1 2
5 4
3 7
Biconnected Components:
Component 1: 2
Component 2: 4
Component 3: 5 1 0
Component 4: 7
Component 5: 3
Component 6: 6

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, counter;
int low[MAXN], num[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN], stack, cutpoints;
vector<vector<int> > bcc;
vector<pair<int, int> > bridges;

void dfs(int a, int p) {
  int b;
  vis[a] = true;
  low[a] = num[a] = counter++;
  stack.push_back(a);
  int children = 0;
  bool cutpoint = false;
  for (int j = 0; j < adj[a].size(); j++) {
    if ((b = adj[a][j]) == p) continue;
    if (vis[b]) {
      low[a] = min(low[a], num[b]);
    } else {
      dfs(b, a);
      low[a] = min(low[a], low[b]);
      cutpoint |= (low[b] >= num[a]);
      if (low[b] > num[a])
        bridges.push_back(make_pair(a, b));
      ++children;
    }
  }
  if (p == -1) cutpoint = (children >= 2);
  if (cutpoint) cutpoints.push_back(a);
  if (low[a] == num[a]) {
    vector<int> component;
    do {
      b = stack.back();
      stack.pop_back();
      component.push_back(b);
    } while (a != b);
    bcc.push_back(component);
  }
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
    adj[b].push_back(a);
  }
  counter = 0;
  for (int i = 0; i < nodes; i++)
    if (!vis[i]) dfs(i, -1);

  cout << "Cut-points:";
  for (int i = 0; i < cutpoints.size(); i++)
    cout << " " << cutpoints[i];
  cout << "\nBridges:\n";
  for (int i = 0; i < bridges.size(); i++)
    cout << bridges[i].first << " " << bridges[i].second << "\n";
  cout << "Biconnected Components:\n";
  for (int i = 0; i < bcc.size(); i++) {
    cout << "Component " << i + 1 << ":";
    for (int j = 0; j < bcc[i].size(); j++)
      cout << " " << bcc[i][j];
    cout << "\n";
  }
  return 0;
}
