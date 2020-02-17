/*

Given a flow network with integer capacities, find the maximum flow from a given
source node to a given sink node. The flow of a given edge u -> v is defined as
the minimum of its capacity and the sum of the flows of all incoming edges of u.
dinic() applies to a global adjacency list adj[] that will be modified by the
function call.

Dinic's algorithm will also support real-valued flow capacities. As such, this
implementation will work as intended upon changing the appropriate variables to
doubles.

Time Complexity:
- O(n^2*m) per call to dinic(), where n is the number of nodes and m is the
  number of edges.

Space Complexity:
- O(max(n, m)) for storage of the flow network, where n is the number of nodes
  and m is the number of edges.
- O(n) auxiliary stack and heap space for dinic().

*/

#include <algorithm>
#include <queue>
#include <vector>

struct edge { int v, rev, cap, f; };

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<edge> adj[MAXN];
int dist[MAXN], ptr[MAXN];

void add_edge(int u, int v, int cap) {
  adj[u].push_back((edge){v, (int)adj[v].size(), cap, 0});
  adj[v].push_back((edge){u, (int)adj[u].size() - 1, 0, 0});
}

bool dinic_bfs(int nodes, int source, int sink) {
  std::fill(dist, dist + nodes, -1);
  dist[source] = 0;
  std::queue<int> q;
  q.push(source);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int j = 0; j < (int)adj[u].size(); j++) {
      edge &e = adj[u][j];
      if (dist[e.v] < 0 && e.f < e.cap) {
        dist[e.v] = dist[u] + 1;
        q.push(e.v);
      }
    }
  }
  return dist[sink] >= 0;
}

int dinic_dfs(int u, int f, int sink) {
  if (u == sink) {
    return f;
  }
  for (; ptr[u] < (int)adj[u].size(); ptr[u]++) {
    edge &e = adj[u][ptr[u]];
    if (dist[e.v] == dist[u] + 1 && e.f < e.cap) {
      int flow = dinic_dfs(e.v, std::min(f, e.cap - e.f), sink);
      if (flow > 0) {
        e.f += flow;
        adj[e.v][e.rev].f -= flow;
        return flow;
      }
    }
  }
  return 0;
}

int dinic(int nodes, int source, int sink) {
  int flow, max_flow = 0;
  while (dinic_bfs(nodes, source, sink)) {
    std::fill(ptr, ptr + nodes, 0);
    while ((flow = dinic_dfs(source, INF, sink)) != 0) {
      max_flow += flow;
    }
  }
  return max_flow;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  add_edge(0, 1, 3);
  add_edge(0, 2, 3);
  add_edge(1, 2, 2);
  add_edge(1, 3, 3);
  add_edge(2, 4, 2);
  add_edge(3, 4, 1);
  add_edge(3, 5, 2);
  add_edge(4, 5, 3);
  assert(dinic(6, 0, 5) == 5);
  return 0;
}
