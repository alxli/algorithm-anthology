/*

1.4.1 - Minimum Spanning Tree (Prim's Algorithm)

Description: Given an undirected graph, its minimum spanning tree (MST)
is a tree connecting all nodes with a subset of its edges such that their
total weight is minimized. The input graph is stored in an adjacency list.
Prim's algorithm greedily selects edges from a priority queue, and is
similar to Dijkstra's algorithm, where instead of processing nodes, we
process individual edges. Note that the concept of the minimum spanning
tree makes Prim's algorithm work with edge weights of an arbitrary sign.
In fact, a big positive constant added to all of the edge weights of the
graph will not change the resulting spanning tree.

Warning: The implementation below only works on connected graphs. In order
to make Prim's work on unconnected graphs to produce a minimum spanning
forest, you will have to modify the code to run Prim once per connected
component of the graph. Or, use Kruskal algorithm in the next section.

Implementation Notes: Similar to the implementation of Dijkstra's algorithm
in the previous section, weights are negated before they are added to the
priority queue (and negated once again when they are retrieved). To find
the maximum spanning tree, simply skip the two negation steps and the
highest weighted edges will be prioritized.

Complexity: This version uses an adjacency list and priority queue
(internally a binary heap) and has a complexity of O((E + V)*log(V)) =
O(E*log(V)). The priority queue and adjacency list improves the
simplest O(V^2) version of the algorithm, which uses looping and
an adjacency matrix. If the priority queue is implemented as a more
sophisticated Fibonacci heap, the complexity becomes O(E + V*log(V)).  

=~=~=~=~= Sample Input =~=~=~=~=
9 14
0 1 4
0 7 8
1 2 9
1 7 11
2 3 7
2 8 2
2 7 4
3 4 9
3 5 14
4 5 10
5 6 2
6 7 1
6 8 6
7 8 7

=~=~=~=~= Sample Output =~=~=~=~=
0<->1
0<->7
7<->6
6<->5
7<->2
2<->8
2<->3
3<->4
Total distance: 37

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100;
int nodes, edges, a, b, weight;
vector<bool> vis(MAXN);
vector<pair<int, int> > adj[MAXN];

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> weight;
    adj[a].push_back(make_pair(b, weight));
    adj[b].push_back(make_pair(a, weight));
  }
  int start = 0; //arbitrarily pick 0 as a start node
  vis[start] = true;
  priority_queue<pair<int, pair<int, int> > > pq;
  for (int j = 0; j < adj[start].size(); j++)
    pq.push(make_pair(-adj[start][j].second,
              make_pair(start, adj[start][j].first)));
  int mst_nodes = 1, total_dist = 0;
  while (mst_nodes < nodes) {
    if (pq.empty()) {
      cout << "Error: Graph is not connected!\n";
      return 0;
    }
    weight = -pq.top().first;
    a = pq.top().second.first;
    b = pq.top().second.second;
    pq.pop();
    if (vis[a] && !vis[b]) {
      vis[b] = true;
      mst_nodes++;
      if (b != start) cout << a << "<->" << b << "\n";
      total_dist += weight;
      for (int j = 0; j < adj[b].size(); j++)
        pq.push(make_pair(-adj[b][j].second,
                  make_pair(b, adj[b][j].first)));
    }
  }
  cout << "Total distance: " << total_dist << "\n";
  return 0;
}
