/*

Given a directed acyclic graph (DAG), find one of possibly many orderings of the nodes such that for
every edge from node $u$ to $v$, $u$ comes before $v$ in the ordering.

The DFS version uses the fact that every outgoing dependency of a node is finished before the node
itself is appended, so reversing DFS post-order places each prerequisite before everything it can
reach. Kahn's algorithm instead repeatedly removes nodes with indegree zero: these are exactly the
nodes whose remaining prerequisites have all already been placed. If at some point not every node
can be removed, the leftover nodes must lie on a directed cycle.

- `toposort_dfs()` returns a valid topological ordering using DFS post-order, or an empty vector if
  the graph contains a cycle.
- `toposort_kahn()` returns a valid topological ordering using indegrees and a queue, or returns an
  empty vector if the graph contains a cycle.

Time Complexity:
- O(max(n, m)) per call to either function, where $n$ is the number of nodes and $m$ is the number
  of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary stack and heap space for `toposort_dfs()`.
- O(n) auxiliary heap space for the indegree array and queue in `toposort_kahn()`.

*/

#include <algorithm>
#include <cassert>
#include <queue>
#include <vector>

std::vector<std::vector<int>> adj;

std::vector<int> toposort_dfs() {
  int n = static_cast<int>(adj.size());
  std::vector<char> state(n);
  std::vector<int> res;
  auto dfs = [&](auto &&dfs, int u) -> bool {
    if (state[u] == 1) {
      return false;
    }
    if (state[u] == 2) {
      return true;
    }
    state[u] = 1;
    for (int v : adj[u]) {
      if (!dfs(dfs, v)) {
        return false;
      }
    }
    state[u] = 2;
    res.push_back(u);
    return true;
  };
  for (int i = 0; i < n; i++) {
    if (state[i] == 0 && !dfs(dfs, i)) {
      return {};
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
  if (static_cast<int>(res.size()) != n) {
    return {};
  }
  return res;
}

/*** Example Usage ***/

using namespace std;

int main() {
  //         1
  //         |
  //         v
  // 0 ----> 3 ---> 5
  // |     / |
  // v    v  v
  // 4 -> 6  7 <--- 2
  // ^______________|
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
  assert((dfs_res == vector<int>{2, 1, 0, 4, 3, 7, 6, 5}));
  vector<int> kahn = toposort_kahn();
  assert(dfs_res.size() == adj.size());
  assert(kahn.size() == adj.size());
  vector<int> position(kahn.size());
  for (int i = 0; i < static_cast<int>(kahn.size()); i++) {
    position[kahn[i]] = i;
  }
  for (int u = 0; u < static_cast<int>(adj.size()); u++) {
    for (int v : adj[u]) {
      assert(position[u] < position[v]);
    }
  }
  adj = {{1}, {0}};
  dfs_res = toposort_dfs();
  kahn = toposort_kahn();
  assert(dfs_res.empty());
  assert(kahn.empty());
  return 0;
}
