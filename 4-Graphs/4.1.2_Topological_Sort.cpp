/*

Given a directed acyclic graph, find one of possibly many orderings of the nodes such that for every
edge from node $u$ to $v$, $u$ comes before $v$ in the ordering.

- `toposort_dfs()` returns a valid topological ordering using DFS post-order, or throws if the
  graph contains a cycle.
- `toposort_kahn()` returns a valid topological ordering using indegrees and a queue, or returns an
  empty vector if the graph contains a cycle.

Time Complexity:
- O(max(n, m)) per call to either function, where $n$ is the number of nodes and $m$ is the number
  of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary stack space for `toposort_dfs()`.
- O(n) auxiliary indegree and queue space for `toposort_kahn()`.

*/

#include <algorithm>
#include <cassert>
#include <queue>
#include <stdexcept>
#include <vector>

std::vector<std::vector<int>> adj;

void dfs(int u, std::vector<bool> &visit, std::vector<bool> &done, std::vector<int> &res) {
  if (visit[u]) {
    throw std::runtime_error("Not a directed acyclic graph.");
  }
  if (done[u]) {
    return;
  }
  visit[u] = true;
  for (int v : adj[u]) {
    dfs(v, visit, done, res);
  }
  visit[u] = false;
  done[u] = true;
  res.push_back(u);
}

std::vector<int> toposort_dfs() {
  int n = static_cast<int>(adj.size());
  std::vector<bool> visit(n, false), done(n, false);
  std::vector<int> res;
  for (int i = 0; i < n; i++) {
    if (!done[i]) {
      dfs(i, visit, done, res);
    }
  }
  std::reverse(res.begin(), res.end());
  return res;
}

std::vector<int> toposort_kahn() {
  int n = static_cast<int>(adj.size());
  std::vector<int> indegree(n), res;
  for (int u = 0; u < n; u++) {
    for (int v : adj[u]) {
      indegree[v]++;
    }
  }
  std::queue<int> q;
  for (int i = 0; i < n; i++) {
    if (indegree[i] == 0) {
      q.push(i);
    }
  }
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    res.push_back(u);
    for (int v : adj[u]) {
      if (--indegree[v] == 0) {
        q.push(v);
      }
    }
  }
  return static_cast<int>(res.size()) == n ? res : std::vector<int>();
}

/*** Example Usage and Output:

The topological order: 2 1 0 4 3 7 6 5

***/

#include <iostream>
using namespace std;

int main() {
  adj.assign(8, {});
  adj[0].push_back(3);
  adj[0].push_back(4);
  adj[1].push_back(3);
  adj[2].push_back(4);
  adj[2].push_back(7);
  adj[3].push_back(5);
  adj[3].push_back(6);
  adj[3].push_back(7);
  adj[4].push_back(6);
  vector<int> dfs_res = toposort_dfs();
  cout << "The topological order:";
  for (int v : dfs_res) {
    cout << " " << v;
  }
  cout << endl;
  vector<int> kahn = toposort_kahn();
  vector<int> position(kahn.size());
  for (int i = 0; i < static_cast<int>(kahn.size()); i++) {
    position[kahn[i]] = i;
  }
  for (int u = 0; u < static_cast<int>(adj.size()); u++) {
    for (int v : adj[u]) {
      assert(position[u] < position[v]);
    }
  }
  return 0;
}
