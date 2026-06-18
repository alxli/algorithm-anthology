/*

Given a flow network with integer or floating-point capacities, find the maximum flow from a given
source node to a given sink node. The flow along each edge may not exceed its capacity, and flow is
conserved at every node other than the source and sink.

Dinic's algorithm proceeds in phases. Each phase runs a breadth-first search to build a level graph
of shortest residual distances from the source, then finds a blocking flow that saturates that level
graph using depth-first search, before repeating on the new residual graph.
This variant stores residual edges in a flat array so callers can inspect edge flows and decompose
the final flow into source-to-sink paths.

- `Dinic<T>(n)` constructs an empty flow network with nodes numbered [0, `n`).
- `add_edge(u, v, cap)` adds a directed residual-network edge from `u` to `v` and returns its edge
  ID.
- `edge_flow(id)` returns the flow through a previously added edge.
- `max_flow(source, sink)` modifies the residual network and returns maximum flow.
- `clear_flow()` resets all edge flows to zero, allowing a fresh recomputation on the same graph.
- `min_cut(source)` returns the source side of a minimum cut after `max_flow()` has been called.
- `decompose(source, sink)` decomposes the current flow into paths from `source` to `sink`.

Repeated calls to `max_flow()` continue augmenting from the current flow. This is useful after
adding new edges or increasing capacities; call `clear_flow()` first to recompute from zero.
The capacity type `T` should be signed, since reverse residual edges store negative flow.

Time Complexity:
- O(n^2*m) per call to `max_flow()`, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n + m) per call to `min_cut()`.
- O(p*m) per call to `decompose()`, where $p$ is the number of paths returned.

Space Complexity:
- O(max(n, m)) for storage of the flow network.
- O(n) auxiliary stack and heap space for `max_flow()`.
- O(n + m) auxiliary heap space for `decompose()`, not counting the returned paths.

*/

#include <algorithm>
#include <functional>
#include <limits>
#include <queue>
#include <vector>

template<typename T>
class Dinic {
  struct Edge {
    int u, v;
    T cap, flow;
  };

  static constexpr T EPS = static_cast<T>(1e-9);
  static bool residual(const Edge &e) { return e.cap - e.flow > EPS; }

  int nodes;
  T flow;
  std::vector<Edge> edges;
  std::vector<std::vector<int>> adj;
  std::vector<int> dist, ptr;

  bool bfs(int source, int sink) {
    dist.assign(nodes, -1);
    dist[source] = 0;
    std::queue<int> q;
    q.push(source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (int id : adj[u]) {
        const Edge &e = edges[id];
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
      int id = adj[u][ptr[u]];
      Edge &e = edges[id];
      if (dist[e.v] == dist[u] + 1 && residual(e)) {
        T flow = dfs(e.v, std::min(pushed, e.cap - e.flow), sink);
        if (flow > EPS) {
          e.flow += flow;
          edges[id ^ 1].flow -= flow;
          return flow;
        }
      }
    }
    return 0;
  }

 public:
  struct FlowPath {
    T flow;
    std::vector<int> vertices;
    std::vector<int> edges;
  };

  explicit Dinic(int n) : nodes(n), flow(0), adj(n), dist(n), ptr(n) {}

  int add_edge(int u, int v, T cap) {
    int id = static_cast<int>(edges.size());
    adj[u].push_back(id);
    edges.push_back(Edge{u, v, cap, 0});
    adj[v].push_back(id + 1);
    edges.push_back(Edge{v, u, 0, 0});
    return id;
  }

  T edge_flow(int id) const { return edges[id].flow; }

  void clear_flow() {
    for (Edge &e : edges) {
      e.flow = 0;
    }
    flow = 0;
  }

  T max_flow(int source, int sink) {
    while (bfs(source, sink)) {
      ptr.assign(nodes, 0);
      while (true) {
        T flow = dfs(source, std::numeric_limits<T>::max(), sink);
        if (flow <= EPS) {
          break;
        }
        this->flow += flow;
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
      for (int id : adj[u]) {
        const Edge &e = edges[id];
        if (!reachable[e.v] && residual(e)) {
          reachable[e.v] = true;
          q.push(e.v);
        }
      }
    }
    return reachable;
  }

  std::vector<FlowPath> decompose(int source, int sink) const {
    std::vector<T> rem(edges.size());
    for (int id = 0; id < static_cast<int>(edges.size()); id++) {
      rem[id] = edges[id].flow;
    }
    std::vector<FlowPath> res;
    while (true) {
      FlowPath path{std::numeric_limits<T>::max(), {source}, {}};
      std::vector<bool> seen(nodes);
      std::function<bool(int)> find_path = [&](int u) {
        if (u == sink) {
          return true;
        }
        seen[u] = true;
        for (int id : adj[u]) {
          const Edge &e = edges[id];
          if (rem[id] > EPS && !seen[e.v]) {
            path.edges.push_back(id);
            path.vertices.push_back(e.v);
            if (find_path(e.v)) {
              return true;
            }
            path.vertices.pop_back();
            path.edges.pop_back();
          }
        }
        return false;
      };
      if (!find_path(source)) {
        break;
      }
      for (int id : path.edges) {
        path.flow = std::min(path.flow, rem[id]);
      }
      for (int id : path.edges) {
        rem[id] -= path.flow;
      }
      res.push_back(path);
    }
    return res;
  }
};

/*** Example Usage and Output:

Max flow from 0 to 5: 5
Flow decomposition:
2: 0 -> 1 -> 3 -> 5
1: 0 -> 1 -> 4 -> 5
2: 0 -> 2 -> 4 -> 5

***/

#include <cassert>
#include <iostream>
using namespace std;

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
  int id01 = g.add_edge(0, 1, 4);
  int id02 = g.add_edge(0, 2, 3);
  int id13 = g.add_edge(1, 3, 2);
  int id14 = g.add_edge(1, 4, 1);
  int id24 = g.add_edge(2, 4, 2);
  int id35 = g.add_edge(3, 5, 2);
  int id45 = g.add_edge(4, 5, 3);
  int flow = g.max_flow(0, 5);
  cout << "Max flow from 0 to 5: " << flow << '\n';
  assert(flow == 5);
  assert(g.max_flow(0, 5) == 5);
  assert(g.edge_flow(id01) == 3);
  assert(g.edge_flow(id02) == 2);
  assert(g.edge_flow(id13) == 2);
  assert(g.edge_flow(id14) == 1);
  assert(g.edge_flow(id24) == 2);
  assert(g.edge_flow(id35) == 2);
  assert(g.edge_flow(id45) == 3);
  vector<bool> cut = g.min_cut(0);
  assert(cut[0] && !cut[5]);
  vector<Dinic<int>::FlowPath> paths = g.decompose(0, 5);
  cout << "Flow decomposition:\n";
  int total = 0;
  for (const auto &path : paths) {
    cout << path.flow << ": ";
    for (int i = 0; i < static_cast<int>(path.vertices.size()); i++) {
      if (i > 0) {
        cout << " -> ";
      }
      cout << path.vertices[i];
    }
    cout << '\n';
    assert(path.vertices.front() == 0 && path.vertices.back() == 5);
    assert(path.edges.size() + 1 == path.vertices.size());
    total += path.flow;
  }
  assert(total == 5);
  g.clear_flow();
  assert(g.max_flow(0, 5) == 5);
  return 0;
}
