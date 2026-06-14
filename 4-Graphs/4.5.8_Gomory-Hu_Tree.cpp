/*

Given an undirected graph with nonnegative edge weights, build a Gomory-Hu tree: a weighted tree on
the same vertices that represents every pairwise minimum s-t cut value. For any two vertices `u` and
`v`, the minimum cut value between them in the original graph is the minimum edge weight on the path
from `u` to `v` in the Gomory-Hu tree.

The algorithm starts with every vertex attached to vertex 0, then performs `n - 1` max-flow/min-cut
computations. After computing the minimum cut between vertex `s` and its current parent, vertices on
the `s` side of that cut are reparented under `s`, gradually refining the cut-equivalent tree.

- `gomory_hu(n, edges)` returns the `n - 1` edges of a Gomory-Hu tree as `(u, v, weight)` triples.
- `min_cut_value(n, tree, source, sink)` returns the minimum cut value between two vertices using a
  Gomory-Hu tree. Note that for many pairwise cut queries on the same tree, it's more efficient to
  prebuild the tree adjacency once and answer minimum edge-on-path queries with LCA/binary lifting
  instead of calling `min_cut_value()` each time.
- The input `edges` are undirected weighted edges `(u, v, weight)`. Parallel edges are allowed.

Time Complexity:
- O(n) calls to maximum flow. With the included Dinic implementation, this is O(n^3*m) in the worst
  case, where $n$ is the number of vertices and $m$ is the number of undirected edges.
- O(n) per call to `min_cut_value()`.

Space Complexity:
- O(max(n, m)) for the residual network and auxiliary arrays.

*/

#include <algorithm>
#include <cassert>
#include <limits>
#include <queue>
#include <utility>
#include <vector>

template<class T>
class Dinic {
  struct Edge {
    int v, rev;
    T cap;
  };

  int nodes;
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
        if (dist[e.v] < 0 && e.cap > 0) {
          dist[e.v] = dist[u] + 1;
          q.push(e.v);
        }
      }
    }
    return dist[sink] >= 0;
  }

  T dfs(int u, int sink, T pushed) {
    if (u == sink || pushed == 0) {
      return pushed;
    }
    for (; ptr[u] < static_cast<int>(adj[u].size()); ptr[u]++) {
      Edge &e = adj[u][ptr[u]];
      if (dist[e.v] == dist[u] + 1 && e.cap > 0) {
        T pushed_next = dfs(e.v, sink, std::min(pushed, e.cap));
        if (pushed_next > 0) {
          e.cap -= pushed_next;
          adj[e.v][e.rev].cap += pushed_next;
          return pushed_next;
        }
      }
    }
    return 0;
  }

 public:
  explicit Dinic(int n) : nodes(n), adj(n), dist(n), ptr(n) {}

  void add_edge(int u, int v, T cap) {
    adj[u].push_back(Edge{v, static_cast<int>(adj[v].size()), cap});
    adj[v].push_back(Edge{u, static_cast<int>(adj[u].size()) - 1, 0});
  }

  T max_flow(int source, int sink) {
    T flow = 0;
    while (bfs(source, sink)) {
      ptr.assign(nodes, 0);
      while (true) {
        T pushed = dfs(source, sink, std::numeric_limits<T>::max());
        if (pushed == 0) {
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
        if (!reachable[e.v] && e.cap > 0) {
          reachable[e.v] = true;
          q.push(e.v);
        }
      }
    }
    return reachable;
  }
};

template<class T>
struct CutTreeEdge {
  int u, v;
  T weight;
};

template<class T>
std::vector<CutTreeEdge<T>> gomory_hu(int n, const std::vector<CutTreeEdge<T>> &edges) {
  assert(n >= 1);
  std::vector<int> parent(n, 0);
  std::vector<T> cut_value(n);
  for (int s = 1; s < n; s++) {
    int t = parent[s];
    Dinic<T> mf(n);
    for (const CutTreeEdge<T> &e : edges) {
      mf.add_edge(e.u, e.v, e.weight);
      mf.add_edge(e.v, e.u, e.weight);
    }
    T flow = mf.max_flow(s, t);
    std::vector<bool> cut = mf.min_cut(s);
    for (int v = s + 1; v < n; v++) {
      if (parent[v] == t && cut[v] == cut[s]) {
        parent[v] = s;
      }
    }
    if (cut[parent[t]]) {
      parent[s] = parent[t];
      parent[t] = s;
      cut_value[s] = cut_value[t];
      cut_value[t] = flow;
    } else {
      cut_value[s] = flow;
    }
  }
  std::vector<CutTreeEdge<T>> tree;
  for (int v = 1; v < n; v++) {
    tree.push_back({v, parent[v], cut_value[v]});
  }
  return tree;
}

template<class T>
T min_cut_value_dfs(
    const std::vector<std::vector<std::pair<int, T>>> &adj, std::vector<bool> &seen, int u,
    int sink, T best
) {
  if (u == sink) {
    return best;
  }
  seen[u] = true;
  for (auto [v, w] : adj[u]) {
    if (!seen[v]) {
      T res = min_cut_value_dfs(adj, seen, v, sink, std::min(best, w));
      if (res != -1) {
        return res;
      }
    }
  }
  return -1;
}

template<class T>
T min_cut_value(int n, const std::vector<CutTreeEdge<T>> &tree, int source, int sink) {
  assert(0 <= source && source < n && 0 <= sink && sink < n && source != sink);
  // For many queries, prebuild this adjacency and add LCA/binary lifting for min edge on path.
  std::vector<std::vector<std::pair<int, T>>> adj(n);
  for (const CutTreeEdge<T> &e : tree) {
    adj[e.u].push_back({e.v, e.weight});
    adj[e.v].push_back({e.u, e.weight});
  }
  std::vector<bool> seen(n);
  return min_cut_value_dfs(adj, seen, source, sink, std::numeric_limits<T>::max());
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<CutTreeEdge<int>> edges{
      {0, 1, 3}, {0, 2, 2}, {1, 2, 1}, {1, 3, 2}, {2, 3, 4},
  };
  vector<CutTreeEdge<int>> tree = gomory_hu(4, edges);
  assert(tree.size() == 3);
  assert(min_cut_value(4, tree, 0, 1) == 5);
  assert(min_cut_value(4, tree, 0, 2) == 5);
  assert(min_cut_value(4, tree, 0, 3) == 5);
  assert(min_cut_value(4, tree, 2, 3) == 6);
  return 0;
}
