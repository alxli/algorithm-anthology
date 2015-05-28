/*

1.7.1 - Edmonds-Karp Algorithm

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized.

The flow capacities are stored in an adjacency matrix. The edges
themselves are stored in an adjacency list to optimize the BFS.
The function edmonds_karp() takes in three parameters: the number
of nodes, the source node, and the sink node. Nodes are zero-based.
That is, values in adj[][] and flow[][] must describe a graph with
nodes labeled in the range [0..nodes-1].

Complexity: O(V*E^2), where V is the number of vertices
and E is the number of edges. This improves the original
Ford-Fulkerson algorithm, which has a complexity of O(E*|F|),
where F is the max-flow of the graph.

Despites its worst case complexity of O(V*E^2), this algorithm
is also bounded by O(E*|F|), so it is guaranteed to be at least as
fast as the Ford-Fulkerson algorithm. For an even faster algorithm,
see Dinic’s algorithm (section 1.7.2), which runs in O(V^2*E).

Comparison with Ford-Fulkerson Algorithm:
The Ford-Fulkerson algorithm is only optimal on graphs with integer
capacities; there exists certain real-valued inputs for which it
will never terminate. The Edmonds-Karp algorithm here also works
on real-valued capacities.

=~=~=~=~= Sample Input =~=~=~=~=
6 8
1 2 3
1 3 3
2 3 2
2 4 3
3 5 2
4 5 1
4 6 2
5 6 3
1 6

=~=~=~=~= Sample Output =~=~=~=~=
5

*/

#include <iostream>
#include <queue>
#include <vector>
using namespace std;

const int MAX_N = 1000, INF = 1 << 28;
int cap[MAXN][MAXN];
vector<int> adj[MAXN];

int edmonds_karp(int nodes, int source, int sink) {
  int max_flow = 0, a, b, best[nodes], pred[nodes];
  bool visit[nodes];
  for (int i = 0; i < nodes; i++) best[i] = 0;
  while (true) {
    for (int i = 0; i < nodes; i++) visit[i] = false;
    visit[source] = true;
    best[source] = INF;
    pred[sink] = -1;
    queue<int> q;
    for (q.push(source); !q.empty(); q.pop()) {
      a = q.front();
      if (a == sink) break;
      for (int j = 0; j < adj[a].size(); j++) {
        b = adj[a][j];
        if (!visit[b] && cap[a][b] > 0) {
          visit[b] = true;
          pred[b] = a;
          if (best[a] < cap[a][b])
            best[b] = best[a];
          else
            best[b] = cap[a][b];
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
    a--; b--;
    adj[a].push_back(b);
    cap[a][b] = capacity;
  }
  cin >> source >> sink;
  cout << edmonds_karp(nodes, --source, --sink);
  return 0;
}
