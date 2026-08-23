/*

Given two sets of nodes $A = \{0, 1, \ldots, n_1 - 1\}$ and $B = \{0, 1, \ldots, n_2 - 1\}$, as well
as a set of edges $E$ mapping nodes from set $A$ to set $B$, find the largest possible subset of $E$
containing no edges that share the same node.

Kuhn's algorithm builds the matching one left node at a time. For each, a depth-first search looks
for an augmenting path: an alternating sequence of unmatched and matched edges that reaches a free
right node. Flipping the edges along such a path enlarges the matching by one.

- `match_bipartite(n2)` populates `match_left` and `match_right`, then returns maximum matching size
  for a global, pre-populated adjacency list `adj`, whose indices represent left-side nodes and
  whose entries contain right-side neighbors numbered $[0, `n2`)$. The two sides are given as
  separate node numberings. Use the bipartite check of section 4.6.1 to produce them for a graph
  that does not already arrive presented as two sets.

Time Complexity:
- O(n_1*m) per call, where $m$ is the number of edges.

Space Complexity:
- O(max(n_1 + n_2, m)) for storage of the graph, where $m$ is the number of edges.
- O(n_1 + n_2) auxiliary heap space and O(n_1) auxiliary stack space.

*/

#include <vector>

std::vector<std::vector<int>> adj;
std::vector<int> match_left, match_right, visit;
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

int match_bipartite(int n2) {
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
  assert(match_bipartite(n2) == 3);
  assert((match_right == vector<int>{1, 0, 2, -1}));
  return 0;
}
