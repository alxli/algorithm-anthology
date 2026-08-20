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

std::vector<int> grundy_on_dag(const std::vector<std::vector<int>> &g) {
  std::vector<int> memo(g.size(), -1);
  auto dfs = [&](auto &&dfs, int u) {
    if (memo[u] != -1) {
      return;
    }
    std::vector<char> seen(g[u].size() + 1);
    for (int v : g[u]) {
      dfs(dfs, v);
      int x = memo[v];
      if (x < static_cast<int>(seen.size())) {
        seen[x] = true;
      }
    }
    int res = 0;
    while (res < static_cast<int>(seen.size()) && seen[res]) {
      res++;
    }
    memo[u] = res;
  };
  for (int u = 0; u < static_cast<int>(g.size()); u++) {
    dfs(dfs, u);
  }
  return memo;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<int>> g{{1, 2}, {3}, {3, 4}, {}, {}};
  // Terminal nodes have Grundy 0, their predecessors have mex {0} = 1, and node 0 has mex {1} = 0.
  assert((grundy_on_dag(g) == vector<int>{0, 1, 1, 0, 0}));
  return 0;
}
