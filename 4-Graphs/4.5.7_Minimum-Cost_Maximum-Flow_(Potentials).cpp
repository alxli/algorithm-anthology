/*

Given a flow network with capacities and edge costs, find a minimum-cost flow from a source node to
a sink node. This is the successive shortest path method with Johnson potentials. A node potential
$h[v]$ transforms each edge cost $c(u, v)$ into a reduced cost $c(u, v) + h[u] - h[v]$; when all
reduced costs are nonnegative, Dijkstra's algorithm can find the shortest augmenting path safely.
After each augmentation, $h$ is updated with the new shortest-path distances so that reduced costs
remain nonnegative on the updated residual graph.

- `MinCostMaxFlow<T, C>(n)` constructs an empty residual network with nodes numbered $[0, `n`)$.
- `add_edge(u, v, cap, cost, rev_cap = 0)` adds a directed residual-network edge and returns its
  edge ID.
- `edge_flow(id)` returns the flow through a previously added edge.
- `clear_flow()` resets all edge flows to zero.
- `min_cost_flow(source, sink, target_flow)` sends up to `target_flow` additional units of flow and
  returns a pair (`flow`, `cost`) for the flow sent by that call. If the returned flow is smaller
  than `target_flow`, the residual network cannot carry the requested amount.

If all initial edge costs are nonnegative, the initial potentials can be zero and the first shortest
path search is already Dijkstra-safe. If negative costs are present, this implementation first runs
SPFA to compute valid initial potentials; it assumes there is no reachable negative-cost cycle. The
capacity type `T` should be signed, since reverse residual edges store negative flow.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the number of nodes.
- O(1) per call to `add_edge()`.
- O(m) to check whether initial potentials are needed, plus O(n*m) to compute them when negative
  residual costs are reachable.
- O(f*m*log n) for the augmenting paths, where $f$ is the amount of flow sent, $n$ is the number of
  nodes, and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the residual network and auxiliary arrays.

*/

#include <algorithm>
#include <cstdint>
#include <functional>
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
  static bool residual(const Edge &e) { return e.cap - e.flow > EPS; }

  int nodes;
  std::vector<Edge> edges;
  std::vector<std::vector<int>> adj;
  std::vector<C> potential;

  void init_potentials(int source) {
    static const C INF_COST = std::numeric_limits<C>::max() / 4;
    std::fill(potential.begin(), potential.end(), 0);
    bool has_negative_cost = false;
    for (const Edge &e : edges) {
      has_negative_cost |= residual(e) && e.cost < 0;
    }
    if (!has_negative_cost) {
      return;
    }
    std::fill(potential.begin(), potential.end(), INF_COST);
    std::vector<char> in_queue(nodes);
    std::queue<int> q;
    potential[source] = 0;
    q.push(source);
    in_queue[source] = true;
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      in_queue[u] = false;
      for (int id : adj[u]) {
        const Edge &e = edges[id];
        if (residual(e) && potential[u] + e.cost < potential[e.v]) {
          potential[e.v] = potential[u] + e.cost;
          if (!in_queue[e.v]) {
            q.push(e.v);
            in_queue[e.v] = true;
          }
        }
      }
    }
    for (C &p : potential) {
      if (p == INF_COST) {
        p = 0;
      }
    }
  }

 public:
  explicit MinCostMaxFlow(int n) : nodes(n), adj(n), potential(n, 0) {}

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
    std::fill(potential.begin(), potential.end(), 0);
  }

  std::pair<T, C> min_cost_flow(int source, int sink, T target_flow) {
    static const C INF_COST = std::numeric_limits<C>::max() / 4;
    init_potentials(source);
    T flow = 0;
    C cost = 0;
    std::vector<C> dist(nodes);
    std::vector<int> parent_edge(nodes);
    while (flow < target_flow) {
      std::fill(dist.begin(), dist.end(), INF_COST);
      std::priority_queue<
          std::pair<C, int>, std::vector<std::pair<C, int>>, std::greater<std::pair<C, int>>>
          pq;
      dist[source] = 0;
      pq.push({0, source});
      while (!pq.empty()) {
        auto [du, u] = pq.top();
        pq.pop();
        if (du != dist[u]) {
          continue;
        }
        for (int id : adj[u]) {
          const Edge &e = edges[id];
          C reduced_cost = e.cost + potential[u] - potential[e.v];
          if (residual(e) && dist[u] + reduced_cost < dist[e.v]) {
            dist[e.v] = dist[u] + reduced_cost;
            parent_edge[e.v] = id;
            pq.push({dist[e.v], e.v});
          }
        }
      }
      if (dist[sink] == INF_COST) {
        break;
      }
      for (int i = 0; i < nodes; i++) {
        if (dist[i] < INF_COST) {
          potential[i] += dist[i];
        }
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
  MinCostMaxFlow<int> g(6);
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
