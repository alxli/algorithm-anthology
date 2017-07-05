/*

A Eulerian trail is a path in a graph which contains every edge exactly once. An
Eulerian cycle or circuit is an Eulerian trail which begins and ends on the same
node. A directed graph has an Eulerian cycle if and only if every node has an
in-degree equal to its out-degree, and all of its nodes with nonzero degree
belong to a single strongly connected component. An undirected graph has an
Eulerian cycle if and only if every node has even degree, and all of its nodes
with nonzero degree belong to a single connected component.

Given a graph as an adjacency list along with the starting node of the cycle,
both functions below return a vector containing all nodes reachable from the
starting node in an order which forms an Eulerian cycle. The first node of the
cycle will be repeated as the last element of the vector. All nodes of input
adjacency lists to both functions must be be between 0 and MAXN - 1, inclusive.
In addition, euler_cycle_undirected() requires that for every node v which is
found in adj[u], node u must also be found in adj[v].

Time Complexity:
- O(max(n, m)) per call to either function, where n and m are the numbers of
  nodes and edges respectively.

Space Complexity:
- O(n) auxiliary heap space for euler_cycle_directed(), where n is the number of
  nodes.
- O(n^2) auxiliary heap space for euler_cycle_undirected(), where n is the
  number of nodes. This can be reduced to O(m) auxiliary heap space on the
  number of edges if the used[][] bit matrix is replaced with an
  std::unordered_set<std::pair<int, int>>.

*/

#include <algorithm>
#include <bitset>
#include <vector>

const int MAXN = 100;

std::vector<int> euler_cycle_directed(std::vector<int> adj[], int u) {
  std::vector<int> stack, curr_edge(MAXN), res;
  stack.push_back(u);
  while (!stack.empty()) {
    u = stack.back();
    stack.pop_back();
    while (curr_edge[u] < (int)adj[u].size()) {
      stack.push_back(u);
      u = adj[u][curr_edge[u]++];
    }
    res.push_back(u);
  }
  std::reverse(res.begin(), res.end());
  return res;
}

std::vector<int> euler_cycle_undirected(std::vector<int> adj[], int u) {
  std::bitset<MAXN> used[MAXN];
  std::vector<int> stack, curr_edge(MAXN), res;
  stack.push_back(u);
  while (!stack.empty()) {
    u = stack.back();
    stack.pop_back();
    while (curr_edge[u] < (int)adj[u].size()) {
      int v = adj[u][curr_edge[u]++];
      int mn = std::min(u, v), mx = std::max(u, v);
      if (!used[mn][mx]) {
        used[mn][mx] = true;
        stack.push_back(u);
        u = v;
      }
    }
    res.push_back(u);
  }
  std::reverse(res.begin(), res.end());
  return res;
}

/*** Example Usage and Output:

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
    for (int i = 0; i < (int)cycle.size(); i++) {
      cout << " " << cycle[i];
    }
    cout << endl;
  }
  {
    vector<int> g[5], cycle;
    g[0].push_back(1);
    g[1].push_back(0);
    g[1].push_back(2);
    g[2].push_back(1);
    g[2].push_back(0);
    g[0].push_back(2);
    g[1].push_back(3);
    g[3].push_back(1);
    g[3].push_back(4);
    g[4].push_back(3);
    g[4].push_back(1);
    g[1].push_back(4);
    cycle = euler_cycle_undirected(g, 2);
    cout << "Eulerian cycle from 2 (undirected):";
    for (int i = 0; i < (int)cycle.size(); i++) {
      cout << " " << cycle[i];
    }
    cout << endl;
  }
  return 0;
}
