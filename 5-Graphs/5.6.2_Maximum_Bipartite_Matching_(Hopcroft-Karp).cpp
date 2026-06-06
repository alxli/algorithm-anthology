/*

Given two sets of nodes $A = \{0, 1, \ldots, n_1 - 1\}$ and $B = \{0, 1, \ldots, n_2 - 1\}$, as
well as a set of edges $E$ mapping nodes from set $A$ to set $B$, find the
largest possible subset of $E$ containing no edges that share the same node. `hopcroft_karp()`
applies to a global, pre-populated adjacency list `adj` which must only consist of nodes numbered
with integers between 0 (inclusive) and $n_2$ (exclusive). The left side size is inferred from
`adj.size()`.

Time Complexity:
- O(m*sqrt(n1 + n2)) per call to `hopcroft_karp()`, where $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n1 + n2) auxiliary stack and heap space for `hopcroft_karp()`.

*/

#include <algorithm>
#include <queue>
#include <vector>

std::vector<std::vector<int>> adj;
std::vector<bool> used, visit;
std::vector<int> match, dist;

void bfs() {
  int n1 = adj.size();
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
  int n1 = adj.size();
  match.assign(n2, -1);
  used.assign(n1, false);
  int res = 0;
  for (;;) {
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

#include <iostream>
using namespace std;

int main() {
  int n1 = 3, n2 = 4;
  adj.resize(n1);
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(1);
  adj[1].push_back(2);
  adj[2].push_back(2);
  adj[2].push_back(3);
  cout << "Matched " << hopcroft_karp(n2) << " pair(s):" << endl;
  for (int i = 0; i < n2; i++) {
    if (match[i] != -1) {
      cout << match[i] << " " << i << endl;
    }
  }
  return 0;
}
