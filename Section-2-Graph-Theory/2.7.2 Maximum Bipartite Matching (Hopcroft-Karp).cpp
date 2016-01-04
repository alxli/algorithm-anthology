/*

Description: Given two sets of vertices A = {0, 1, ..., n1}
and B = {0, 1, ..., n2} as well as a set of edges E mapping
nodes from set A to set B, determine the largest possible
subset of E such that no pair of edges in the subset share
a common vertex. Precondition: n2 >= n1.

Complexity: O(E sqrt V) on the number of edges and vertices.

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
int match[MAXN], dist[MAXN];
vector<bool> used(MAXN), vis(MAXN);
vector<int> adj[MAXN];

void bfs(int n1, int n2) {
  fill(dist, dist + n1, -1);
  int q[n2], qb = 0;
  for (int u = 0; u < n1; ++u) {
    if (!used[u]) {
      q[qb++] = u;
      dist[u] = 0;
    }
  }
  for (int i = 0; i < qb; i++) {
    int u = q[i];
    for (int j = 0; j < (int)adj[u].size(); j++) {
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
  for (int j = 0; j < (int)adj[u].size(); j++) {
    int v = match[adj[u][j]];
    if (v < 0 || (!vis[v] && dist[v] == dist[u] + 1 && dfs(v))) {
      match[adj[u][j]] = u;
      used[u] = true;
      return true;
    }
  }
  return false;
}

int hopcroft_karp(int n1, int n2) {
  fill(match, match + n2, -1);
  fill(used.begin(), used.end(), false);
  int res = 0;
  for (;;) {
    bfs(n1, n2);
    fill(vis.begin(), vis.end(), false);
    int f = 0;
    for (int u = 0; u < n1; ++u)
      if (!used[u] && dfs(u)) f++;
    if (!f) return res;
    res += f;
  }
  return res;
}

int main() {
  int n1, n2, edges, u, v;
  cin >> n1 >> n2 >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  cout << "Matched " << hopcroft_karp(n1, n2);
  cout << " pair(s). Matchings are:\n";
  for (int i = 0; i < n2; i++) {
    if (match[i] == -1) continue;
    cout << match[i] << " " << i << "\n";
  }
  return 0;
}
