/*

Given a flow network with integer capacities, find the maximum flow from a given
source node to a given sink node. The flow of a given edge u -> v is defined as
the minimum of its capacity and the sum of the flows of all incoming edges of u.
edmonds_karp() applies to a global adjacency list adj[] that will be modified by
the function call.

The Edmonds-Karp algorithm will also support real-valued flow capacities. As
such, this implementation will work as intended upon changing the appropriate
variables to doubles.

Time Complexity:
- O(min(n*m^2, m*f)) per call to edmonds_karp(), where n is the number of nodes,
  m is the number of edges, and f is the maximum flow.

Space Complexity:
- O(max(n, m)) for storage of the flow network, where n is the number of nodes
  and m is the number of edges.

*/

#include <algorithm>
#include <queue>
#include <vector>

struct edge { int u, v, rev, cap, f; };

const int MAXN = 100, INF = 0x3f3f3f3f;
std::vector<edge> adj[MAXN];

void add_edge(int u, int v, int cap) {
  adj[u].push_back((edge){u, v, (int)adj[v].size(), cap, 0});
  adj[v].push_back((edge){v, u, (int)adj[u].size() - 1, 0, 0});
}

int edmonds_karp(int nodes, int source, int sink) {
  int max_flow = 0;
  for (;;) {
    std::vector<edge*> pred(nodes, (edge*)0);
    std::queue<int> q;
    q.push(source);
    while (!q.empty() && !pred[sink]) {
      int u = q.front();
      q.pop();
      for (int j = 0; j < (int)adj[u].size(); j++) {
        edge &e = adj[u][j];
        if (!pred[e.v] && e.cap > e.f) {
          pred[e.v] = &e;
          q.push(e.v);
        }
      }
    }
    if (!pred[sink]) {
      break;
    }
    int flow = INF;
    for (int u = sink; u != source; u = pred[u]->u) {
      flow = std::min(flow, pred[u]->cap - pred[u]->f);
    }
    for (int u = sink; u != source; u = pred[u]->u) {
      pred[u]->f += flow;
      adj[pred[u]->v][pred[u]->rev].f -= flow;
    }
    max_flow += flow;
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
  assert(edmonds_karp(6, 0, 5) == 5);
  return 0;
}
