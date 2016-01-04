/*

Description: Given two sets of vertices A = {0, 1, ..., n1}
and B = {0, 1, ..., n2} as well as a set of edges E mapping
nodes from set A to set B, determine the largest possible
subset of E such that no pair of edges in the subset share
a common vertex. Precondition: n2 >= n1.

Complexity: O(V*E) on the number of vertices and edges.

=~=~=~=~= Sample Input =~=~=~=~=
3 4 6
0 1
1 0
1 1
1 2
2 2
2 3

=~=~=~=~= Sample Output =~=~=~=~=
Matched 3 pairs. Matchings are:
1 0
0 1
2 2

*/

#include <algorithm> /* std::fill() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int match[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

bool dfs(int u) {
  vis[u] = true;
  for (int j = 0; j < (int)adj[u].size(); j++) {
    int v = match[adj[u][j]];
    if (v == -1 || (!vis[v] && dfs(v))) {
      match[adj[u][j]] = u;
      return true;
    }
  }
  return false;
}

int kuhn(int n1, int n2) {
  fill(vis.begin(), vis.end(), false);
  fill(match, match + n2, -1);
  int matches = 0;
  for (int i = 0; i < n1; i++) {
    for (int j = 0; j < n1; j++) vis[j] = 0;
    if (dfs(i)) matches++;
  }
  return matches;
}

int main() {
  int n1, n2, edges, u, v;
  cin >> n1 >> n2 >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  cout << "Matched " << kuhn(n1, n2);
  cout << " pair(s). Matchings are:\n";
  for (int i = 0; i < n2; i++) {
    if (match[i] == -1) continue;
    cout << match[i] << " " << i << "\n";
  }
  return 0;
}
