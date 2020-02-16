/*

Given a connected, undirected, weighted graph with possibly negative weights,
its minimum spanning tree is a subgraph which is a tree that connects all nodes
with a subset of its edges such that their total weight is minimized. kruskal()
applies to a global, pre-populated adjacency list adj[] which must only consist
of nodes numbered with integers between 0 (inclusive) and the total number of
nodes (exclusive), as passed in the function argument. If the input graph is not
connected, then this implementation will find the minimum spanning forest.

Time Complexity:
- O(m log n) per call to kruskal(), where m is the number of edges and n is the
  number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n the number of nodes and m is
  the number of edges
- O(n) auxiliary stack space for kruskal().

*/

#include <algorithm>
#include <utility>
#include <vector>

const int MAXN = 100;
std::vector<std::pair<int, std::pair<int, int> > > edges;
int root[MAXN];
std::vector<std::pair<int, int> > mst;

int find_root(int x) {
  if (root[x] != x) {
    root[x] = find_root(root[x]);
  }
  return root[x];
}

int kruskal(int nodes) {
  mst.clear();
  std::sort(edges.begin(), edges.end());
  int total_dist = 0;
  for (int i = 0; i < nodes; i++) {
    root[i] = i;
  }
  for (int i = 0; i < (int)edges.size(); i++) {
    int u = find_root(edges[i].second.first);
    int v = find_root(edges[i].second.second);
    if (u != v) {
      root[u] = root[v];
      mst.push_back(edges[i].second);
      total_dist += edges[i].first;
    }
  }
  return total_dist;
}

/*** Example Usage and Output:

Total distance: 13
3 <-> 4
4 <-> 5
2 <-> 0
5 <-> 6
0 <-> 1

***/

#include <iostream>
using namespace std;

void add_edge(int u, int v, int w) {
  edges.push_back(make_pair(w, make_pair(u, v)));
}

int main() {
  add_edge(0, 1, 4);
  add_edge(1, 2, 6);
  add_edge(2, 0, 3);
  add_edge(3, 4, 1);
  add_edge(4, 5, 2);
  add_edge(5, 6, 3);
  add_edge(6, 4, 4);
  cout << "Total distance: " << kruskal(7) << endl;
  for (int i = 0; i < (int)mst.size(); i++) {
    cout << mst[i].first << " <-> " << mst[i].second << endl;
  }
  return 0;
}
