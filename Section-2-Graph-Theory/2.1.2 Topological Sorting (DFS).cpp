/*

Given a directed acyclic graph (a.k.a. DAG), find one of possibly many orderings
of the nodes such that for every edge from node u to v, u comes before v in the
output ordering. Depth-first search is used to traverse all nodes in post-order,
which yields a reversed topological ordering.

toposort() takes a directed graph stored as a global adjacency list with nodes
indexed from 0 to (nodes - 1) and assigns a valid topological ordering to the
global result vector. If the graph contains a cycle, then an error is thrown.

Time Complexity: O(max(n, m)) on the numbers of nodes and edges respectively.
Space Complexity: O(max(n, m)) on the numbers of nodes and edges respectively.

*/

#include <algorithm>
#include <stdexcept>
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
  for (int j = 0; j < (int)adj[u].size(); j++) {
    dfs(adj[u][j]);
  }
  vis[u] = false;
  done[u] = true;
  res.push_back(u);
}

void toposort(int nodes) {
  fill(vis.begin(), vis.end(), false);
  fill(done.begin(), done.end(), false);
  res.clear();
  for (int i = 0; i < nodes; i++) {
    if (!done[i]) {
      dfs(i);
    }
  }
  std::reverse(res.begin(), res.end());
}

/*** Example Usage and Output:

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
  for (int i = 0; i < (int)res.size(); i++) {
    cout << " " << res[i];
  }
  cout << endl;
  return 0;
}
