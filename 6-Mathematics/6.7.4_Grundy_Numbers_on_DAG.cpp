/*

Computes Grundy numbers for impartial games whose positions form a directed acyclic graph. Each node
is a game position, and each outgoing edge is a legal move. Terminal nodes have Grundy number $0$;
all other nodes take the MEX of their successors' Grundy numbers.

The implementation uses memoized DFS. It assumes the graph is acyclic; if cycles are present, the
usual finite impartial-game Grundy recurrence is not directly valid without additional analysis.

- `grundy_on_dag(g)` returns the Grundy number of every node in graph `g`, where `g[u]` contains all
  positions reachable from position `u` in one move.

Time Complexity:
- O(n + m) per call, where $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(d) auxiliary stack space and O(n + m) auxiliary heap space, where $d$ is DFS depth.

*/

#include <vector>

int grundy_dfs(int u, const std::vector<std::vector<int>> &g, std::vector<int> *memo) {
  if ((*memo)[u] != -1) {
    return (*memo)[u];
  }
  std::vector<char> seen(g[u].size() + 1, false);
  for (int v : g[u]) {
    int x = grundy_dfs(v, g, memo);
    if (x < static_cast<int>(seen.size())) {
      seen[x] = true;
    }
  }
  int res = 0;
  while (res < static_cast<int>(seen.size()) && seen[res]) {
    res++;
  }
  (*memo)[u] = res;
  return res;
}

std::vector<int> grundy_on_dag(const std::vector<std::vector<int>> &g) {
  std::vector<int> memo(g.size(), -1);
  for (int u = 0; u < static_cast<int>(g.size()); u++) {
    grundy_dfs(u, g, &memo);
  }
  return memo;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<int>> g{{1, 2}, {3}, {3, 4}, {}, {}};
  auto grundy = grundy_on_dag(g);
  // Terminal nodes have Grundy 0; their predecessors have mex {0} = 1.
  assert(grundy[3] == 0);
  assert(grundy[4] == 0);
  assert(grundy[1] == 1);
  assert(grundy[2] == 1);
  // Node 0 can move to two positions with Grundy 1, so mex {1} = 0.
  assert(grundy[0] == 0);
  return 0;
}
