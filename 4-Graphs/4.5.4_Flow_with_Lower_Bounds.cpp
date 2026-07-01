/*

Given a directed network where each edge must carry at least `lo` and at most `hi` units of flow,
find whether the lower bounds can be satisfied, and optionally optimize an `s`-to-`t` flow value.
Subtract each lower bound from its edge capacity, then record the forced imbalance: `lo` units must
leave `u` and enter `v`. A super-source supplies every node with net demand, and every node with
net surplus sends that surplus to a super-sink. The bounds are feasible exactly when all such
auxiliary edges can be saturated.

For `s`-to-`t` flow, add an infinite edge from `t` back to `s` before checking feasibility. The
flow on that artificial edge is one feasible value. After removing the auxiliary nodes, augmenting
from `s` to `t` maximizes the value; augmenting from `t` to `s` cancels as much flow as possible
and minimizes it.

- `BoundedFlow(n)` constructs a directed lower-bound flow network with nodes in $[0, n)$.
- `add_edge(u, v, lo, hi)` adds an edge with lower capacity `lo` and upper capacity `hi`.
- `feasible_circulation()` returns whether all edge bounds can be satisfied with flow conserved at
  every node.
- `max_flow(source, sink)` returns the maximum feasible `source`-to-`sink` flow, or $-1$ if no
  feasible flow exists.
- `min_flow(source, sink)` returns the minimum feasible `source`-to-`sink` flow, or $-1$ if no
  feasible flow exists.
- `edge_flows()` returns one feasible flow value for each original edge after a successful call.

All capacities should be nonnegative integers and `lo` $\leq$ `hi`. Choose `INF` larger than any
possible finite flow value.

Time Complexity:
- O(n^2*m) per feasibility check or optimized flow call, where $n$ is the number of nodes and $m$ is
  the number of edges.

Space Complexity:
- O(max(n, m)) for network storage and auxiliary arrays.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <queue>
#include <vector>

class BoundedFlow {
  struct Edge {
    int v, rev;
    int64_t cap;
  };

  struct OriginalEdge {
    int u, index;
    int64_t lo, hi;
  };

  static constexpr int64_t INF = (1LL << 60);
  int nodes;
  std::vector<std::vector<Edge>> adj;
  std::vector<OriginalEdge> original;
  std::vector<int> level, ptr;

  void add_residual_edge(int u, int v, int64_t cap) {
    adj[u].push_back(Edge{v, static_cast<int>(adj[v].size()), cap});
    adj[v].push_back(Edge{u, static_cast<int>(adj[u].size()) - 1, 0});
  }

  bool bfs(int source, int sink) {
    level.assign(static_cast<int>(adj.size()), -1);
    level[source] = 0;
    std::queue<int> q;
    q.push(source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (const Edge &e : adj[u]) {
        if (e.cap > 0 && level[e.v] == -1) {
          level[e.v] = level[u] + 1;
          q.push(e.v);
        }
      }
    }
    return level[sink] != -1;
  }

  int64_t dfs(int u, int sink, int64_t pushed) {
    if (u == sink || pushed == 0) {
      return pushed;
    }
    for (; ptr[u] < static_cast<int>(adj[u].size()); ptr[u]++) {
      Edge &e = adj[u][ptr[u]];
      if (e.cap > 0 && level[e.v] == level[u] + 1) {
        int64_t next = dfs(e.v, sink, std::min(pushed, e.cap));
        if (next > 0) {
          e.cap -= next;
          adj[e.v][e.rev].cap += next;
          return next;
        }
      }
    }
    return 0;
  }

  int64_t dinic(int source, int sink) {
    int64_t flow = 0;
    while (bfs(source, sink)) {
      ptr.assign(static_cast<int>(adj.size()), 0);
      while (int64_t pushed = dfs(source, sink, INF)) {
        flow += pushed;
      }
    }
    return flow;
  }

  bool satisfy_demands(int source, int sink, int &artificial_index, int64_t &forced_flow) {
    int super_source = static_cast<int>(adj.size());
    int super_sink = super_source + 1;
    adj.resize(super_sink + 1);
    level.resize(super_sink + 1);
    ptr.resize(super_sink + 1);
    std::vector<int64_t> balance(nodes);
    for (const OriginalEdge &e : original) {
      balance[e.u] -= e.lo;
      balance[adj[e.u][e.index].v] += e.lo;
    }
    if (source != -1) {
      artificial_index = static_cast<int>(adj[sink].size());
      add_residual_edge(sink, source, INF);
    }
    int64_t need = 0;
    for (int u = 0; u < nodes; u++) {
      if (balance[u] > 0) {
        add_residual_edge(super_source, u, balance[u]);
        need += balance[u];
      } else if (balance[u] < 0) {
        add_residual_edge(u, super_sink, -balance[u]);
      }
    }
    bool ok = dinic(super_source, super_sink) == need;
    forced_flow = source == -1 ? 0 : adj[source][adj[sink][artificial_index].rev].cap;
    return ok;
  }

  int64_t optimize_flow(int source, int sink, bool maximize) {
    int artificial_index = -1;
    int64_t value = 0;
    if (!satisfy_demands(source, sink, artificial_index, value)) {
      return -1;
    }
    Edge &artificial = adj[sink][artificial_index];
    Edge &reverse = adj[source][artificial.rev];
    artificial.cap = reverse.cap = 0;
    return maximize ? value + dinic(source, sink) : value - dinic(sink, source);
  }

 public:
  explicit BoundedFlow(int n) : nodes(n), adj(n), level(n), ptr(n) {}

  void add_edge(int u, int v, int64_t lo, int64_t hi) {
    assert(0 <= u && u < nodes && 0 <= v && v < nodes);
    assert(0 <= lo && lo <= hi);
    original.push_back(OriginalEdge{u, static_cast<int>(adj[u].size()), lo, hi});
    add_residual_edge(u, v, hi - lo);
  }

  bool feasible_circulation() {
    int artificial_index = -1;
    int64_t value = 0;
    return satisfy_demands(-1, -1, artificial_index, value);
  }

  int64_t max_flow(int source, int sink) {
    assert(0 <= source && source < nodes && 0 <= sink && sink < nodes);
    return optimize_flow(source, sink, true);
  }

  int64_t min_flow(int source, int sink) {
    assert(0 <= source && source < nodes && 0 <= sink && sink < nodes);
    return optimize_flow(source, sink, false);
  }

  std::vector<int64_t> edge_flows() const {
    std::vector<int64_t> flow;
    flow.reserve(original.size());
    for (const OriginalEdge &e : original) {
      const Edge &forward = adj[e.u][e.index];
      flow.push_back(e.hi - forward.cap);
    }
    return flow;
  }
};

/*** Example Usage ***/

int main() {
  BoundedFlow max_network(4);
  max_network.add_edge(0, 1, 1, 4);
  max_network.add_edge(0, 2, 0, 2);
  max_network.add_edge(1, 3, 1, 3);
  max_network.add_edge(2, 3, 0, 2);
  assert(max_network.max_flow(0, 3) == 5);
  std::vector<int64_t> max_flow = max_network.edge_flows();
  assert(max_flow[0] + max_flow[1] == 5);

  BoundedFlow min_network(4);
  min_network.add_edge(0, 1, 1, 4);
  min_network.add_edge(0, 2, 0, 2);
  min_network.add_edge(1, 3, 1, 3);
  min_network.add_edge(2, 3, 0, 2);
  assert(min_network.min_flow(0, 3) == 1);

  BoundedFlow circulation(3);
  circulation.add_edge(0, 1, 2, 3);
  circulation.add_edge(1, 2, 2, 3);
  circulation.add_edge(2, 0, 2, 3);
  assert(circulation.feasible_circulation());
  assert((circulation.edge_flows() == std::vector<int64_t>{2, 2, 2}));
  return 0;
}
