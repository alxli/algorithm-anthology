/*

Given a directed acyclic graph, find one of possibly many orderings of the nodes
such that for every edge from node u to v, u comes before v in the ordering.
Depth-first search is used to traverse all nodes in post-order.

toposort(nodes) takes a directed graph stored as a global adjacency list with
nodes indexed from 0 to (nodes - 1) and assigns a valid topological ordering to
the global result vector. An error is thrown if the graph contains a cycle.

Time Complexity:
- O(max(n, m)) per call to toposort(), where n is the number of nodes and m is
  the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n is the number of nodes and m
  is the number of edges.
- O(n) auxiliary stack space for toposort().

*/

#include <algorithm>
#include <stdexcept>
#include <vector>

const int MAXN = 100;
std::vector<int> adj[MAXN], res;
std::vector<bool> visit(MAXN), done(MAXN);

void dfs(int u) {
  if (visit[u]) {
    throw std::runtime_error("Not a directed acyclic graph.");
  }
  if (done[u]) {
    return;
  }
  visit[u] = true;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    dfs(adj[u][j]);
  }
  visit[u] = false;
  done[u] = true;
  res.push_back(u);
}

void toposort(int nodes) {
  fill(visit.begin(), visit.end(), false);
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
