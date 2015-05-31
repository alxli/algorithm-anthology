/*

1.7.2 - Maximum Flow (Dinic's Blocking Flow Algorithm)

Description: Given a flow network, find a flow from a single
source node to a single sink node that is maximized.

Complexity: O(V^2*E) on the number of vertices and edges.

Comparison with Edmonds-Karp Algorithm:
Dinic’s algorithm is similar to the Edmonds-Karp algorithm in that
it uses the shortest augmenting path. The introduction of the
concepts of the level graph and blocking flow enable Dinic's
algorithm to achieve its better performance. Hence, Dinic’s
algorithm is also called Dinic’s blocking flow algorithm.

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
#include <vector>
using namespace std;

struct edge { int to, rev, f; };

const int MAXN = 100, INF = 0x3F3F3F3F;
int nodes, source, sink;
vector<edge> adj[MAXN];

void add_edge(int s, int t, int cap) {
  adj[s].push_back((edge){t, (int)adj[t].size(), cap});
  adj[t].push_back((edge){s, (int)adj[s].size()-1, 0});
}

int dist[MAXN], queue[MAXN], work[MAXN];

bool dinic_bfs() {
  for (int i = 0; i < nodes; i++) dist[i] = -1;
  dist[source] = 0;
  int qh = 0, qt = 0;               /* queue<int> q; */
  queue[qt++] = source;           /* q.push(source); */
  while (qh < qt) {          /* while (!q.empty()) { */
    int u = queue[qh++];  /* u = q.front(); q.pop(); */
    for (int j = 0; j < adj[u].size(); j++) {
      edge &e = adj[u][j];
      int v = e.to;
      if (dist[v] < 0 && e.f) {
        dist[v] = dist[u] + 1;
        queue[qt++] = v;                /* q.push(v) */
      }
    }
  }
  return dist[sink] >= 0;
}

int dinic_dfs(int u, int f) {
  if (u == sink) return f;
  for (int &i = work[u]; i < adj[u].size(); i++) {
    edge &e = adj[u][i];
    if (!e.f) continue;
    int v = e.to, df;
    if (dist[v] == dist[u] + 1 &&
        (df = dinic_dfs(v, min(f, e.f))) > 0) {
      e.f -= df;
      adj[v][e.rev].f += df;
      return df;
    }
  }
  return 0;
}

int dinic() {
  int max_flow = 0;
  while (dinic_bfs()) {
    for (int i = 0; i < nodes; i++) work[i] = 0;
    while (int delta = dinic_dfs(source, INF))
      max_flow += delta;
  }
  return max_flow;
}

int main() {
  int edges, a, b, capacity;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b >> capacity;
    add_edge(a, b, capacity);
  }
  cin >> source >> sink;
  cout << dinic() << "\n";
  return 0;
}
