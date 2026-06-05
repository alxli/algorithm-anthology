/*

Given a directed graph, determine the strongly connected components. The strongly connected
components of a graph is the set of all strongly (maximally) connected subgraphs. A subgraph is
strongly connected if there is a path between each pair of nodes. Condensing the strongly connected
components of a graph into single nodes will result in a directed acyclic graph. `tarjan()` applies
to a global, pre-populated adjacency list `adj` which must only consist of nodes numbered with
integers between 0 (inclusive) and the total number of nodes (exclusive), as passed in the function
argument.

Time Complexity:
- O(max(n, m)) per call to `tarjan()`, where $n$ is the number of nodes and $m$ is the number of
  edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary stack space for `tarjan()`.

*/

#include <algorithm>
#include <climits>
#include <vector>

const int INF = INT_MAX / 2;
std::vector<std::vector<int>> adj;
std::vector<int> currstack;
int timer;
// lowlink[u] = discovery time of u, or INF once u is assigned to a finished SCC.
// "Is u still on the stack?" is tested as lowlink[u] < INF rather than a separate bool.
std::vector<int> lowlink;
std::vector<bool> visit;
std::vector<std::vector<int>> scc;

void dfs(int u) {
  lowlink[u] = timer++;
  visit[u] = true;
  currstack.push_back(u);
  bool is_component_root = true;
  for (int v : adj[u]) {
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
  int v;
  do {
    v = currstack.back();
    currstack.pop_back();
    lowlink[v] = INF;  // marks v as removed from the stack
    component.push_back(v);
  } while (u != v);
  scc.push_back(component);
}

void tarjan() {
  int nodes = adj.size();
  scc.clear();
  currstack.clear();
  lowlink.assign(nodes, 0);
  visit.assign(nodes, false);
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
  int nodes = 8;
  adj.resize(nodes);
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
  tarjan();
  cout << "Components:" << endl;
  for (auto &comp : scc) {
    for (int v : comp) {
      cout << v << " ";
    }
    cout << endl;
  }
  return 0;
}
