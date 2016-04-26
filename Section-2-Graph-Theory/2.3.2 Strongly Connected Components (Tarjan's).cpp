/*

Given a directed graph, determine the strongly connected components.
The strongly connected components (SCC) of a graph is the set of all
strongly (maximally) connected subgraphs. A subgraph is strongly
connected if there is a path between each pair of nodes. Condensing the
strongly connected components of a graph into single nodes will result
in a directed acyclic graph.

Time Complexity: O(n) on the number of edges.

Space Complexity: O(n) on the number of edges to store the input graph
as an adjacency list and O(n) auxiliary on the number of nodes.

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
  for (int j = 0; j < (int)adj[u].size(); j++) {
    if (!vis[v = adj[u][j]])
      dfs(v);
    if (lowlink[u] > lowlink[v]) {
      lowlink[u] = lowlink[v];
      is_component_root = false;
    }
  }
  if (!is_component_root)
    return;
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
  for (int i = 0; i < nodes; i++) {
    if (!vis[i])
      dfs(i);
  }
}

/*** Example Usage

Sample Output:
Component: 5 6
Component: 7 3 2
Component: 4 1 0

***/

int main() {
  adj[0].push_back(1);
  adj[1].push_back(2);
  adj[1].push_back(4);
  adj[1].push_back(5);
  adj[2].push_back(3);
  adj[2].push_back(6);
  adj[3].push_back(2);
  adj[3].push_back(7);
  adj[4].push_back(0);
  adj[4].push_back(5);
  adj[5].push_back(6);
  adj[6].push_back(5);
  adj[7].push_back(3);
  adj[7].push_back(6);
  tarjan(8);
  for (int i = 0; i < (int)scc.size(); i++) {
    cout << "Component:";
    for (int j = 0; j < (int)scc[i].size(); j++)
      cout << " " << scc[i][j];
    cout << "\n";
  }
  return 0;
}
