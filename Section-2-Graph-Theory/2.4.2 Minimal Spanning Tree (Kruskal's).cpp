/*

2.4.2 - Minimum Spanning Tree (Kruskal's Algorithm)

Description: Given an undirected graph, its minimum spanning tree (MST)
is a tree connecting all nodes with a subset of its edges such that their
total weight is minimized. The input graph is stored in an edge list.
If the graph is not connected, Kruskal's algorithm will produce the
minimum spanning forest (refer to the sample).

Complexity: O(E*log(V)) on the number of edges and vertices respectively.

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
3<->4
4<->5
2<->0
5<->6
0<->1
Total distance: 13

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
int nodes, edges, a, b, weight, root[MAXN];
vector<pair<int, pair<int, int> > > E;

int find_root(int x) {
  if (root[x] != x) root[x] = find_root(root[x]);
  return root[x];
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> weight;
    E.push_back(make_pair(weight, make_pair(a, b)));
  }
  sort(E.begin(), E.end());
  int total_dist = 0;
  for (int i = 0; i < nodes; i++) root[i] = i;
  for (int i = 0; i < E.size(); i++) {
    a = find_root(E[i].second.first);
    b = find_root(E[i].second.second);
    if (a != b) {
      cout << E[i].second.first << "<->";
      cout << E[i].second.second << "\n";
      total_dist += E[i].first;
      root[a] = root[b];
    }
  }
  cout << "Total distance: " << total_dist << "\n";
  return 0;
}
