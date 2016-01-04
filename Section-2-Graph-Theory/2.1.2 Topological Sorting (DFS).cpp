/*

Given a directed acyclic graph (DAG), find an ordering of the nodes
such that for every edge from nodes u to v, u comes before v in the
ordering. There may exist more than one possible topological ordering,
in which case the following program will produce only one. Depth-first
search can be used to produce all vertices in postorder, which happens
to be the reverse of a valid topological ordering. By definition, the
a node from a postorder traversal is always visited after all nodes
that come earlier in the list.

The toposort() function takes a directed graph stored as an adjacency
list with nodes labeled from 0 to nodes - 1 and sets the global result
vector to a valid topological ordering. If the graph contains a cycle,
then an error is thrown. This is also known as Tarjan's algorithm.

Time Complexity: O(n) on the number of edges.
Space Complexity: O(n) auxiliary on the number of nodes.

*/

#include <algorithm> /* std::fill(), std::reverse() */
#include <stdexcept> /* std::runtime_error() */
#include <vector>

const int MAXN = 100;

std::vector<bool> vis(MAXN), done(MAXN);
std::vector<int> adj[MAXN], res;

void dfs(int u) {
  if (vis[u])
    throw std::runtime_error("Not a DAG.");
  if (done[u])
    return;
  vis[u] = true;
  for (int j = 0; j < (int)adj[u].size(); j++)
    dfs(adj[u][j]);
  vis[u] = false;
  done[u] = true;
  res.push_back(u);
}

void toposort(int nodes) {
  fill(vis.begin(), vis.end(), false);
  fill(done.begin(), done.end(), false);
  res.clear();
  for (int i = 0; i < nodes; i++) {
    if (!done[i])
      dfs(i);
  }
  std::reverse(res.begin(), res.end());
}

/*** Example Usage

Sample Output:
The topological order: 2 1 0 4 3 7 6 5

***/

#include <iostream>
using namespace std;

int main() {
  adj[0].push_back(3);
  adj[0].push_back(4);
  adj[1].push_back(3);
  adj[2].push_back(4);
  adj[2].push_back(7);
  adj[3].push_back(5);
  adj[3].push_back(6);
  adj[3].push_back(7);
  adj[4].push_back(6);
  toposort(8);
  cout << "The topological order:";
  for (int i = 0; i < (int)res.size(); i++)
    cout << " " << res[i];
  cout << endl;
  return 0;
}
