/*

Given a flow network with integer capacities and edge costs, find a minimum-cost flow from a source
node to a sink node.

- `MinCostMaxFlow(nodes)` constructs an empty residual network whose nodes are numbered from 0 to
  `nodes - 1`.
- `init(nodes)` clears the residual network and resets it to contain `nodes` nodes.
- `add_edge(u, v, cap, cost)` adds a directed residual-network edge.
- `min_cost_flow(source, sink, target_flow)` sends up to `target_flow` units of flow and returns
  `(flow, cost)`. If the returned flow is smaller than `target_flow`, the network cannot carry the
  requested amount.

This implementation uses shortest augmenting paths with SPFA, so it supports negative edge costs as
long as the residual graph has no reachable negative-cost cycle. For dense or adversarial inputs, a
potential-based Dijkstra version is usually faster when reduced costs are nonnegative.

Time Complexity:
- O(n) per call to the constructor or `init(n)`, where $n$ is the number of nodes.
- O(1) per call to `add_edge()`.
- O(f*n*m) per call to `min_cost_flow()`, where $f$ is the amount of flow sent, $n$ is the number of
  nodes, and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the residual network and auxiliary arrays.

*/

#include <algorithm>
#include <climits>
#include <queue>
#include <utility>
#include <vector>

struct Edge {
  int v, rev, cap, cost;
  Edge(int v_, int rev_, int cap_, int cost_) : v(v_), rev(rev_), cap(cap_), cost(cost_) {}
};

struct FlowResult {
  int flow;
  long long cost;  // Total cost can exceed 32 bits even with int capacities and edge costs.
};

const long long MCMF_INF = LLONG_MAX / 4;

struct MinCostMaxFlow {
  std::vector<std::vector<Edge>> adj;

  MinCostMaxFlow(int nodes = 0) { init(nodes); }

  void init(int nodes) { adj.assign(nodes, std::vector<Edge>()); }

  void add_edge(int u, int v, int cap, int cost) {
    adj[u].emplace_back(v, static_cast<int>(adj[v].size()), cap, cost);
    adj[v].emplace_back(u, static_cast<int>(adj[u].size()) - 1, 0, -cost);
  }

  FlowResult min_cost_flow(int source, int sink, int target_flow) {
    int nodes = adj.size(), flow = 0;
    long long cost = 0;
    std::vector<long long> dist(nodes);
    std::vector<int> parent_node(nodes), parent_edge(nodes);
    std::vector<bool> in_queue(nodes);
    while (flow < target_flow) {
      std::fill(dist.begin(), dist.end(), MCMF_INF);
      std::fill(in_queue.begin(), in_queue.end(), false);
      std::queue<int> q;
      dist[source] = 0;
      q.push(source);
      in_queue[source] = true;
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        in_queue[u] = false;
        for (int i = 0, n = static_cast<int>(adj[u].size()); i < n; i++) {
          Edge &e = adj[u][i];
          if (e.cap > 0 && dist[e.v] > dist[u] + e.cost) {
            dist[e.v] = dist[u] + e.cost;
            parent_node[e.v] = u;
            parent_edge[e.v] = i;
            if (!in_queue[e.v]) {
              q.push(e.v);
              in_queue[e.v] = true;
            }
          }
        }
      }
      if (dist[sink] == MCMF_INF) {
        break;
      }
      int aug = target_flow - flow;
      for (int v = sink; v != source; v = parent_node[v]) {
        aug = std::min(aug, adj[parent_node[v]][parent_edge[v]].cap);
      }
      for (int v = sink; v != source; v = parent_node[v]) {
        Edge &e = adj[parent_node[v]][parent_edge[v]];
        e.cap -= aug;
        adj[v][e.rev].cap += aug;
        cost += static_cast<long long>(aug) * e.cost;
      }
      flow += aug;
    }
    return {flow, cost};
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  MinCostMaxFlow g(4);
  g.add_edge(0, 1, 2, 1);
  g.add_edge(0, 2, 1, 5);
  g.add_edge(1, 2, 1, 1);
  g.add_edge(1, 3, 1, 3);
  g.add_edge(2, 3, 2, 1);
  FlowResult result = g.min_cost_flow(0, 3, 3);
  assert(result.flow == 3);
  assert(result.cost == 13);
  return 0;
}
