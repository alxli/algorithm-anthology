/*

1.5.1 - Maximum Flow (Edmonds-Karp Algorithm)

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized. Note
that in this implementation, the adjacency list adj[] will
be modified by the function edmonds_karp() after it's been called.

Complexity: O(min(V*E^2, E*|F|)), where V is the number of vertices,
E is the number of edges, and F is the max flow. This improves the
original Ford-Fulkerson algorithm, which runs in O(E*|F|). As the
Edmonds-Karp algorithm is also bounded by O(E*|F|), it is guaranteed
to be at least as fast as Ford-Fulkerson. For an even faster algorithm,
see Dinic's algorithm in the next section, which runs in O(V^2*E).

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

#include <algorithm>
#include <iostream>
#include <vector>
using namespace std;

struct edge { int s, t, rev, cap, f; };

const int MAXN = 100, INF = 0x3f3f3f3f;
vector<edge> adj[MAXN];

void add_edge(int s, int t, int cap) {
  adj[s].push_back((edge){s, t, adj[t].size(), cap, 0});
  adj[t].push_back((edge){t, s, adj[s].size() - 1, 0, 0});
}

int edmonds_karp(int nodes, int source, int sink) {
  static int q[MAXN];
  int max_flow = 0;
  for (;;) {
    int qt = 0;
    q[qt++] = source;
    edge * pred[nodes];
    for (int i = 0; i < nodes; i++) pred[i] = 0;
    for (int qh = 0; qh < qt && !pred[sink]; qh++) {
      int u = q[qh];
      for (int j = 0; j < adj[u].size(); j++) {
        edge * e = &adj[u][j];
        if (!pred[e->t] && e->cap > e->f) {
          pred[e->t] = e;
          q[qt++] = e->t;
        }
      }
    }
    if (!pred[sink]) break;
    int df = INF;
    for (int u = sink; u != source; u = pred[u]->s)
      df = min(df, pred[u]->cap - pred[u]->f);
    for (int u = sink; u != source; u = pred[u]->s) {
      pred[u]->f += df;
      adj[pred[u]->t][pred[u]->rev].f -= df;
    }
    max_flow += df;
  }
  return max_flow;
}

int main() {
  int nodes, edges, a, b, capacity, source, sink;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> capacity;
    add_edge(a, b, capacity);
  }
  cin >> source >> sink;
  cout << edmonds_karp(nodes, source, sink) << "\n";
  return 0;
}
