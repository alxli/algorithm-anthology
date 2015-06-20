/*

2.1.5 - Unweighted Tree Centers, Centroid, and Diameter

The following applies to unweighted, undirected trees only.

find_centers(): Returns 1 or 2 tree centers. The center
(or Jordan center) of a graph is the set of all vertices of
minimum eccentricity, that is, the set of all vertices A
where the max distance d(A,B) to other vertices B is minimal.

find_centroid(): Returns a vertex where all of its subtrees
have size <= N/2, where N is the number of nodes in the tree.

diameter(): The diameter of a tree is the greatest distance
d(A,B) between any two of the nodes in the tree.

Complexity: All three functions are O(V) on the number of
vertices in the tree.

=~=~=~=~= Sample Input =~=~=~=~=
6
0 1
1 2
1 4
3 4
4 5

=~=~=~=~= Sample Output =~=~=~=~=
Center(s): 1 4
Centroid: 4
Diameter: 3

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
vector<int> adj[MAXN];

vector<int> find_centers(int n) {
  vector<int> leaves, degree(n);
  for (int i = 0; i < n; i++) {
    degree[i] = adj[i].size();
    if (degree[i] <= 1) leaves.push_back(i);
  }
  int removed = leaves.size();
  while (removed < n) {
    vector<int> nleaves;
    for (int i = 0; i < leaves.size(); i++) {
      int u = leaves[i];
      for (int j = 0; j < adj[u].size(); j++) {
        int v = adj[u][j];
        if (--degree[v] == 1)
          nleaves.push_back(v);
      }
    }
    leaves = nleaves;
    removed += leaves.size();
  }
  return leaves;
}

int find_centroid(int n, int u = 0, int p = -1) {
  int cnt = 1, v;
  bool good_center = true;
  for (int j = 0; j < adj[u].size(); j++) {
    if ((v = adj[u][j]) == p) continue;
    int res = find_centroid(n, v, u);
    if (res >= 0) return res;
    int size = -res;
    good_center &= (size <= n / 2);
    cnt += size;
  }
  good_center &= (n - cnt <= n / 2);
  return good_center ? u : -cnt;
}

pair<int, int> dfs(int u, int p, int depth) {
  pair<int, int> res = make_pair(depth, u);
  for (int j = 0; j < adj[u].size(); j++)
    if (adj[u][j] != p)
      res = max(res, dfs(adj[u][j], u, depth + 1));
  return res;
}

int diameter() {
  int furthest_vertex = dfs(0, -1, 0).second;
  return dfs(furthest_vertex, -1, 0).first;
}

int main() {
  int nodes, u, v;
  cin >> nodes;
  for (int i = 0; i < nodes - 1; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
    adj[v].push_back(u);
  }
  vector<int> centers = find_centers(nodes);
  cout << "Center(s):";
  for (int i = 0; i < centers.size(); i++)
    cout << " " << centers[i];
  cout << "\nCentroid: " << find_centroid(nodes);
  cout << "\nDiameter: " << diameter() << "\n";
  return 0;
}
