/*

Maintain a tree with values associated with either its edges or nodes, while
supporting both dynamic queries and dynamic updates of all values on a given
path between two nodes in the tree. Heavy-light decomposition partitions the
nodes of the tree into disjoint paths where all nodes have degree two, except
the endpoints of a path which has degree one.

The query operation is defined by an associative join_values() function which
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the tree. The default code below assumes a
numerical tree type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case the join_values()
function should be defined to return "a + b".

The update operation is defined by the join_value_with_delta() and join_deltas()
functions, which determines the change made to values. These must satisfy:
- join_deltas(d1, join_deltas(d2, d3)) = join_deltas(join_deltas(d1, d2), d3).
- join_value_with_delta(join_values(v, ...(m times)..., v), d, m)) should be
  equal to join_values(join_value_with_delta(v, d, 1), ...(m times)).
- if a sequence d_1, ..., d_m of deltas is used to update a value v, then
  join_value_with_delta(v, join_deltas(d_1, ..., d_m), 1) should be equivalent
  to m sequential calls to join_value_with_delta(v, d_i, 1) for i = 1..m.
The default code below defines updates that "set" a path's edges or nodes to a
new value. Another possible update operation is "increment", in which case
join_value_with_delta(v, d, len) should be defined to return "v + d*len" and
join_deltas(d1, d2) should be defined to return "d1 + d2".

- heavy_light(n, adj[], v) constructs a new heavy light decomposition on a tree
  with n nodes defined by the adjacency list adj[], with all values initialized
  to v. The adjacency list must be a size n array of vectors consisting of only
  the integers from 0 to n - 1, inclusive. No duplicate edges should exist, and
  the graph must be connected.
- query(u, v) returns the result of join_values() applied to all values on the
  path from node u to node v.
- update(u, v, d) modifies all values on the path from node u to node v by
  respectively joining them with d using join_value_with_delta().

Time Complexity:
- O(n) per call to the constructor, where n is the number of nodes.
- O(log n) per call to query() and update();

Space Complexity:
- O(n) for storage of the decomposition.
- O(n) auxiliary stack space for the constructor.
- O(1) auxiliary for query() and update().

*/

#include <algorithm>
#include <stdexcept>
#include <vector>

template<class T>
class heavy_light {
  // Set this to true to store values on edges, false to store values on nodes.
  static const bool VALUES_ON_EDGES = true;

  static T join_values(const T &a, const T &b) {
    return std::min(a, b);
  }

  static T join_value_with_delta(const T &v, const T &d, int len) {
    return d;
  }

  static T join_deltas(const T &d1, const T &d2) {
    return d2;  // For "set" updates, the more recent delta prevails.
  }

  int counter, paths;
  std::vector<std::vector<T> > value, delta;
  std::vector<std::vector<bool> > pending;
  std::vector<std::vector<int> > len;
  std::vector<int> size, parent, tin, tout, path, pathlen, pathpos, pathroot;
  std::vector<int> *adj;

  void dfs(int u, int p) {
    tin[u] = counter++;
    parent[u] = p;
    size[u] = 1;
    for (int j = 0; j < (int)adj[u].size(); j++) {
      int v = adj[u][j];
      if (v != p) {
        dfs(v, u);
        size[u] += size[v];
      }
    }
    tout[u] = counter++;
  }

  int new_path(int u) {
    pathroot[paths] = u;
    return paths++;
  }

  void build_paths(int u, int path) {
    this->path[u] = path;
    pathpos[u] = pathlen[path]++;
    for (int j = 0; j < (int)adj[u].size(); j++) {
      int v = adj[u][j];
      if (v != parent[u]) {
        build_paths(v, (2*size[v] >= size[u]) ? path : new_path(v));
      }
    }
  }

  inline T join_value_with_delta(int path, int i) {
    return pending[path][i]
        ? join_value_with_delta(value[path][i], delta[path][i], len[path][i])
        : value[path][i];
  }

  void push_delta(int path, int i) {
    int d = 0;
    while ((i >> d) > 0) {
      d++;
    }
    for (d -= 2; d >= 0; d--) {
      int l = (i >> d), r = (l ^ 1), n = l/2;
      if (pending[path][n]) {
        value[path][n] = join_value_with_delta(path, n);
        delta[path][l] =
            pending[path][l] ? join_deltas(delta[path][l], delta[path][n])
                             : delta[path][n];
        delta[path][r] =
            pending[path][r] ? join_deltas(delta[path][r], delta[path][n])
                             : delta[path][n];
        pending[path][l] = pending[path][r] = true;
        pending[path][n] = false;
      }
    }
  }

