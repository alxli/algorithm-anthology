/*

Given a flow network with integer or floating-point capacities, find the maximum flow from a given
source node to a given sink node. The flow along each edge may not exceed its capacity, and flow is
conserved at every node other than the source and sink.

Dinic's algorithm proceeds in phases. Each phase runs a breadth-first search to build a level graph
of shortest residual distances from the source, then finds a blocking flow that saturates that level
graph using depth-first search, before repeating on the new residual graph.

- `Dinic<T>(n)` constructs an empty flow network with nodes numbered [0, `n`).
- `add_edge(u, v, cap)` adds a directed residual-network edge from `u` to `v`.
- `max_flow(source, sink)` modifies the residual network and returns maximum flow.
- `clear_flow()` resets all edge flows to zero, allowing a fresh recomputation on the same graph.
- `min_cut(source)` returns the source side of a minimum cut after `max_flow()` has been called.

Repeated calls to `max_flow()` continue augmenting from the current flow. This is useful after
adding new edges or increasing capacities; call `clear_flow()` first to recompute from zero. The
capacity type `T` should be signed, since reverse residual edges store negative flow.

Time Complexity:
- O(n^2*m) per call to `max_flow()`, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n + m) per call to `min_cut()`.

Space Complexity:
- O(max(n, m)) for storage of the flow network.
- O(n) auxiliary stack and heap space for `max_flow()`.

*/

#include <algorithm>
#include <limits>
#include <queue>
#include <vector>

template<typename T>
class Dinic {
  struct Edge {
    int v, rev;
    T cap, flow;
  };

  static constexpr T EPS = static_cast<T>(1e-9);
  static bool residual(const Edge &e) { return e.cap - e.flow > EPS; }

  int nodes;
  T flow;
  std::vector<std::vector<Edge>> adj;
  std::vector<int> dist, ptr;

  bool bfs(int source, int sink) {
    dist.assign(nodes, -1);
    dist[source] = 0;
    std::queue<int> q;
    q.push(source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (const Edge &e : adj[u]) {
        if (dist[e.v] < 0 && residual(e)) {
          dist[e.v] = dist[u] + 1;
          q.push(e.v);
        }
      }
    }
    return dist[sink] >= 0;
  }

  T dfs(int u, T pushed, int sink) {
    if (u == sink || pushed <= EPS) {
      return pushed;
    }
    for (; ptr[u] < static_cast<int>(adj[u].size()); ptr[u]++) {
      Edge &e = adj[u][ptr[u]];
      if (dist[e.v] == dist[u] + 1 && residual(e)) {
        T pushed_next = dfs(e.v, std::min(pushed, e.cap - e.flow), sink);
        if (pushed_next > EPS) {
          e.flow += pushed_next;
          adj[e.v][e.rev].flow -= pushed_next;
          return pushed_next;
        }
      }
    }
    return 0;
  }

 public:
  explicit Dinic(int n) : nodes(n), flow(0), adj(n), dist(n), ptr(n) {}

  void add_edge(int u, int v, T cap) {
    adj[u].push_back(Edge{v, static_cast<int>(adj[v].size()), cap, 0});
    adj[v].push_back(Edge{u, static_cast<int>(adj[u].size()) - 1, 0, 0});
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
    while (bfs(source, sink)) {
      ptr.assign(nodes, 0);
      while (true) {
        T pushed = dfs(source, std::numeric_limits<T>::max(), sink);
        if (pushed <= EPS) {
          break;
        }
        flow += pushed;
      }
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
  Dinic<int> g(6);
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
