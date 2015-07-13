/*

2.4.2 - Minimum Spanning Tree (Kruskal's Algorithm)

Description: Given an undirected graph, its minimum spanning
tree (MST) is a tree connecting all nodes with a subset of its
edges such that their total weight is minimized. If the graph
is not connected, Kruskal's algorithm will produce the minimum
spanning forest. The input graph is stored in an edge list.

Complexity: O(E log V) on the number of edges and vertices.

=~=~=~=~= Sample Input =~=~=~=~=
7 7
0 1 4
1 2 6
2 0 3
3 4 1
4 5 2
5 6 3
6 4 4

=~=~=~=~= Sample Output =~=~=~=~=
Total distance: 13
3<->4
4<->5
2<->0
5<->6
0<->1

Note: If you already have a disjoint set data structure,
then the middle section of the program can be replaced by:

disjoint_set_forest<int> dsf;
for (int i = 0; i < nodes; i++) dsf.make_set(i);
for (int i = 0; i < E.size(); i++) {
  a = E[i].second.first;
  b = E[i].second.second;
  if (!dsf.is_united(a, b)) {
    ...
    dsf.unite(a, b);
  }
}

*/

#include <algorithm> /* std::sort() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int root[MAXN];
vector<pair<int, pair<int, int> > > E;
vector<pair<int, int> > mst;

int find_root(int x) {
  if (root[x] != x)
    root[x] = find_root(root[x]);
  return root[x];
}

int kruskal(int nodes) {
  mst.clear();
  sort(E.begin(), E.end());
  int u, v, total_dist = 0;
  for (int i = 0; i < nodes; i++) root[i] = i;
  for (int i = 0; i < (int)E.size(); i++) {
    u = find_root(E[i].second.first);
    v = find_root(E[i].second.second);
    if (u != v) {
      mst.push_back(E[i].second);
      total_dist += E[i].first;
      root[u] = root[v];
    }
  }
  return total_dist;
}

int main() {
  int nodes, edges, u, v, w;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> w;
    E.push_back(make_pair(w, make_pair(u, v)));
  }
  cout << "Total distance: " << kruskal(nodes) << "\n";
  for (int i = 0; i < (int)mst.size(); i++)
    cout << mst[i].first << "<->" << mst[i].second << "\n";
  return 0;
}
