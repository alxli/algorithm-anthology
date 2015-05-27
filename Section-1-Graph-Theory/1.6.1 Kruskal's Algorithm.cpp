/*

1.6.1 - Kruskal's Algorithm (Minimum Spanning Tree)

Description: Given an undirected graph, its minimum spanning tree (MST)
is a tree connecting all nodes with a subset of its edges such that their
total weight is minimized. The input graph is stored in an edge list.

Complexity: O(E*log(V)), where E and V are the number of edges and vertices.

=~=~=~=~= Sample Input =~=~=~=~=
7 7
1 2 4
2 3 6
3 1 3
4 5 1
5 6 2
6 7 3
7 5 4

=~=~=~=~= Sample Output =~=~=~=~=
4<->5
5<->6
3<->1
6<->7
1<->2
Total distance: 13

Note: If you already have a disjoint set data structure,
then the middle section of the program can be replaced by:

disjoint_set_forest<int> F;
for (int i = 1; i <= nodes; i++) F.make_set(i);
for (int i = 0; i < E.size(); i++) {
  a = E[i].second.first;
  b = E[i].second.second;
  if (!F.is_united(a, b)) {
    ...
    F.unite(a, b);
  }
}

*/

#include <algorithm> /* std::sort() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, weight, root[MAXN+1];
vector< pair<int, pair<int, int> > > E;

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
  int totalDistance = 0;
  for (int i = 1; i <= nodes; i++) root[i] = i;
  for (int i = 0; i < E.size(); i++) {
    a = find_root(E[i].second.first);
    b = find_root(E[i].second.second);
    if (a != b) {
      cout << E[i].second.first << "<->";
      cout << E[i].second.second << "\n";
      totalDistance += E[i].first;
      root[a] = root[b];
    }
  }
  cout << "Total distance: " << totalDistance << endl;
  return 0;
} 
