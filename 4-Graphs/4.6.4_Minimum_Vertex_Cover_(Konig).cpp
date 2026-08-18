/*

Given a bipartite graph with left nodes $A = \{0, \ldots, n_1 - 1\}$ and right nodes
$B = \{0, \ldots, n_2 - 1\}$, find a minimum vertex cover: a smallest set of nodes such that every
edge has at least one endpoint in the set. By Konig's theorem, in a bipartite graph the size of a
minimum vertex cover equals the size of a maximum matching, and the nodes left uncovered form a
maximum independent set.

The construction first computes a maximum matching with Kuhn's algorithm. It then marks every left
node reachable from an unmatched left node by an alternating path (following unmatched edges from
left to right and matched edges from right to left). Konig's theorem says the minimum vertex cover
is exactly the left nodes that are not reachable together with the right nodes that are reachable;
each matched edge contributes exactly one endpoint, so the cover size equals the matching size.

- `min_vertex_cover(n2)` returns the nodes of a minimum vertex cover for the global adjacency list
  `adj` (left nodes $[0, `n1`)$, right neighbors $[0, `n2`)$), where `n1` is `adj.size()`. Each
  returned value is a left node `i` in $[0, `n1`)$, or a right node `j` encoded as `n1 + j`.

Time Complexity:
- O(n_1*m) per call, dominated by the matching, where $m$ is the number of edges.

Space Complexity:
- O(max(n_1 + n_2, m)) for storage of the graph.
- O(n_1 + n_2) auxiliary.

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

std::vector<int> min_vertex_cover(int n2) {
  int n1 = static_cast<int>(adj.size());
  bipartite_matching(n2);
  std::vector<char> reachable_left(n1), reachable_right(n2);
  std::vector<int> st;
  for (int u = 0; u < n1; u++) {
    if (match_left[u] == -1) {
      reachable_left[u] = true;
      st.push_back(u);
    }
  }
  while (!st.empty()) {
    int u = st.back();
    st.pop_back();
    for (int v : adj[u]) {
      if (v != match_left[u] && !reachable_right[v]) {
        reachable_right[v] = true;
        int next = match_right[v];
        if (next != -1 && !reachable_left[next]) {
          reachable_left[next] = true;
          st.push_back(next);
        }
      }
    }
  }
  std::vector<int> cover;
  for (int u = 0; u < n1; u++) {
    if (!reachable_left[u]) {
      cover.push_back(u);
    }
  }
  for (int v = 0; v < n2; v++) {
    if (reachable_right[v]) {
      cover.push_back(n1 + v);
    }
  }
  return cover;
}

/*** Example Usage ***/

#include <algorithm>
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
  vector<int> cover = min_vertex_cover(n2);
  assert(cover.size() == 3);  // Equals the maximum matching size, by Konig's theorem.
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
