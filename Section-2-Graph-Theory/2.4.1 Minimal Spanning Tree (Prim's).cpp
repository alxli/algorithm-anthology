/*

Given a connected, undirected, weighted graph with possibly negative weights,
its minimum spanning tree is a subgraph which is a tree that connects all nodes
with a subset of its edges such that their total weight is minimized. prim()
applies to a global, pre-populated adjacency list adj[] which must only consist
of nodes numbered with integers between 0 (inclusive) and the total number of
nodes (exclusive), as passed in the function argument. If the input graph is not
connected, then this implementation will find the minimum spanning forest.

Since std::priority_queue is by default a max-heap, we simulate a min-heap by
negating node distances before pushing them and negating them again after
popping them. To modify this implementation to find the maximum spanning tree,
the two negation steps can be skipped to prioritize the max edges.

Time Complexity:
- O(m log n) per call to prim(), where m is the number of edges and n is the
  number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n the number of nodes and m is
  the number of edges.
- O(n) auxiliary heap space for prim().

*/

#include <queue>
#include <utility>
#include <vector>

const int MAXN = 100;
std::vector<std::pair<int, int> > adj[MAXN], mst;

int prim(int nodes) {
  mst.clear();
  std::vector<bool> visit(nodes);
  int total_dist = 0;
  for (int i = 0; i < nodes; i++) {
    if (visit[i]) {
      continue;
    }
    visit[i] = true;
    std::priority_queue<std::pair<int, std::pair<int, int> > > pq;
    for (int j = 0; j < (int)adj[i].size(); j++) {
      pq.push(std::make_pair(-adj[i][j].second,
                             std::make_pair(i, adj[i][j].first)));
    }
    while (!pq.empty()) {
      int u = pq.top().second.first;
      int v = pq.top().second.second;
      int w = -pq.top().first;
      pq.pop();
      if (visit[u] && !visit[v]) {
        visit[v] = true;
        if (v != i) {
          mst.push_back(std::make_pair(u, v));
          total_dist += w;
        }
        for (int j = 0; j < (int)adj[v].size(); j++) {
          pq.push(std::make_pair(-adj[v][j].second,
                                 std::make_pair(v, adj[v][j].first)));
        }
      }
    }
  }
  return total_dist;
}

/*** Example Usage and Output:

Total distance: 13
0 <-> 2
0 <-> 1
3 <-> 4
4 <-> 5
5 <-> 6

***/

#include <iostream>
using namespace std;

void add_edge(int u, int v, int w) {
  adj[u].push_back(make_pair(v, w));
  adj[v].push_back(make_pair(u, w));
}

int main() {
  add_edge(0, 1, 4);
  add_edge(1, 2, 6);
  add_edge(2, 0, 3);
  add_edge(3, 4, 1);
  add_edge(4, 5, 2);
  add_edge(5, 6, 3);
  add_edge(6, 4, 4);
  cout << "Total distance: " << prim(7) << endl;
  for (int i = 0; i < (int)mst.size(); i++) {
    cout << mst[i].first << " <-> " << mst[i].second << endl;
  }
  return 0;
}
