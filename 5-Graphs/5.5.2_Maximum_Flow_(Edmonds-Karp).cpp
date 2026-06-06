/*

Given a flow network with integer capacities, find the maximum flow from a given source node to a
given sink node. The flow along each edge may not exceed its capacity, and flow is conserved at
every node other than the source and sink. `edmonds_karp()` applies to a global adjacency list
`adj` that will be modified by the function call.

The Edmonds-Karp algorithm will also support real-valued flow capacities. As such, this
implementation will work as intended upon changing the appropriate variables to doubles.

Time Complexity:
- O(min(n*m^2, m*f)) per call to `edmonds_karp()`, where $n$ is the number of nodes, $m$ is the
  number of edges, and $f$ is the maximum flow.

Space Complexity:
- O(max(n, m)) for storage of the flow network, where $n$ is the number of nodes and $m$ is the
  number of edges.

*/

#include <algorithm>
#include <climits>
#include <queue>
#include <vector>

struct edge {
  int u, v, rev, cap, f;
};

const int INF = INT_MAX / 2;
std::vector<std::vector<edge>> adj;

void add_edge(int u, int v, int cap) {
  adj[u].push_back(edge{u, v, static_cast<int>(adj[v].size()), cap, 0});
  adj[v].push_back(edge{v, u, static_cast<int>(adj[u].size()) - 1, 0, 0});
}

long long edmonds_karp(int source, int sink) {
  int nodes = adj.size();
  long long max_flow = 0;
  for (;;) {
    std::vector<edge *> pred(nodes, nullptr);
    std::queue<int> q;
    q.push(source);
    while (!q.empty() && !pred[sink]) {
      int u = q.front();
      q.pop();
      for (edge &e : adj[u]) {
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
  int nodes = 6;
  adj.resize(nodes);
  add_edge(0, 1, 3);
  add_edge(0, 2, 3);
  add_edge(1, 2, 2);
  add_edge(1, 3, 3);
  add_edge(2, 4, 2);
  add_edge(3, 4, 1);
  add_edge(3, 5, 2);
  add_edge(4, 5, 3);
  assert(edmonds_karp(0, 5) == 5);
  return 0;
}
