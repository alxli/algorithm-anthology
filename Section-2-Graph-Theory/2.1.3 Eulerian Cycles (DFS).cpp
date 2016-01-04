/*

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

*/

#include <algorithm> /* std::max(), std::min(), std::reverse() */
#include <vector>

const int MAXN = 100;

std::vector<int> euler_cycle_directed(std::vector<int> adj[], int u) {
  std::vector<int> stack, res, cur_edge(MAXN);
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
  std::reverse(res.begin(), res.end());
  return res;
}

std::vector<int> euler_cycle_undirected(std::vector<int> adj[], int u) {
  std::vector< std::vector<bool> > used(MAXN, std::vector<bool>(MAXN));
  std::vector<int> stack, res, cur_edge(MAXN);
  stack.push_back(u);
  while (!stack.empty()) {
    u = stack.back();
    stack.pop_back();
    while (cur_edge[u] < (int)adj[u].size()) {
      int v = adj[u][cur_edge[u]++];
      int mn = std::min(u, v), mx = std::max(u, v);
      if (!used[mn][mx]) {
        used[mn][mx] = 1;
        stack.push_back(u);
        u = v;
      }
    }
    res.push_back(u);
  }
  std::reverse(res.begin(), res.end());
  return res;
}

/*** Example Usage

Sample Output:
Eulerian cycle from 0 (directed): 0 1 3 4 1 2 0
Eulerian cycle from 2 (undirected): 2 1 3 4 1 0 2

***/

#include <iostream>
using namespace std;

int main() {
  {
    vector<int> g[5], cycle;
    g[0].push_back(1);
    g[1].push_back(2);
    g[2].push_back(0);
    g[1].push_back(3);
    g[3].push_back(4);
    g[4].push_back(1);
    cycle = euler_cycle_directed(g, 0);
    cout << "Eulerian cycle from 0 (directed):";
    for (int i = 0; i < (int)cycle.size(); i++)
      cout << " " << cycle[i];
    cout << endl;
  }
  {
    vector<int> g[5], cycle;
    g[0].push_back(1); g[1].push_back(0);
    g[1].push_back(2); g[2].push_back(1);
    g[2].push_back(0); g[0].push_back(2);
    g[1].push_back(3); g[3].push_back(1);
    g[3].push_back(4); g[4].push_back(3);
    g[4].push_back(1); g[1].push_back(4);
    cycle = euler_cycle_undirected(g, 2);
    cout << "Eulerian cycle from 2 (undirected):";
    for (int i = 0; i < (int)cycle.size(); i++)
      cout << " " << cycle[i];
    cout << endl;
  }
  return 0;
}
