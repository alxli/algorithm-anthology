/*

Maintain a tree or forest with values associated with either edges or nodes, while supporting both
dynamic queries and dynamic updates of all values on a given path between two nodes in the tree.
Heavy-light decomposition chooses at most one heavy child for each node and partitions the tree into
maximal paths of heavy edges. This implementation keeps a child on its parent's path when its
subtree contains at least half of its parent's nodes. Every remaining edge is light and at least
halves the remaining subtree size, so walking from any node to the root crosses O(log n) heavy
paths. Each path stores its values in its own lazy segment tree, decomposing any path query or
update into O(log n) contiguous range operations.

The query operation is defined by an associative `combine()` function. The default code below
assumes a numerical tree type, defining queries for the "min" of the target range. Another possible
query operation is "sum", in which case `combine(a, b)` should return `a + b`. For
direction-independent path queries, `combine()` should also be commutative; otherwise, store enough
information in each aggregate to combine paths in the required order.

The update operation is defined by `apply_delta()` and `compose_deltas()`. A delta must act on an
aggregate summary of a path of length `len`: `apply_delta(v, d, len)` returns the aggregate after
applying update `d` to every element represented by aggregate `v`. Pending deltas are combined in
chronological order by `compose_deltas(old, d)`, meaning "apply `old`, then apply `d`". These hooks
do not support arbitrary query/update pairings; the delta operation must distribute over
`combine()`, and composed deltas must have the same effect as applying their updates sequentially.
The default code below defines updates that "set" a path's edges or nodes to a new value. For range
increment updates, `apply_delta(v, d, len)` would return `v + d` for min/max queries, or
`v + d * len` for sum queries, and `compose_deltas(old, d)` would return `old + d`.

- `HeavyLight<T, VALUES_ON_EDGES = true>(adj, v)` constructs a new heavy light decomposition on a
  forest defined by the adjacency list `adj`, with all values initialized to `v`. Its entries must
  be valid indices into `adj`, and no duplicate edges should exist. Set `VALUES_ON_EDGES` to false
  to store values on nodes instead.
- `query(u, v)` returns the result of `combine()` applied to all values on the path from node `u` to
  node `v`. The nodes must be in the same tree and, when values are on edges, must be distinct.
- `update(u, v, d)` modifies all values on the path from node `u` to node `v` by applying the delta
  `d`. The nodes must be in the same tree.
- `for_each_path(u, v, include_lca, f)` decomposes the path from node `u` to node `v` into heavy
  path ranges and calls `f(path, lo, hi, up)` on each range, where `up` says the path segment is
  traversed upward from `u` toward the LCA. If `VALUES_ON_EDGES = true`, pass `include_lca = false`
  to skip the LCA's node slot. Returns false if `u` and `v` are in different trees.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the number of nodes.
- O(log n) calls to `f()` per call to `for_each_path()`.
- O(log^2 n) per call to `query()` and `update()`.

Space Complexity:
- O(n) for storage of the decomposition.
- O(n) auxiliary stack space for the constructor.
- O(log n) auxiliary for `for_each_path()`, `query()`, and `update()`.

*/

#include <algorithm>
#include <cassert>
#include <optional>
#include <tuple>
#include <vector>

