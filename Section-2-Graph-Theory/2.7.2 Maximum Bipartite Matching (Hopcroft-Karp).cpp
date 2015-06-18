/*

2.7.2 - Maximum Bipartite Matching (Hopcroft-Karp Algorithm)

Description: Given two sets of vertices A = {0, 1, ..., n1}
and B = {0, 1, ..., n2} as well as a set of edges E mapping
nodes from set A to set B, determine the largest possible
subset of E that do not contain common vertices.

Complexity: O(E*sqrt(V)) on the number of edges and vertices.

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

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int n1, n2, edges, a, b;
int match[MAXN], dist[MAXN], q[MAXN];
vector<bool> used(MAXN), vis(MAXN);
vector<int> adj[MAXN];

void bfs() {
  fill(dist, dist + n1, -1);
  int qb = 0;
  for (int u = 0; u < n1; ++u) {
    if (!used[u]) {
      q[qb++] = u;
      dist[u] = 0;
    }
  }
  for (int i = 0; i < qb; i++) {
    int u = q[i];
    for (int j = 0; j < adj[u].size(); j++) {
      int v = match[adj[u][j]];
      if (v >= 0 && dist[v] < 0) {
        dist[v] = dist[u] + 1;
        q[qb++] = v;
      }
    }
  }
}

bool dfs(int u) {
  vis[u] = true;
  for (int j = 0; j < adj[u].size(); j++) {
    int v = match[adj[u][j]];
    if (v < 0 || !vis[v] && dist[v] == dist[u] + 1 && dfs(v)) {
      match[adj[u][j]] = u;
      used[u] = true;
      return true;
    }
  }
  return false;
}

int match_bipartite() {
  for (int i = 0; i < n2; i++) match[i] = -1;
  int res = 0;
  for (;;) {
    for (int i = 0; i < n1; i++) {
      dist[i] = -1;
      vis[i] = false;
    }
    bfs();
    int f = 0;
    for (int u = 0; u < n1; ++u)
      if (!used[u] && dfs(u)) f++;
    if (!f) return res;
    res += f;
  }
  return res;
}

int main() {
  cin >> n1 >> n2 >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cout << "Matched " << match_bipartite();
  cout << " pair(s). Matchings are:\n";
  for (int i = 0; i < n2; i++) {
    if (match[i] == -1) continue;
    cout << match[i] << " " << i << "\n";
  }
  return 0;
}
