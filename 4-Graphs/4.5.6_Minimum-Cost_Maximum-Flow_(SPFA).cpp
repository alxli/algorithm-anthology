/*

Given a flow network with capacities and edge costs, find a minimum-cost flow from a source node to
a sink node. This is the successive shortest path method: repeatedly find a minimum-cost augmenting
path from source to sink in the residual graph and push as much flow as it allows, until the flow
target is met or no path remains. The path search uses SPFA (queue-based Bellman-Ford), since
residual edges carry negated costs.

- `MinCostMaxFlow<T, C>(n)` constructs an empty residual network with nodes numbered $[0, `n`)$.
- `add_edge(u, v, cap, cost, rev_cap = 0)` adds a directed residual-network edge and returns its
  edge ID.
- `edge_flow(id)` returns the flow through a previously added edge.
- `clear_flow()` resets all edge flows to zero.
- `min_cost_flow(source, sink, target_flow)` sends up to `target_flow` additional units of flow and
  returns $(flow, cost)$ for the flow sent by that call. If the returned flow is smaller than
  `target_flow`, the residual network cannot carry the requested amount.

This implementation uses shortest augmenting paths with SPFA, so it supports negative edge costs as
long as the residual graph has no reachable negative-cost cycle. For dense or adversarial inputs, a
potential-based Dijkstra version is usually faster when reduced costs are nonnegative. The capacity
type `T` should be signed, since reverse residual edges store negative flow.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the number of nodes.
- O(1) per call to `add_edge()`.
- O(f*n*m) per call to `min_cost_flow()`, where $f$ is the amount of flow sent, $n$ is the number of
  nodes, and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the residual network and auxiliary arrays.

*/

#include <algorithm>
#include <cstdint>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

template<typename T, typename C = int64_t>
class MinCostMaxFlow {
  struct Edge {
    int u, v;
    T cap, flow;
    C cost;
  };

  static constexpr T EPS = static_cast<T>(1e-9);

  int nodes;
  std::vector<Edge> edges;
  std::vector<std::vector<int>> adj;

 public:
  explicit MinCostMaxFlow(int n) : nodes(n), adj(n) {}

  int add_edge(int u, int v, T cap, C cost, T rev_cap = 0) {
    int id = static_cast<int>(edges.size());
    adj[u].push_back(id);
    edges.push_back(Edge{u, v, cap, 0, cost});
    adj[v].push_back(id + 1);
    edges.push_back(Edge{v, u, rev_cap, 0, -cost});
    return id;
  }

  T edge_flow(int id) const { return edges[id].flow; }

  void clear_flow() {
    for (Edge &e : edges) {
      e.flow = 0;
    }
  }

  std::pair<T, C> min_cost_flow(int source, int sink, T target_flow) {
    static const C INF_COST = std::numeric_limits<C>::max() / 4;
    T flow = 0;
    C cost = 0;
    std::vector<C> dist(nodes);
    std::vector<int> parent_edge(nodes);
    std::vector<char> in_queue(nodes);
    while (flow < target_flow) {
      std::fill(dist.begin(), dist.end(), INF_COST);
      std::fill(in_queue.begin(), in_queue.end(), false);
      std::queue<int> q;
      dist[source] = 0;
      q.push(source);
      in_queue[source] = true;
      while (!q.empty()) {
        int u = q.front();
        q.pop();
        in_queue[u] = false;
        for (int id : adj[u]) {
          const Edge &e = edges[id];
          if (e.cap - e.flow > EPS && dist[u] + e.cost < dist[e.v]) {
            dist[e.v] = dist[u] + e.cost;
            parent_edge[e.v] = id;
            if (!in_queue[e.v]) {
              q.push(e.v);
              in_queue[e.v] = true;
            }
          }
        }
      }
      if (dist[sink] == INF_COST) {
        break;
      }
      T aug = target_flow - flow;
      for (int v = sink; v != source; v = edges[parent_edge[v]].u) {
        const Edge &e = edges[parent_edge[v]];
        aug = std::min(aug, e.cap - e.flow);
      }
      for (int v = sink; v != source; v = edges[parent_edge[v]].u) {
        int id = parent_edge[v];
        Edge &e = edges[id];
        e.flow += aug;
        edges[id ^ 1].flow -= aug;
        cost += aug * e.cost;
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
  MinCostMaxFlow<int> g(6);
  // Example graph after max flow, with each edge labeled flow/capacity:
  //            2/2
  //       1 --------> 3
  //      / \          |
  // 3/4 /   \ 1/1     | 2/2
  //    /     v        v
  //   0       4 ----> 5
  //    \     ^   3/3
  // 2/3 \   / 2/2
  //      v /
  //       2
  int id01 = g.add_edge(0, 1, 4, 1);
  g.add_edge(0, 2, 3, 1);
  g.add_edge(1, 3, 2, 1);
  g.add_edge(1, 4, 1, 2);
  g.add_edge(2, 4, 2, 1);
  g.add_edge(3, 5, 2, 1);
  g.add_edge(4, 5, 3, 1);
  auto [flow, cost] = g.min_cost_flow(0, 5, 5);
  assert(flow == 5);
  assert(cost == 16);
  assert(g.edge_flow(id01) == 3);
  g.clear_flow();
  auto [flow2, cost2] = g.min_cost_flow(0, 5, 5);
  assert(flow2 == 5);
  assert(cost2 == 16);
  return 0;
}
