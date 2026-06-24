/*

Given a bipartite graph with left nodes $A = \{0, \ldots, n_1 - 1\}$ and right nodes
$B = \{0, \ldots, n_2 - 1\}$, find a minimum vertex cover: a smallest set of nodes such that every
edge has at least one endpoint in the set. By Koenig's theorem, in a bipartite graph the size of a
minimum vertex cover equals the size of a maximum matching, and the nodes left uncovered form a
maximum independent set.

The construction first computes a maximum matching with Kuhn's algorithm. It then marks every left
node reachable from an unmatched left node by an alternating path (following unmatched edges from
left to right and matched edges from right to left). Koenig's theorem says the minimum vertex cover
is exactly the left nodes that are not reachable together with the right nodes that are reachable;
each matched edge contributes exactly one endpoint, so the cover size equals the matching size.

- `min_vertex_cover(n2)` returns the nodes of a minimum vertex cover for the global adjacency list
  `adj` (left nodes $[0, `n1`)$, right neighbors $[0, `n2`)$), where `n1` is `adj.size()`. Each
  returned value is a left node `i` in $[0, `n1`)$, or a right node `j` encoded as `n1 + j`.

The adjacency list `adj` is shared with the matching routine: `adj[u]` lists the right-side
neighbors of left node `u`. Cover nodes are reported with right node `j` encoded as `n1 + j` so the
two sides share one index space.

Time Complexity:
- O(m*(n_1 + n_2)) per call, dominated by the matching, where $m$ is the number of edges.

Space Complexity:
- O(n_1 + n_2) auxiliary space.

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

std::vector<int> min_vertex_cover(int n2) {
  int n1 = static_cast<int>(adj.size());
  kuhn(n2);
  // reachable_left[u] becomes true for left nodes on an alternating path from an unmatched left
  // node; seen_right[e] for the right nodes on those paths.
  std::vector<char> reachable_left(n1, true), seen_right(n2, false);
  for (int u = 0; u < n1; u++) {
    if (match_left[u] != -1) {
      reachable_left[u] = false;
    }
  }
  std::vector<int> stack;
  for (int u = 0; u < n1; u++) {
    if (reachable_left[u]) {
      stack.push_back(u);
    }
  }
  while (!stack.empty()) {
    int u = stack.back();
    stack.pop_back();
    reachable_left[u] = true;
    for (int e : adj[u]) {
      if (!seen_right[e] && match_right[e] != -1) {
        seen_right[e] = true;
        stack.push_back(match_right[e]);  // Cross the matched edge back to the left.
      }
    }
  }
  std::vector<int> cover;
  for (int u = 0; u < n1; u++) {
    if (!reachable_left[u]) {
      cover.push_back(u);
    }
  }
  for (int e = 0; e < n2; e++) {
    if (seen_right[e]) {
      cover.push_back(n1 + e);
    }
  }
  return cover;
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  // Left {0,1,2} and right {0,1,2,3}; a maximum matching has size 3.
  int n1 = 3, n2 = 4;
  adj.assign(n1, {});
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(1);
  adj[1].push_back(2);
  adj[2].push_back(2);
  adj[2].push_back(3);
  vector<int> cover = min_vertex_cover(n2);
  assert(cover.size() == 3);  // Equals the maximum matching size, by Koenig's theorem.
  // The cover is valid: every edge has at least one endpoint in it.
  for (int u = 0; u < n1; u++) {
    for (int v : adj[u]) {
      bool left_in = find(cover.begin(), cover.end(), u) != cover.end();
      bool right_in = find(cover.begin(), cover.end(), n1 + v) != cover.end();
      assert(left_in || right_in);
    }
  }
  return 0;
}
