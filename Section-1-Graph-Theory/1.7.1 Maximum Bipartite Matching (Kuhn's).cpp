/*

1.7.1 - Maximum Bipartite Matching (Kuhn's Algorithm)

Description: Given two sets of vertices A = {0, 1, ..., n1}
and B = {0, 1, ..., n2} as well as a set of edges E mapping
nodes from set A to set B, determine the largest possible
subset of E that do not contain common vertices.

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

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int n1, n2, edges, a, b, match[MAXN];
vector<bool> vis(MAXN);
vector<int> adj[MAXN];

bool find_path(int u) {
  vis[u] = true;
  for (int j = 0; j < adj[u].size(); j++) {
    int v = match[adj[u][j]];
    if (v == -1 || !vis[v] && find_path(v)) {
      match[adj[u][j]] = u;
      return true;
    }
  }
  return false;
}

int match_bipartite() {
  for (int i = 0; i < n2; i++) match[i] = -1;
  int matches = 0;
  for (int i = 0; i < n1; i++) {
    for (int j = 0; j < n1; j++) vis[j] = 0;
    if (find_path(i)) matches++;
  }
  return matches;
}

int main() {
  cin >> n1 >> n2 >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  cout << "Matched " << match_bipartite();
  cout << " pairs. Matchings are:\n";
  for (int i = 0; i < n2; i++) {
    if (match[i] == -1) continue;
    cout << match[i] << " " << i << "\n";
  }
  return 0;
}
