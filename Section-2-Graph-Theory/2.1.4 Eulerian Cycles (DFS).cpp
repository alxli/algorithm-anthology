/*

2.1.4 - Eulerian Cycles (DFS)

Description: A Eulerian trail is a trail in a graph which
visits every edge exactly once. Similarly, an Eulerian circuit
or Eulerian cycle is an Eulerian trail which starts and ends
on the same vertex.

An undirected graph has an Eulerian cycle if and only if every
vertex has even degree, and all of its vertices with nonzero
degree belong to a single connected component.

A directed graph has an Eulerian cycle if and only if every
vertex has equal in degree and out degree, and all of its
vertices with nonzero degree belong to a single strongly
connected component.

Complexity: O(V+E) on the number of vertices and edges.

=~=~=~=~= Sample Input =~=~=~=~=
5 6
0 1
1 2
2 0
1 3
3 4
4 1

=~=~=~=~= Sample Output =~=~=~=~=
Eulerian cycle from 0 (directed): 0 1 3 4 1 2 0
Eulerian cycle from 2 (undirected): 2 1 3 4 1 0 2

*/

#include <algorithm> /* std::reverse() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;

vector<int> euler_cycle_directed(vector<int> adj[], int u) {
  vector<int> stack, res, cur_edge(MAXN);
  stack.push_back(u);
  while (!stack.empty()) {
    u = stack.back();
    stack.pop_back();
    while (cur_edge[u] < (int)adj[u].size()) {
      stack.push_back(u);
      u = adj[u][cur_edge[u]++];
    }
    res.push_back(u);
  }
  reverse(res.begin(), res.end());
  return res;
}

vector<int> euler_cycle_undirected(vector<int> adj[], int u) {
  bool used[MAXN][MAXN] = {0};
  vector<int> stack, res, cur_edge(MAXN);
  stack.push_back(u);
  while (!stack.empty()) {
    u = stack.back();
    stack.pop_back();
    while (cur_edge[u] < (int)adj[u].size()) {
      int v = adj[u][cur_edge[u]++];
      if (!used[min(u, v)][max(u, v)]) {
        used[min(u, v)][max(u, v)] = 1;
        stack.push_back(u);
        u = v;
      }
    }
    res.push_back(u);
  }
  reverse(res.begin(), res.end());
  return res;
}

int main() {
  int nodes, edges, u, v;
  vector<int> g1[5], g2[5], cycle;

  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    g1[u].push_back(v);
    g2[u].push_back(v);
    g2[v].push_back(u);
  }

  cycle = euler_cycle_directed(g1, 0);
  cout << "Eulerian cycle from 0 (directed):";
  for (int i = 0; i < (int)cycle.size(); i++)
    cout << " " << cycle[i];
  cout << "\n";

  cycle = euler_cycle_undirected(g2, 2);
  cout << "Eulerian cycle from 2 (undirected):";
  for (int i = 0; i < (int)cycle.size(); i++)
    cout << " " << cycle[i];
  cout << "\n";
  return 0;
}
