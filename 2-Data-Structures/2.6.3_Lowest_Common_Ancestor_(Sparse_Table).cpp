/*

Given a tree, determine the lowest common ancestor of any two nodes in the tree.
The lowest common ancestor of two nodes u and v is the node that has the longest
distance from the root while having both u and v as its descendant. A nodes is
considered to be a descendant of itself. build() applies to a global,
pre-populated adjacency list adj[] which must only consist of nodes numbered
with integers between 0 (inclusive) and the total number of nodes (exclusive),
as passed in the function argument.

Time Complexity:
- O(n log n) per call to build(), where n is the number of nodes.
- O(log n) per call to lca().

Space Complexity:
- O(n log n) to store the sparse table, where n is the number of nodes.
- O(n) auxiliary stack space for build().
- O(1) auxiliary for lca().

*/

#include <vector>

const int MAXN = 1000;
std::vector<int> adj[MAXN], dp[MAXN];
int len, timer, tin[MAXN], tout[MAXN];

void dfs(int u, int p) {
  tin[u] = timer++;
  dp[u][0] = p;
  for (int i = 1; i < len; i++) {
    dp[u][i] = dp[dp[u][i - 1]][i - 1];
  }
  for (int j = 0; j < (int)adj[u].size(); j++) {
    int v = adj[u][j];
    if (v != p) {
      dfs(v, u);
    }
  }
  tout[u] = timer++;
}

void build(int nodes, int root = 0) {
  len = 1;
  while ((1 << len) <= nodes) {
    len++;
  }
  for (int i = 0; i < nodes; i++) {
    dp[i].resize(len);
  }
  timer = 0;
  dfs(root, root);
}

bool is_ancestor(int parent, int child) {
  return (tin[parent] <= tin[child]) && (tout[child] <= tout[parent]);
}

int lca(int u, int v) {
  if (is_ancestor(u, v)) {
    return u;
  }
  if (is_ancestor(v, u)) {
    return v;
  }
  for (int i = len - 1; i >= 0; i--) {
    if (!is_ancestor(dp[u][i], v)) {
      u = dp[u][i];
    }
  }
  return dp[u][0];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[3].push_back(1);
  adj[1].push_back(3);
  adj[0].push_back(4);
  adj[4].push_back(0);
  build(5, 0);
  assert(lca(3, 2) == 1);
  assert(lca(2, 4) == 0);
  return 0;
}
