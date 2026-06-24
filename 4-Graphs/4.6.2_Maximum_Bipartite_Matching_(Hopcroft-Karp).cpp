/*

Given two sets of nodes $A = \{0, 1, \ldots, n_1 - 1\}$ and $B = \{0, 1, \ldots, n_2 - 1\}$, as
well as a set of edges $E$ mapping nodes from set $A$ to set $B$, find the largest possible subset
of $E$ containing no edges that share the same node.

Hopcroft-Karp augments along many shortest augmenting paths per phase. Each phase first runs a
breadth-first search to layer the graph by distance, then a depth-first search to find a maximal set
of vertex-disjoint shortest augmenting paths to flip at once, for O(m sqrt(n)) running time overall.

- `hopcroft_karp(n2)` populates `match` and returns maximum matching size for a global,
  pre-populated adjacency list `adj` whose left-side nodes are numbered $[0, `n`)$ and whose
  right-side neighbors are numbered $[0, `n2`)$, where `n` is `adj.size()`.

Time Complexity:
- O(m*sqrt(n_1 + n_2)) per call, where $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n_1 + n_2) auxiliary stack and heap space for `hopcroft_karp()`.

*/

#include <algorithm>
#include <queue>
#include <vector>

std::vector<std::vector<int>> adj;
std::vector<char> used, visit;
std::vector<int> match, dist;

void bfs() {
  int n1 = static_cast<int>(adj.size());
  dist.assign(n1, -1);
  std::queue<int> q;
  for (int u = 0; u < n1; u++) {
    if (!used[u]) {
      q.push(u);
      dist[u] = 0;
    }
  }
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int nb : adj[u]) {
      int v = match[nb];
      if (v >= 0 && dist[v] < 0) {
        dist[v] = dist[u] + 1;
        q.push(v);
      }
    }
  }
}

bool dfs(int u) {
  visit[u] = true;
  for (int nb : adj[u]) {
    int v = match[nb];
    if (v < 0 || (!visit[v] && dist[v] == dist[u] + 1 && dfs(v))) {
      match[nb] = u;
      used[u] = true;
      return true;
    }
  }
  return false;
}

int hopcroft_karp(int n2) {
  int n1 = static_cast<int>(adj.size());
  match.assign(n2, -1);
  used.assign(n1, false);
  int res = 0;
  while (true) {
    bfs();
    visit.assign(n1, false);
    int f = 0;
    for (int u = 0; u < n1; u++) {
      if (!used[u] && dfs(u)) {
        f++;
      }
    }
    if (f == 0) {
      return res;
    }
    res += f;
  }
  return res;
}

/*** Example Usage and Output:

Matched 3 pair(s):
1 0
0 1
2 2

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  int n1 = 3, n2 = 4;
  adj.assign(n1, {});
  // Left vertices L0..L2, right vertices R0..R3:
  //   L0 -- R1
  //   L1 -- R0, R1, R2
  //   L2 -- R2, R3
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(1);
  adj[1].push_back(2);
  adj[2].push_back(2);
  adj[2].push_back(3);
  int pairs = hopcroft_karp(n2);
  assert(pairs == 3);
  cout << "Matched " << pairs << " pair(s):" << endl;
  for (int i = 0; i < n2; i++) {
    if (match[i] != -1) {
      cout << match[i] << " " << i << endl;
    }
  }
  return 0;
}
