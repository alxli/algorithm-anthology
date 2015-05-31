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

const int MAXN1 = 100;
int n1, n2, edges, a, b;
vector<int> adj[MAXN1];

bool find_path(int u1, int res[], vector<bool> & vis) {
  vis[u1] = true;
  for (int j = 0, u2, v; j < adj[u1].size(); j++) {
    v = adj[u1][j];
    u2 = res[v];
    if (u2 == -1 || !vis[u2] && find_path(u2, res, vis)) {
      res[v] = u1;
      return true;
    }
  }
  return false;
}

int match_bipartite(int res[]) {
  for (int i = 0; i < n2; i++) res[i] = -1;
  int matches = 0;
  for (int i = 0; i < n1; i++) {
    vector<bool> vis(n1);
    if (find_path(i, res, vis)) matches++;
  }
  return matches;
}

int main() {
  cin >> n1 >> n2 >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b;
    adj[a].push_back(b);
  }
  int res[n2];
  cout << "Matched " << match_bipartite(res);
  cout << " pairs. Matchings are:\n";
  for (int i = 0; i < n2; i++) {
  	if (res[i] == -1) continue;
    cout << res[i] << " " << i << "\n";
  }
  return 0;
}
