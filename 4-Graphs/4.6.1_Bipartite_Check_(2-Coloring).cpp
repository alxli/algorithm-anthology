/*

Given an undirected graph, determine whether its nodes can be split into two sets such that every
edge joins a node of one set to a node of the other. A graph admitting such a split is bipartite,
and the split is exactly a proper coloring with two colors. Both routines below give each node the
color opposite to the node they reached it from, so an edge joining two nodes of equal color closes
a cycle of odd length. No bipartition can accommodate an odd cycle, so the first such edge decides
the answer.

Only the depth-first version reports the cycle it found. A depth-first conflict is always a back
edge to an ancestor, so the cycle is one walk up the parent chain, while a breadth-first conflict
may join two nodes in unrelated subtrees and would cost a depth array and a climb from both ends.

- `bipartite_dfs()` returns whether the graph is bipartite for a global, bidirectionally
  pre-populated adjacency list `adj` whose indices represent the nodes. On success it populates the
  global array `side` with 0 or 1 for every node and leaves `odd_cycle` empty. On failure it leaves
  `side` partially filled and populates `odd_cycle` with the nodes of an odd cycle, in cycle order.
- `bipartite_bfs()` returns the same answer and coloring using a queue instead of recursion, which
  suits graphs deep enough to overflow the call stack. It does not compute `odd_cycle` on failure.

Each connected component is colored independently, so a disconnected graph receives one arbitrary
orientation of the two colors per component. The sections that follow take the two sides as separate
node numberings: number the nodes with `side[u]` of 0 and those with `side[u]` of 1 each
consecutively from 0, then store only the neighbors of the first set.

Time Complexity:
- O(max(n, m)) per call, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary heap space, plus O(n) auxiliary stack space for `bipartite_dfs()`.

*/

#include <algorithm>
#include <queue>
#include <vector>

std::vector<std::vector<int>> adj;
std::vector<int> side, odd_cycle;

bool bipartite_dfs() {
  int n = static_cast<int>(adj.size());
  side.assign(n, -1);
  std::vector<int> parent(n, -1);
  odd_cycle.clear();
  // A back edge to an ancestor of equal color closes an odd cycle along the tree path to it.
  auto dfs = [&](auto &&dfs, int u) -> bool {
    for (int v : adj[u]) {
      if (side[v] == -1) {
        side[v] = side[u] ^ 1;
        parent[v] = u;
        if (!dfs(dfs, v)) {
          return false;
        }
      } else if (side[v] == side[u]) {
        for (int x = u; x != v; x = parent[x]) {
          odd_cycle.push_back(x);
        }
        odd_cycle.push_back(v);
        std::reverse(odd_cycle.begin(), odd_cycle.end());
        return false;
      }
    }
    return true;
  };
  for (int u = 0; u < n; u++) {
    if (side[u] == -1) {
      side[u] = 0;
      if (!dfs(dfs, u)) {
        return false;
      }
    }
  }
  return true;
}

bool bipartite_bfs() {
  int n = static_cast<int>(adj.size());
  side.assign(n, -1);
  for (int s = 0; s < n; s++) {
    if (side[s] != -1) {
      continue;
    }
    side[s] = 0;
    std::queue<int> q;
    for (q.push(s); !q.empty(); q.pop()) {
      int u = q.front();
      for (int v : adj[u]) {
        if (side[v] == -1) {
          side[v] = side[u] ^ 1;
          q.push(v);
        } else if (side[v] == side[u]) {
          return false;
        }
      }
    }
  }
  return true;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
}

int main() {
  // 0---1
  // |   |    4
  // 3---2
  adj.assign(5, {});
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(2, 3);
  add_edge(3, 0);
  assert(bipartite_dfs() && odd_cycle.empty());
  assert(side[0] == side[2] && side[1] == side[3] && side[0] != side[1]);
  assert(bipartite_bfs());
  assert(side[0] == side[2] && side[1] == side[3] && side[0] != side[1]);

  // 0---1---3
  //  \ /
  //   2
  adj.assign(4, {});
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(2, 0);
  add_edge(1, 3);
  assert(!bipartite_dfs());
  assert((odd_cycle == vector<int>{0, 1, 2}));
  assert(!bipartite_bfs());
  return 0;
}
