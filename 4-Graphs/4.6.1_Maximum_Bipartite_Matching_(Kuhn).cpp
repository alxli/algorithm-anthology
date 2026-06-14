/*

Given two sets of nodes $A = \{0, 1, \ldots, n_1 - 1\}$ and $B = \{0, 1, \ldots, n_2 - 1\}$, as
well as a set of edges $E$ mapping nodes from set $A$ to set $B$, find the
largest possible subset of $E$ containing no edges that share the same node.

Kuhn's algorithm builds the matching one left node at a time. For each, a depth-first search looks
for an augmenting path: an alternating sequence of unmatched and matched edges that reaches a free
right node. Flipping the edges along such a path enlarges the matching by one.

- `kuhn(n2)` populates `match_left` and `match_right`, then returns maximum matching size for a
  global, pre-populated adjacency list `adj` whose left-side nodes are numbered from 0 to
  `adj.size() - 1` and whose right-side neighbors are numbered from 0 to `n2 - 1`.

Time Complexity:
- O(m*(n1 + n2)) per call to `kuhn()`, where $m$ is the number of edges.

Space Complexity:
- O(n1 + n2) auxiliary stack space for `kuhn()`.

*/

#include <vector>

std::vector<int> match_left, match_right;
std::vector<int> visit;
std::vector<std::vector<int>> adj;
int visit_time;

bool dfs(int u) {
  visit[u] = visit_time;
  for (int nb : adj[u]) {
    if (match_right[nb] == -1) {
      match_left[u] = nb;
      match_right[nb] = u;
      return true;
    }
  }
  for (int nb : adj[u]) {
    int v = match_right[nb];
    if (visit[v] != visit_time && dfs(v)) {
      match_left[u] = nb;
      match_right[nb] = u;
      return true;
    }
  }
  return false;
}

int kuhn(int n2) {
  int n1 = static_cast<int>(adj.size());
  match_left.assign(n1, -1);
  match_right.assign(n2, -1);
  visit.assign(n1, 0);
  visit_time = 0;
  int matches = 0;
  for (int i = 0; i < n1; i++) {
    visit_time++;
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

#include <iostream>
using namespace std;

int main() {
  int n1 = 3, n2 = 4;
  adj.assign(n1, {});
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(1);
  adj[1].push_back(2);
  adj[2].push_back(2);
  adj[2].push_back(3);
  cout << "Matched " << kuhn(n2) << " pair(s):" << endl;
  for (int i = 0; i < n2; i++) {
    if (match_right[i] != -1) {
      cout << match_right[i] << " " << i << endl;
    }
  }
  return 0;
}
