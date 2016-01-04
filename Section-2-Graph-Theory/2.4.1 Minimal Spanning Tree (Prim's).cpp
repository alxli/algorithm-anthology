/*

Description: Given an undirected graph, its minimum spanning
tree (MST) is a tree connecting all nodes with a subset of its
edges such that their total weight is minimized. Prim's algorithm
greedily selects edges from a priority queue, and is similar to
Dijkstra's algorithm, where instead of processing nodes, we
process individual edges. If the graph is not connected, Prim's
algorithm will produce the minimum spanning forest. The input
graph is stored in an adjacency list.

Note that the concept of the minimum spanning tree makes Prim's
algorithm work with negative weights. In fact, a big positive
constant added to all of the edge weights of the graph will not
change the resulting spanning tree.

Implementation Notes: Similar to the implementation of Dijkstra's
algorithm in the previous section, weights are negated before they
are added to the priority queue (and negated once again when they
are retrieved). To find the maximum spanning tree, simply skip the
two negation steps and the max weighted edges will be prioritized.

Complexity: This version uses an adjacency list and priority queue
(internally a binary heap) and has a complexity of O((E+V) log V) =
O(E log V). The priority queue and adjacency list improves the
simplest O(V^2) version of the algorithm, which uses looping and
an adjacency matrix. If the priority queue is implemented as a more
sophisticated Fibonacci heap, the complexity becomes O(E + V log V).

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
0<->2
0<->1
3<->4
4<->5
5<->6

*/

#include <algorithm> /* std::fill() */
#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100;
vector<pair<int, int> > adj[MAXN], mst;

int prim(int nodes) {
  mst.clear();
  vector<bool> vis(nodes);
  int u, v, w, total_dist = 0;
  for (int i = 0; i < nodes; i++) {
    if (vis[i]) continue;
    vis[i] = true;
    priority_queue<pair<int, pair<int, int> > > pq;
    for (int j = 0; j < (int)adj[i].size(); j++)
      pq.push(make_pair(-adj[i][j].second,
                make_pair(i, adj[i][j].first)));
    while (!pq.empty()) {
      w = -pq.top().first;
      u = pq.top().second.first;
      v = pq.top().second.second;
      pq.pop();
      if (vis[u] && !vis[v]) {
        vis[v] = true;
        if (v != i) {
          mst.push_back(make_pair(u, v));
          total_dist += w;
        }
        for (int j = 0; j < (int)adj[v].size(); j++)
          pq.push(make_pair(-adj[v][j].second,
                    make_pair(v, adj[v][j].first)));
      }
    }
  }
  return total_dist;
}

int main() {
  int nodes, edges, u, v, w;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> w;
    adj[u].push_back(make_pair(v, w));
    adj[v].push_back(make_pair(u, w));
  }
  cout << "Total distance: " << prim(nodes) << "\n";
  for (int i = 0; i < (int)mst.size(); i++)
    cout << mst[i].first << "<->" << mst[i].second << "\n";
  return 0;
}
