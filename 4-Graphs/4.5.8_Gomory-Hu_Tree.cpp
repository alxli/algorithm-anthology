/*

Given an undirected graph with nonnegative edge weights, build a Gomory-Hu tree: a weighted tree on
the same vertices that represents every pairwise minimum $s$-$t$ cut value. For any two vertices $u$
and $v$, the minimum cut value between them in the original graph is the minimum edge weight on the
path from $u$ to $v$ in the Gomory-Hu tree.

The algorithm starts with every vertex attached to vertex 0, then performs $n - 1$ max-flow/min-cut
computations. After computing the minimum cut between vertex $s$ and its current parent, vertices on
the $s$ side of that cut are reparented under $s$, gradually refining the cut-equivalent tree.

- `gomory_hu(n, edges)` returns the $`n` - 1$ edges of a Gomory-Hu tree as (`u`, `v`, `weight`)
  tripes for an undirected, weighted graph with `n` nodes and `edges` of the same shape.
- `min_cut_value(n, tree, source, sink)` returns the minimum cut value between two vertices using a
  Gomory-Hu tree. Note that for many pairwise cut queries on the same tree, it's more efficient to
  prebuild the tree adjacency once and answer minimum edge-on-path queries with LCA/binary lifting
  instead of calling `min_cut_value()` each time.

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
#include <tuple>
#include <utility>
#include <vector>

template<typename T>
class Dinic {
  int nodes;
  std::vector<std::vector<std::tuple<int, int, T>>> adj;  // (v, rev, cap)
  std::vector<int> dist, ptr;

  bool bfs(int source, int sink) {
    dist.assign(nodes, -1);
    dist[source] = 0;
    std::queue<int> q;
    q.push(source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (const auto &[v, rev, cap] : adj[u]) {
        if (dist[v] < 0 && cap > 0) {
          dist[v] = dist[u] + 1;
          q.push(v);
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
      auto &[v, rev, cap] = adj[u][ptr[u]];
      if (dist[v] == dist[u] + 1 && cap > 0) {
        T pushed_next = dfs(v, sink, std::min(pushed, cap));
        if (pushed_next > 0) {
          cap -= pushed_next;
          std::get<2>(adj[v][rev]) += pushed_next;
          return pushed_next;
        }
      }
    }
    return 0;
  }

 public:
  explicit Dinic(int n) : nodes(n), adj(n), dist(n), ptr(n) {}

  void add_edge(int u, int v, T cap) {
    adj[u].emplace_back(v, static_cast<int>(adj[v].size()), cap);
    adj[v].emplace_back(u, static_cast<int>(adj[u].size()) - 1, 0);
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

  std::vector<char> min_cut(int source) const {
    std::vector<char> reachable(nodes);
    std::queue<int> q;
    reachable[source] = true;
    q.push(source);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (const auto &[v, rev, cap] : adj[u]) {
        if (!reachable[v] && cap > 0) {
          reachable[v] = true;
          q.push(v);
        }
      }
    }
    return reachable;
  }
};

template<typename T>
using Edge = std::tuple<int, int, T>;

template<typename T>
std::vector<Edge<T>> gomory_hu(int n, const std::vector<Edge<T>> &edges) {
  assert(n >= 1);
  std::vector<int> parent(n, 0);
  std::vector<T> cut_value(n);
  for (int s = 1; s < n; s++) {
    int t = parent[s];
    Dinic<T> mf(n);
    for (const auto &[u, v, w] : edges) {
      mf.add_edge(u, v, w);
      mf.add_edge(v, u, w);
    }
    T flow = mf.max_flow(s, t);
    std::vector<char> cut = mf.min_cut(s);
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
  std::vector<Edge<T>> tree;
  for (int v = 1; v < n; v++) {
    tree.emplace_back(v, parent[v], cut_value[v]);
  }
  return tree;
}

template<typename T>
T min_cut_value_dfs(
    const std::vector<std::vector<std::pair<int, T>>> &adj, std::vector<char> &seen, int u,
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

template<typename T>
T min_cut_value(int n, const std::vector<Edge<T>> &tree, int source, int sink) {
  assert(0 <= source && source < n && 0 <= sink && sink < n && source != sink);
  // For many queries, prebuild this adjacency and add LCA/binary lifting for min edge on path.
  std::vector<std::vector<std::pair<int, T>>> adj(n);
  for (const auto &[u, v, w] : tree) {
    adj[u].emplace_back(v, w);
    adj[v].emplace_back(u, w);
  }
  std::vector<char> seen(n);
  return min_cut_value_dfs(adj, seen, source, sink, std::numeric_limits<T>::max());
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<Edge<int>> edges{
      {0, 1, 3}, {0, 2, 2}, {1, 2, 1}, {1, 3, 2}, {2, 3, 4},
  };
  auto tree = gomory_hu(4, edges);
  assert(tree.size() == 3);
  assert(min_cut_value(4, tree, 0, 1) == 5);
  assert(min_cut_value(4, tree, 0, 2) == 5);
  assert(min_cut_value(4, tree, 0, 3) == 5);
  assert(min_cut_value(4, tree, 2, 3) == 6);
  return 0;
}
