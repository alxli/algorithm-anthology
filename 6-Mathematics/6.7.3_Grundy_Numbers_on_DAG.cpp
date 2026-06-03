/*

Computes Grundy numbers for impartial games whose positions form a directed
acyclic graph. Each vertex is a game position, and each outgoing edge is a legal
move. Terminal vertices have Grundy number 0; all other vertices take the MEX of
their successors' Grundy numbers.

The implementation uses memoized DFS. It assumes the graph is acyclic; if cycles
are present, the usual finite impartial-game Grundy recurrence is not directly
valid without additional analysis.

- `grundy_on_dag(g)` returns the Grundy number of every vertex in graph `g`.
- `g[u]` contains all positions reachable from position `u` in one move.

Time Complexity:
- O(n + m + s), where $n$ is the number of vertices, $m$ is the number of edges,
  and $s$ is the total cost of MEX sets over outgoing edges.

Space Complexity:
- O(n + d) auxiliary heap and stack space, where $d$ is DFS depth.

*/

#include <vector>

int grundy_dfs(int u, const std::vector<std::vector<int> > &g,
               std::vector<int> *memo) {
  if ((*memo)[u] != -1) {
    return (*memo)[u];
  }
  std::vector<bool> seen(g[u].size() + 1, false);
  for (int i = 0; i < (int)g[u].size(); i++) {
    int v = g[u][i];
    int x = grundy_dfs(v, g, memo);
    if (x < (int)seen.size()) {
      seen[x] = true;
    }
  }
  int res = 0;
  while (res < (int)seen.size() && seen[res]) {
    res++;
  }
  (*memo)[u] = res;
  return res;
}

std::vector<int> grundy_on_dag(const std::vector<std::vector<int> > &g) {
  std::vector<int> memo(g.size(), -1);
  for (int u = 0; u < (int)g.size(); u++) {
    grundy_dfs(u, g, &memo);
  }
  return memo;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<int> > g(5);
  g[0].push_back(1);
  g[0].push_back(2);
  g[1].push_back(3);
  g[2].push_back(3);
  g[2].push_back(4);
  vector<int> grundy = grundy_on_dag(g);
  assert(grundy[3] == 0);
  assert(grundy[4] == 0);
  assert(grundy[1] == 1);
  assert(grundy[2] == 1);
  assert(grundy[0] == 0);
  return 0;
}
