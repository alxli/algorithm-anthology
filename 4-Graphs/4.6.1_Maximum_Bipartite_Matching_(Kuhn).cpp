/*

Given two sets of nodes $A = \{0, 1, \ldots, n_1 - 1\}$ and $B = \{0, 1, \ldots, n_2 - 1\}$, as well
as a set of edges $E$ mapping nodes from set $A$ to set $B$, find the largest possible subset of $E$
containing no edges that share the same node.

Kuhn's algorithm builds the matching one left node at a time. For each, a depth-first search looks
for an augmenting path: an alternating sequence of unmatched and matched edges that reaches a free
right node. Flipping the edges along such a path enlarges the matching by one.

- `bipartite_matching(n2)` populates `match_left` and `match_right`, then returns maximum matching
  size for a global, pre-populated adjacency list `adj` whose left-side nodes are numbered
  $[0, `n`)$ and whose right-side neighbors are numbered $[0, `n2`)$, where `n` is `adj.size()`.

Time Complexity:
- O(n_1*m) per call, where $m$ is the number of edges.

Space Complexity:
- O(max(n_1 + n_2, m)) for storage of the graph, where $m$ is the number of edges.
- O(n_1 + n_2) auxiliary heap space and O(n_1) auxiliary stack space per call.

*/

#include <vector>

std::vector<int> match_left, match_right;
std::vector<int> visit;
std::vector<std::vector<int>> adj;
int timer;

bool dfs(int u) {
  visit[u] = timer;
  for (int nb : adj[u]) {
    if (match_right[nb] == -1) {
      match_left[u] = nb;
      match_right[nb] = u;
      return true;
    }
  }
  for (int nb : adj[u]) {
    int v = match_right[nb];
    if (visit[v] != timer && dfs(v)) {
      match_left[u] = nb;
      match_right[nb] = u;
      return true;
    }
  }
  return false;
}

int bipartite_matching(int n2) {
  int n1 = static_cast<int>(adj.size());
  match_left.assign(n1, -1);
  match_right.assign(n2, -1);
  visit.assign(n1, 0);
  timer = 0;
  int matches = 0;
  for (int i = 0; i < n1; i++) {
    timer++;
    if (dfs(i)) {
      matches++;
    }
  }
  return matches;
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
  int pairs = bipartite_matching(n2);
  assert(pairs == 3);
  cout << "Matched " << pairs << " pair(s):" << endl;
  for (int i = 0; i < n2; i++) {
    if (match_right[i] != -1) {
      cout << match_right[i] << " " << i << endl;
    }
  }
  return 0;
}
