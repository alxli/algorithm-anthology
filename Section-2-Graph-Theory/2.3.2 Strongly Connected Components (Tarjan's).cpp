/*

Given a directed graph, determine the strongly connected components. The
strongly connected components of a graph is the set of all strongly (maximally)
connected subgraphs. A subgraph is strongly connected if there is a path between
each pair of nodes. Condensing the strongly connected components of a graph into
single nodes will result in a directed acyclic graph. tarjan() applies to a
global, pre-populated adjacency list adj[] which must only consist of nodes
numbered with integers between 0 (inclusive) and the total number of nodes
(exclusive), as passed in the function argument.

Time Complexity:
- O(max(n, m)) per call to tarjan(), where n is the number of nodes and m is the
  number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n the number of nodes and m is
  the number of edges.
- O(n) auxiliary stack space for tarjan().

*/

#include <algorithm>
#include <vector>

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<int> adj[MAXN], stack;
int timer, lowlink[MAXN];
std::vector<bool> visit(MAXN);
std::vector<std::vector<int> > scc;

void dfs(int u) {
  lowlink[u] = timer++;
  visit[u] = true;
  stack.push_back(u);
  bool is_component_root = true;
  int v;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    v = adj[u][j];
    if (!visit[v]) {
      dfs(v);
    }
    if (lowlink[u] > lowlink[v]) {
      lowlink[u] = lowlink[v];
      is_component_root = false;
    }
  }
  if (!is_component_root) {
    return;
  }
  std::vector<int> component;
  do {
    v = stack.back();
    visit[v] = true;
    stack.pop_back();
    lowlink[v] = INF;
    component.push_back(v);
  } while (u != v);
  scc.push_back(component);
}

void tarjan(int nodes) {
  scc.clear();
  stack.clear();
  std::fill(lowlink, lowlink + nodes, 0);
  std::fill(visit.begin(), visit.end(), false);
  timer = 0;
  for (int i = 0; i < nodes; i++) {
    if (!visit[i]) {
      dfs(i);
    }
  }
}

/*** Example Usage and Output:

Components:
5 6
7 3 2
4 1 0

***/

#include <iostream>
using namespace std;

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
  cout << "Components:" << endl;
  for (int i = 0; i < (int)scc.size(); i++) {
    for (int j = 0; j < (int)scc[i].size(); j++) {
      cout << scc[i][j] << " ";
    }
    cout << endl;
  }
  return 0;
}
