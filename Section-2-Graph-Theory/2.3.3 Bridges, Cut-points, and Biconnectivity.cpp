/*

2.3.3 - Bridges, Cut-points, and Biconnectivity (Tarjan's)

Description: The following operations apply to undirected graphs.

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

Any connected graph decomposes into a tree of biconnected
components called the "block tree" of the graph. An unconnected
graph will thus decompose into a "block forest."

See: http://en.wikipedia.org/wiki/Biconnected_component

Complexity: O(V+E) on the number of vertices and edges.

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
Edge-Biconnected Components:
Component 1: 2
Component 2: 4
Component 3: 5 1 0
Component 4: 7
Component 5: 3
Component 6: 6
Adjacency List for Block Forest:
0 => 2
1 => 2
2 => 0 1
3 => 4
4 => 3
5 =>

*/

#include <algorithm> /* std::fill(), std::min() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int timer, lowlink[MAXN], tin[MAXN], comp[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN], bcc_forest[MAXN];
vector<int> stack, cutpoints;
vector<vector<int> > bcc;
vector<pair<int, int> > bridges;

void dfs(int u, int p) {
  vis[u] = true;
  lowlink[u] = tin[u] = timer++;
  stack.push_back(u);
  int v, children = 0;
  bool cutpoint = false;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    if ((v = adj[u][j]) == p) continue;
    if (vis[v]) {
      //lowlink[u] = min(lowlink[u], lowlink[v]);
      lowlink[u] = min(lowlink[u], tin[v]);
    } else {
      dfs(v, u);
      lowlink[u] = min(lowlink[u], lowlink[v]);
      cutpoint |= (lowlink[v] >= tin[u]);
      if (lowlink[v] > tin[u])
        bridges.push_back(make_pair(u, v));
      children++;
    }
  }
  if (p == -1) cutpoint = (children >= 2);
  if (cutpoint) cutpoints.push_back(u);
  if (lowlink[u] == tin[u]) {
    vector<int> component;
    do {
      v = stack.back();
      stack.pop_back();
      component.push_back(v);
    } while (u != v);
    bcc.push_back(component);
  }
}

void tarjan(int nodes) {
  bcc.clear();
  bridges.clear();
  cutpoints.clear();
  stack.clear();
  fill(lowlink, lowlink + nodes, 0);
  fill(tin, tin + nodes, 0);
  fill(vis.begin(), vis.end(), false);
  timer = 0;
  for (int i = 0; i < nodes; i++)
    if (!vis[i]) dfs(i, -1);
}

//condenses each bcc to a node and generates a tree
//global variables adj and bcc must be set beforehand
void get_block_tree(int nodes) {
  fill(comp, comp + nodes, 0);
  for (int i = 0; i < nodes; i++) bcc_forest[i].clear();
  for (int i = 0; i < (int)bcc.size(); i++)
    for (int j = 0; j < (int)bcc[i].size(); j++)
      comp[bcc[i][j]] = i;
  for (int i = 0; i < nodes; i++)
    for (int j = 0; j < (int)adj[i].size(); j++)
      if (comp[i] != comp[adj[i][j]])
        bcc_forest[comp[i]].push_back(comp[adj[i][j]]);
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
  tarjan(nodes);
  cout << "Cut-points:";
  for (int i = 0; i < (int)cutpoints.size(); i++)
    cout << " " << cutpoints[i];
  cout << "\nBridges:\n";
  for (int i = 0; i < (int)bridges.size(); i++)
    cout << bridges[i].first << " " << bridges[i].second << "\n";
  cout << "Edge-Biconnected Components:\n";
  for (int i = 0; i < (int)bcc.size(); i++) {
    cout << "Component:";
    for (int j = 0; j < (int)bcc[i].size(); j++)
      cout << " " << bcc[i][j];
    cout << "\n";
  }
  get_block_tree(nodes);
  cout << "Adjacency List for Block Forest:\n";
  for (int i = 0; i < (int)bcc.size(); i++) {
    cout << i << " =>";
    for (int j = 0; j < (int)bcc_forest[i].size(); j++)
      cout << " " << bcc_forest[i][j];
    cout << "\n";
  }
  return 0;
}
