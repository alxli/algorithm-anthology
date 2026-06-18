/*

Given a flow network with integer or floating-point capacities, find the maximum flow from a given
source node to a given sink node. The flow along each edge may not exceed its capacity, and flow is
conserved at every node other than the source and sink.

Edmonds-Karp is the Ford-Fulkerson method with the augmenting path always chosen as a shortest one
(fewest edges) via breadth-first search. This bounds the number of augmentations at O(n*m),
independent of the capacity magnitudes.

- `EdmondsKarp<T>(n)` constructs an empty flow network with nodes numbered [0, `n`).
- `add_edge(u, v, cap)` adds a directed residual-network edge from `u` to `v`.
- `max_flow(source, sink)` modifies the residual network and returns maximum flow.
- `clear_flow()` resets all edge flows to zero, allowing a fresh recomputation on the same graph.
- `min_cut(source)` returns the source side of a minimum cut after `max_flow()` has been called.

Repeated calls to `max_flow()` continue augmenting from the current flow. This is useful after
adding new edges or increasing capacities; call `clear_flow()` first to recompute from zero.
The capacity type `T` should be signed, since reverse residual edges store negative flow.

Time Complexity:
- O(min(n*m^2, m*f)) per call to `max_flow()`, where $n$ is the number of nodes, $m$ is the number
  of edges, and $f$ is the maximum flow.
- O(n + m) per call to `min_cut()`.

Space Complexity:
- O(max(n, m)) for storage of the flow network, where $n$ is the number of nodes and $m$ is the
  number of edges.

*/

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

template<typename T>
class EdmondsKarp {
  struct Edge {
    int u, v, rev;
    T cap, flow;
  };

  static constexpr T EPS = static_cast<T>(1e-9);
  static bool residual(const Edge &e) { return e.cap - e.flow > EPS; }

  int nodes;
  T flow;
  std::vector<std::vector<Edge>> adj;

 public:
  explicit EdmondsKarp(int n) : nodes(n), flow(0), adj(n) {}

  void add_edge(int u, int v, T cap) {
    adj[u].push_back(Edge{u, v, static_cast<int>(adj[v].size()), cap, 0});
    adj[v].push_back(Edge{v, u, static_cast<int>(adj[u].size()) - 1, 0, 0});
  }

  void clear_flow() {
    for (auto &edges : adj) {
      for (Edge &e : edges) {
        e.flow = 0;
      }
    }
    flow = 0;
  }

  T max_flow(int source, int sink) {
    while (true) {
      std::vector<Edge *> pred(nodes, nullptr);
      std::queue<int> q;
      q.push(source);
      while (!q.empty() && pred[sink] == nullptr) {
        int u = q.front();
        q.pop();
        for (Edge &e : adj[u]) {
          if (e.v != source && pred[e.v] == nullptr && residual(e)) {
            pred[e.v] = &e;
            q.push(e.v);
          }
        }
      }
      if (pred[sink] == nullptr) {
        break;
      }
      T flow = std::numeric_limits<T>::max();
      for (int u = sink; u != source; u = pred[u]->u) {
        flow = std::min(flow, pred[u]->cap - pred[u]->flow);
      }
      for (int u = sink; u != source; u = pred[u]->u) {
        pred[u]->flow += flow;
        adj[pred[u]->v][pred[u]->rev].flow -= flow;
      }
      this->flow += flow;
    }
    return flow;
  }

  std::vector<bool> min_cut(int source) const {
    std::vector<bool> reachable(nodes);
    std::queue<int> q;
    reachable[source] = true;
    q.push(source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (const Edge &e : adj[u]) {
        if (!reachable[e.v] && residual(e)) {
          reachable[e.v] = true;
          q.push(e.v);
        }
      }
    }
    return reachable;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  EdmondsKarp<int> g(6);
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
  g.add_edge(0, 1, 4);
  g.add_edge(0, 2, 3);
  g.add_edge(1, 3, 2);
  g.add_edge(1, 4, 1);
  g.add_edge(2, 4, 2);
  g.add_edge(3, 5, 2);
  g.add_edge(4, 5, 3);
  assert(g.max_flow(0, 5) == 5);
  assert(g.max_flow(0, 5) == 5);
  std::vector<bool> cut = g.min_cut(0);
  assert(cut[0] && !cut[5]);
  g.clear_flow();
  assert(g.max_flow(0, 5) == 5);
  return 0;
}
