/*

A Eulerian trail is a path in a graph which contains every edge exactly
once. An Eulerian cycle or circuit is an Eulerian trail which begins
and ends on the same vertex. A directed graph has an Eulerian cycle if
and only if every vertex has an in-degree equal to its out-degree, and
all of its vertices with nonzero degree belong to a single strongly
connected component. An undirected graph has an Eulerian cycle if and
only if every vertex has even degree, and all of its vertices with
nonzero degree belong to a single connected component.

Both functions below take a graph as an adjacency list along with the
starting node of the cycle, returning a vector containing all nodes
reachable from the starting node in an order which forms an Eulerian
cycle. The first node of the vector will be repeated as the last.

Both euler_cycle_directed() and euler_cycle_undirected() require all
elements of its adj[] vectors to be between 0 and MAXN - 1, inclusive.
However, only euler_cycle_undirected() requires that for every node v
which exists in adj[u], node u must exist in adj[v].

Time Complexity: O(n) on the number of edges for both functions.

Space Complexity: euler_cycle_directed() requires O(n) auxiliary on the
number of nodes. euler_cycle_undirected() requires O(n^2) auxiliary on
the number of nodes, or O(n) auxiliary on the number of edges if the
used[][] matrix is replaced a hashset of pairs.

*/

#include <algorithm> /* std::max(), std::min(), std::reverse() */
#include <bitset>
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
  std::bitset<MAXN> used[MAXN];
  std::vector<int> stack, res, cur_edge(MAXN);
  stack.push_back(u);
  while (!stack.empty()) {
    u = stack.back();
    stack.pop_back();
    while (cur_edge[u] < (int)adj[u].size()) {
      int v = adj[u][cur_edge[u]++];
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
