/*

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized. Note
that in this implementation, the adjacency list adj[] will
be modified by the function dinic() after it's been called.

Complexity: O(V^2*E) on the number of vertices and edges.

Comparison with Edmonds-Karp Algorithm:
Dinic's is similar to the Edmonds-Karp algorithm in that it
uses the shortest augmenting path. The introduction of the
concepts of the level graph and blocking flow enable Dinic's
algorithm to achieve its better performance. Hence, Dinic's
algorithm is also called Dinic's blocking flow algorithm.

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

#include <algorithm> /* std::fill(), std::min() */
#include <iostream>
#include <vector>
using namespace std;

struct edge { int to, rev, cap, f; };

const int MAXN = 100, INF = 0x3f3f3f3f;
int dist[MAXN], ptr[MAXN];
vector<edge> adj[MAXN];

void add_edge(int s, int t, int cap) {
  adj[s].push_back((edge){t, (int)adj[t].size(), cap, 0});
  adj[t].push_back((edge){s, (int)adj[s].size() - 1, 0, 0});
}

bool dinic_bfs(int nodes, int source, int sink) {
  fill(dist, dist + nodes, -1);
  dist[source] = 0;
  int q[nodes], qh = 0, qt = 0;
  q[qt++] = source;
  while (qh < qt) {
    int u = q[qh++];
    for (int j = 0; j < (int)adj[u].size(); j++) {
      edge & e = adj[u][j];
      if (dist[e.to] < 0 && e.f < e.cap) {
        dist[e.to] = dist[u] + 1;
        q[qt++] = e.to;
      }
    }
  }
  return dist[sink] >= 0;
}

int dinic_dfs(int u, int f, int sink) {
  if (u == sink) return f;
  for (; ptr[u] < (int)adj[u].size(); ptr[u]++) {
    edge &e = adj[u][ptr[u]];
    if (dist[e.to] == dist[u] + 1 && e.f < e.cap) {
      int df = dinic_dfs(e.to, min(f, e.cap - e.f), sink);
      if (df > 0) {
        e.f += df;
        adj[e.to][e.rev].f -= df;
        return df;
      }
    }
  }
  return 0;
}

int dinic(int nodes, int source, int sink) {
  int max_flow = 0, delta;
  while (dinic_bfs(nodes, source, sink)) {
    fill(ptr, ptr + nodes, 0);
    while ((delta = dinic_dfs(source, INF, sink)) != 0)
      max_flow += delta;
  }
  return max_flow;
}

int main() {
  int nodes, edges, u, v, capacity, source, sink;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v >> capacity;
    add_edge(u, v, capacity);
  }
  cin >> source >> sink;
  cout << dinic(nodes, source, sink) << "\n";
  return 0;
}
