/*

Given two sets of nodes $A = \{0, 1, \ldots, n_1 - 1\}$ and $B = \{0, 1, \ldots, n_2 - 1\}$, as well
as a set of edges $E$ mapping nodes from set $A$ to set $B$, find the largest possible subset of $E$
containing no edges that share the same node.

Hopcroft-Karp augments along many shortest augmenting paths per phase. Each phase first runs a
breadth-first search to layer the graph by distance, then a depth-first search to find a maximal set
of node-disjoint shortest augmenting paths to flip at once, for O(m*sqrt(n_1 + n_2)) running time
overall.

- `bipartite_matching_hk(n2)` populates `match_left` and `match_right`, then returns maximum
  matching size for a global, pre-populated adjacency list `adj`, whose indices represent left-side
  nodes and whose entries contain right-side neighbors numbered $[0, `n2`)$.

Time Complexity:
- O(m*sqrt(n_1 + n_2)) per call, where $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n_1 + n_2) auxiliary stack space and O(n_1 + n_2) auxiliary heap space.

*/

#include <queue>
#include <vector>

std::vector<std::vector<int>> adj;
std::vector<char> used, visit;
std::vector<int> match_left, match_right, dist;

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
      int v = match_right[nb];
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
    int v = match_right[nb];
    if (v < 0 || (!visit[v] && dist[v] == dist[u] + 1 && dfs(v))) {
      match_left[u] = nb;
      match_right[nb] = u;
      used[u] = true;
      return true;
    }
  }
  return false;
}

int bipartite_matching_hk(int n2) {
  int n1 = static_cast<int>(adj.size());
  match_left.assign(n1, -1);
  match_right.assign(n2, -1);
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

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Left nodes a0..a2, right nodes b0..b3:
  //   a0 -- b1
  //   a1 -- b0, b1, b2
  //   a2 -- b2, b3
  int n1 = 3, n2 = 4;
  adj.assign(n1, {});
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(1);
  adj[1].push_back(2);
  adj[2].push_back(2);
  adj[2].push_back(3);
  int pairs = bipartite_matching_hk(n2);
  assert(pairs == 3);
  assert((match_right == vector<int>{1, 0, 2, -1}));
  return 0;
}