template<typename T, bool VALUES_ON_EDGES = true>
class HeavyLight {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d, int len) { return d; }
  static T compose_deltas(const T &old, const T &d) { return d; }

  std::vector<std::vector<int>> adj;
  std::vector<std::vector<T>> value;
  std::vector<std::vector<std::optional<T>>> delta;
  std::vector<std::vector<int>> len;
  std::vector<int> size, parent, root, tin, tout, path, pathlen, pathpos, pathroot;
  int counter, paths;

  void dfs(int u, int p, int r) {
    tin[u] = counter++;
    parent[u] = p;
    root[u] = r;
    size[u] = 1;
    for (int v : adj[u]) {
      if (v != p) {
        dfs(v, u, r);
        size[u] += size[v];
      }
    }
    tout[u] = counter++;
  }

  int new_path(int u) {
    pathroot[paths] = u;
    return paths++;
  }

  void build_paths(int u, int path_id) {
    path[u] = path_id;
    pathpos[u] = pathlen[path_id]++;
    for (int v : adj[u]) {
      if (v != parent[u]) {
        build_paths(v, (2 * size[v] >= size[u]) ? path_id : new_path(v));
      }
    }
  }

  inline T applied_value(int path_id, int i) {
    return delta[path_id][i] ? apply_delta(value[path_id][i], *delta[path_id][i], len[path_id][i])
                             : value[path_id][i];
  }

  void push_delta(int path_id, int i) {
    int d = 0;
    while ((i >> d) > 0) {
      d++;
    }
    for (d -= 2; d >= 0; d--) {
      int l = (i >> d), r = (l ^ 1), n = l / 2;
      if (delta[path_id][n]) {
        value[path_id][n] = applied_value(path_id, n);
        const T &d = *delta[path_id][n];
        delta[path_id][l] = delta[path_id][l] ? compose_deltas(*delta[path_id][l], d) : d;
        delta[path_id][r] = delta[path_id][r] ? compose_deltas(*delta[path_id][r], d) : d;
        delta[path_id][n].reset();
      }
    }
  }

  std::optional<T> query(int path_id, int u, int v) {
    push_delta(path_id, u += static_cast<int>(value[path_id].size()) / 2);
    push_delta(path_id, v += static_cast<int>(value[path_id].size()) / 2);
    std::optional<T> res;
    for (; u <= v; u = (u + 1) / 2, v = (v - 1) / 2) {
      if ((u & 1) != 0) {
        T part_value = applied_value(path_id, u);
        res = res ? combine(*res, part_value) : part_value;
      }
      if ((v & 1) == 0) {
        T part_value = applied_value(path_id, v);
        res = res ? combine(*res, part_value) : part_value;
      }
    }
    return res;
  }

  void update(int path_id, int u, int v, const T &d) {
    push_delta(path_id, u += static_cast<int>(value[path_id].size()) / 2);
    push_delta(path_id, v += static_cast<int>(value[path_id].size()) / 2);
    int tu = -1, tv = -1;
    for (; u <= v; u = (u + 1) / 2, v = (v - 1) / 2) {
      if ((u & 1) != 0) {
        delta[path_id][u] = delta[path_id][u] ? compose_deltas(*delta[path_id][u], d) : d;
        if (tu == -1) {
          tu = u;
        }
      }
      if ((v & 1) == 0) {
        delta[path_id][v] = delta[path_id][v] ? compose_deltas(*delta[path_id][v], d) : d;
        if (tv == -1) {
          tv = v;
        }
      }
    }
    for (int i = tu; i > 1; i /= 2) {
      value[path_id][i / 2] = combine(applied_value(path_id, i), applied_value(path_id, i ^ 1));
    }
    for (int i = tv; i > 1; i /= 2) {
      value[path_id][i / 2] = combine(applied_value(path_id, i), applied_value(path_id, i ^ 1));
    }
  }

  inline bool is_ancestor(int ancestor, int node) {
    return (tin[ancestor] <= tin[node]) && (tout[node] <= tout[ancestor]);
  }

 public:
  explicit HeavyLight(const std::vector<std::vector<int>> &adj, const T &v = T{})
      : adj(adj),
        size(adj.size()),
        parent(adj.size()),
        root(adj.size(), -1),
        tin(adj.size()),
        tout(adj.size()),
        path(adj.size()),
        pathlen(adj.size()),
        pathpos(adj.size()),
        pathroot(adj.size()),
        counter(0),
        paths(0) {
    for (int u = 0; u < static_cast<int>(adj.size()); u++) {
      if (root[u] == -1) {
        dfs(u, -1, u);
        build_paths(u, new_path(u));
      }
    }
    value.resize(paths);
    delta.resize(paths);
    len.resize(paths);
    for (int i = 0; i < paths; i++) {
      int m = pathlen[i];
      value[i].assign(2 * m, v);
      delta[i].resize(2 * m);
      len[i].assign(2 * m, 1);
      for (int j = 2 * m - 1; j > 1; j -= 2) {
        value[i][j / 2] = combine(value[i][j], value[i][j ^ 1]);
        len[i][j / 2] = len[i][j] + len[i][j ^ 1];
      }
    }
  }

  template<typename Fn>
  bool for_each_path(int u, int v, bool include_lca, Fn f) {
    assert(0 <= u && u < static_cast<int>(root.size()));
    assert(0 <= v && v < static_cast<int>(root.size()));
    if (root[u] != root[v]) {
      return false;
    }
    int path_root;
    while (!is_ancestor(path_root = pathroot[path[u]], v)) {
      f(path[u], 0, pathpos[u], true);
      u = parent[path_root];
    }
    std::vector<std::tuple<int, int, int>> down_parts;
    while (!is_ancestor(path_root = pathroot[path[v]], u)) {
      down_parts.emplace_back(path[v], 0, pathpos[v]);
      v = parent[path_root];
    }
    int lo = std::min(pathpos[u], pathpos[v]) + static_cast<int>(!include_lca);
    int hi = std::max(pathpos[u], pathpos[v]);
    if (lo <= hi) {
      f(path[u], lo, hi, pathpos[u] > pathpos[v]);
    }
    for (int i = static_cast<int>(down_parts.size()) - 1; i >= 0; i--) {
      auto [down_path, down_lo, down_hi] = down_parts[i];
      f(down_path, down_lo, down_hi, false);
    }
    return true;
  }

  T query(int u, int v) {
    assert(0 <= u && u < static_cast<int>(root.size()));
    assert(0 <= v && v < static_cast<int>(root.size()));
    assert(root[u] == root[v]);
    assert(!VALUES_ON_EDGES || u != v);
    std::optional<T> res;
    for_each_path(u, v, !VALUES_ON_EDGES, [&](int path_id, int lo, int hi, bool) {
      if (auto part = query(path_id, lo, hi)) {
        res = res ? combine(*res, *part) : *part;
      }
    });
    assert(res);
    return *res;
  }

  void update(int u, int v, const T &d) {
    assert(0 <= u && u < static_cast<int>(root.size()));
    assert(0 <= v && v < static_cast<int>(root.size()));
    assert(root[u] == root[v]);
    if (VALUES_ON_EDGES && u == v) {
      return;
    }
    for_each_path(u, v, !VALUES_ON_EDGES, [&](int path_id, int lo, int hi, bool) {
      update(path_id, lo, hi, d);
    });
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  //   v=40   v=20   v=10
  // 0------1------2------3    5------6
  //               |
  //               +------4
  //                 v=30
  vector<vector<int>> adj(7);
  auto add_edge = [&](int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  };
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(2, 3);
  add_edge(2, 4);
  add_edge(5, 6);
  HeavyLight<int> hld(adj, 0);
  hld.update(0, 1, 40);
  hld.update(1, 2, 20);
  hld.update(2, 3, 10);
  hld.update(2, 4, 30);
  assert(hld.query(0, 3) == 10);
  assert(hld.query(2, 4) == 30);

  // Update path 3-2-4 to value 5:
  //
  //   v=40   v=20    v=5
  // 0------1------2------3    5------6
  //               |
  //               +------4
  //                  v=5
  hld.update(3, 4, 5);
  assert(hld.query(1, 4) == 5);

  // The disconnected component can be updated independently:
  //
  // 5------6
  //   v=7
  hld.update(5, 6, 7);
  assert(hld.query(5, 6) == 7);
  assert(!hld.for_each_path(0, 5, true, [](int, int, int, bool) {}));

  // v=100  v=100  v=100  v=100    v=100  v=100
  //   0------1------2------3        5------6
  //                 |
  //                 +------4
  //                      v=100
  HeavyLight<int, false> node_hld(adj, 100);

  // Update nodes on path 1-2-4 to value 8:
  //
  // v=100   v=8    v=8   v=100    v=100  v=100
  //   0------1------2------3        5------6
  //                 |
  //                 +------4
  //                       v=8
  node_hld.update(1, 4, 8);
  assert(node_hld.query(0, 3) == 8);    // Min over nodes 0, 1, 2, 3.
  assert(node_hld.query(5, 5) == 100);  // Single-node paths are allowed.
  node_hld.update(5, 5, 7);
  assert(node_hld.query(5, 5) == 7);
  return 0;
}
