/*

3.7.1 - Sparse Tables for Lowest Common Ancestor

Description: Given a rooted tree, the lowest common ancestor (LCA)
of two nodes v and w is the lowest (i.e. deepest) node that has
both v and w as descendants, where we define each node to be a
descendant of itself (so if v has a direct connection from w, w
is the lowest common ancestor). This problem can be solved using
sparse tables. A 2D table of size N by log N is computed using:

dp[i][j] = parent[i],               if j = 0
           dp[dp[i][j - 1]][i - 1], if j > 0

where dp[i][j] is the (2^j)-th ancestor of i.

Time Complexity: O(N log N) for build() and O(log N) for lca(),
where N is the number of nodes in the tree.

Space Complexity: O(N log N).

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 1000;
int len, timer, tin[MAXN], tout[MAXN];
vector<int> adj[MAXN], dp[MAXN];

void dfs(int u, int p) {
  tin[u] = timer++;
  dp[u][0] = p;
  for (int i = 1; i < len; i++)
    dp[u][i] = dp[dp[u][i - 1]][i - 1];
  for (int j = 0; j < adj[u].size(); j++)
    if (adj[u][j] != p) dfs(adj[u][j], u);
  tout[u] = timer++;
}

void build(int nodes, int root) {
  len = 1;
  while ((1 << len) <= nodes) len++;
  for (int i = 0; i < nodes; i++)
    dp[i] = vector<int>(len);
  timer = 0;
  dfs(root, root);
}

inline bool is_parent(int p, int c) {
  return tin[p] <= tin[c] && tout[c] <= tout[p];
}

int lca(int a, int b) {
  if (is_parent(a, b)) return a;
  if (is_parent(b, a)) return b;
  for (int i = len - 1; i >= 0; i--)
    if (!is_parent(dp[a][i], b))
      a = dp[a][i];
  return dp[a][0];
}

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
  cout << lca(3, 2) << "\n"; //1
  cout << lca(2, 4) << "\n"; //0
  return 0;
}