  bool query(int path, int u, int v, T *res) {
    push_delta(path, u += value[path].size()/2);
    push_delta(path, v += value[path].size()/2);
    bool found = false;
    for (; u <= v; u = (u + 1)/2, v = (v - 1)/2) {
      if ((u & 1) != 0) {
        T value = join_value_with_delta(path, u);
        *res = found ? join_values(*res, value) : value;
        found = true;
      }
      if ((v & 1) == 0) {
        T value = join_value_with_delta(path, v);
        *res = found ? join_values(*res, value) : value;
        found = true;
      }
    }
    return found;
  }

  void update(int path, int u, int v, const T &d) {
    push_delta(path, u += value[path].size()/2);
    push_delta(path, v += value[path].size()/2);
    int tu = -1, tv = -1;
    for (; u <= v; u = (u + 1)/2, v = (v - 1)/2) {
      if ((u & 1) != 0) {
        delta[path][u] = pending[path][u] ? join_deltas(delta[path][u], d) : d;
        pending[path][u] = true;
        if (tu == -1) {
          tu = u;
        }
      }
      if ((v & 1) == 0) {
        delta[path][v] = pending[path][v] ? join_deltas(delta[path][v], d) : d;
        pending[path][v] = true;
        if (tv == -1) {
          tv = v;
        }
      }
    }
    for (int i = tu; i > 1; i /= 2) {
      value[path][i/2] = join_values(join_value_with_delta(path, i),
                                     join_value_with_delta(path, i ^ 1));
    }
    for (int i = tv; i > 1; i /= 2) {
      value[path][i/2] = join_values(join_value_with_delta(path, i),
                                     join_value_with_delta(path, i ^ 1));
    }
  }

  inline bool is_ancestor(int parent, int child) {
    return (tin[parent] <= tin[child]) && (tout[child] <= tout[parent]);
  }

 public:
  heavy_light(int n, std::vector<int> adj[], const T &v = T())
      : counter(0), paths(0), size(n), parent(n), tin(n), tout(n), path(n),
        pathlen(n), pathpos(n), pathroot(n), adj(adj) {
    dfs(0, -1);
    build_paths(0, new_path(0));
    value.resize(paths);
    delta.resize(paths);
    pending.resize(paths);
    len.resize(paths);
    for (int i = 0; i < paths; i++) {
      int m = pathlen[i];
      value[i].assign(2*m, v);
      delta[i].resize(2*m);
      pending[i].assign(2*m, false);
      len[i].assign(2*m, 1);
      for (int j = 2*m - 1; j > 1; j -= 2) {
        value[i][j/2] = join_values(value[i][j], value[i][j ^ 1]);
        len[i][j/2] = len[i][j] + len[i][j ^ 1];
      }
    }
  }

  T query(int u, int v) {
    if (VALUES_ON_EDGES && u == v) {
      throw std::runtime_error("No edge between u and v to be queried.");
    }
    bool found = false;
    T res = T(), value;
    int root;
    while (!is_ancestor(root = pathroot[path[u]], v)) {
      if (query(path[u], 0, pathpos[u], &value)) {
        res = found ? join_values(res, value) : value;
        found = true;
      }
      u = parent[root];
    }
    while (!is_ancestor(root = pathroot[path[v]], u)) {
      if (query(path[v], 0, pathpos[v], &value)) {
        res = found ? join_values(res, value) : value;
        found = true;
      }
      v = parent[root];
    }
    if (query(path[u], std::min(pathpos[u], pathpos[v]) + (int)VALUES_ON_EDGES,
              std::max(pathpos[u], pathpos[v]), &value)) {
      res = found ? join_values(res, value) : value;
      found = true;
    }
    if (!found) {
      throw std::runtime_error("Unexpected error: No values found.");
    }
    return res;
  }

  void update(int u, int v, const T &d) {
    if (VALUES_ON_EDGES && u == v) {
      return;
    }
    int root;
    while (!is_ancestor(root = pathroot[path[u]], v)) {
      update(path[u], 0, pathpos[u], d);
      u = parent[root];
    }
    while (!is_ancestor(root = pathroot[path[v]], u)) {
      update(path[v], 0, pathpos[v], d);
      v = parent[root];
    }
    update(path[u], std::min(pathpos[u], pathpos[v]) + (int)VALUES_ON_EDGES,
           std::max(pathpos[u], pathpos[v]), d);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //     w=40      w=20      w=10
  // 0---------1---------2---------3
  //                     |
  //                     ----------4
  //                         w=30
  vector<int> adj[5];
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[2].push_back(3);
  adj[3].push_back(2);
  adj[2].push_back(4);
  adj[4].push_back(2);
  heavy_light<int> hld(5, adj, 0);
  hld.update(0, 1, 40);
  hld.update(1, 2, 20);
  hld.update(2, 3, 10);
  hld.update(2, 4, 30);
  assert(hld.query(0, 3) == 10);
  assert(hld.query(2, 4) == 30);
  hld.update(3, 4, 5);
  assert(hld.query(1, 4) == 5);
  return 0;
}
