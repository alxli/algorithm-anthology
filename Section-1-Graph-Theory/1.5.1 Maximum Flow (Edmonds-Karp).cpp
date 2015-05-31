/*

1.5.1 - Maximum Flow (Edmonds-Karp Algorithm)

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized.

The flow capacities are stored in an adjacency matrix. The edges
themselves are stored in an adjacency list to optimize the BFS.
The function edmonds_karp() takes in three parameters: the number
of nodes, the source node, and the sink node. Nodes are zero-based.
That is, values in adj[][] and flow[][] must describe a graph with
nodes labeled in the range [0..nodes-1].

Complexity: O(min(V*E^2, E*|F|)), where V is the number of vertices,
E is the number of edges, and F is the max flow. This improves the
original Ford-Fulkerson algorithm, which runs in O(E*|F|). As the 
Edmonds-Karp algorithm is also bounded by O(E*|F|), it is guaranteed
to be at least as fast as Ford-Fulkerson. For an even faster algorithm,
see Dinic’s algorithm in the next section, which runs in O(V^2*E).

Comparison with Ford-Fulkerson Algorithm:
The Ford-Fulkerson algorithm is only optimal on graphs with integer
capacities; there exists certain real-valued inputs for which it
will never terminate. The Edmonds-Karp algorithm here also works
on real-valued capacities.

=~=~=~=~= Sample Input =~=~=~=~=
6 8
0 1 3
0 2 3
1 2 2
1 3 3
2 4 2
3 4 1
3 5 2
4 5 3
0 5

=~=~=~=~= Sample Output =~=~=~=~=
5

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAXN = 100, INF = 0x3F3F3F3F;
int cap[MAXN][MAXN];
vector<int> adj[MAXN];

int edmonds_karp(int nodes, int source, int sink) {
  int max_flow = 0, a, b, best[nodes], pred[nodes];
  vector<bool> vis(nodes);
  for (int i = 0; i < nodes; i++) best[i] = 0;
  while (true) {
    for (int i = 0; i < nodes; i++) vis[i] = false;
    vis[source] = true;
    best[source] = INF;
    pred[sink] = -1;
    queue<int> q;
    for (q.push(source); !q.empty(); q.pop()) {
      a = q.front();
      if (a == sink) break;
      for (int j = 0; j < adj[a].size(); j++) {
        b = adj[a][j];
        if (!vis[b] && cap[a][b] > 0) {
          vis[b] = true;
          pred[b] = a;
          best[b] = min(best[a], cap[a][b]);
          q.push(b);
        }
      }
    }
    if (pred[sink] == -1) break;
    for (int i = sink; i != source; i = pred[i]) {
      cap[pred[i]][i] -= best[sink];
      cap[i][pred[i]] += best[sink];
    }
    max_flow += best[sink];
  }
  return max_flow;
}

int main() {
  int nodes, edges, a, b, capacity, source, sink;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> capacity;
    adj[a].push_back(b);
    cap[a][b] = capacity;
  }
  cin >> source >> sink;
  cout << edmonds_karp(nodes, source, sink) << "\n";
  return 0;
}
