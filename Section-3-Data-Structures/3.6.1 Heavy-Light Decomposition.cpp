/*

Description: Given an undirected, connected graph that is a tree, the
heavy-light decomposition (HLD) on the graph is a partitioning of the
vertices into disjoint paths to later support dynamic modification and
querying of values on paths between pairs of vertices.
See: http://wcipeg.com/wiki/Heavy-light_decomposition
and: http://blog.anudeep2011.com/heavy-light-decomposition/
To support dynamic adding and removal of edges, see link/cut tree.

Note: The adjacency list tree[] that is passed to the constructor must
not be changed afterwards in order for modify() and query() to work.

Time Complexity: O(N) for the constructor and O(log N) in the worst
case for both modify() and query(), where N is the number of vertices.

Space Complexity: O(N) on the number of vertices in the tree.

*/

#include <algorithm> /* std::max(), std::min() */
#include <climits>   /* INT_MIN */
#include <vector>

template<class T> class heavy_light {
  //true if you want values on edges, false if you want values on vertices
  static const bool VALUES_ON_EDGES = true;

  //Modify the following 6 functions to implement your custom
  //operations on the tree. This implements the Add/Max operations.
  //Operations like Add/Sum, Set/Max can also be implemented.
  static inline T modify_op(const T & x, const T & y) {
    return x + y;
  }

  static inline T query_op(const T & x, const T & y) {
    return std::max(x, y);
  }

  static inline T delta_on_segment(const T & delta, int seglen) {
    if (delta == null_delta()) return null_delta();
    //Here you must write a fast equivalent of following slow code:
    //  T result = delta;
    //  for (int i = 1; i < seglen; i++) result = query_op(result, delta);
    //  return result;
    return delta;
  }

  static inline T init_value() { return 0; }
  static inline T null_delta() { return 0; }
  static inline T null_value() { return INT_MIN; }

  static inline T join_value_with_delta(const T & v, const T & delta) {
    return delta == null_delta() ? v : modify_op(v, delta);
  }

  static T join_deltas(const T & delta1, const T & delta2) {
    if (delta1 == null_delta()) return delta2;
    if (delta2 == null_delta()) return delta1;
    return modify_op(delta1, delta2);
  }

  int counter, paths;
  std::vector<int> *adj;
  std::vector<std::vector<T> > value, delta;
  std::vector<std::vector<int> > len;
  std::vector<int> size, parent, tin, tout;
  std::vector<int> path, pathlen, pathpos, pathroot;

  void precompute_dfs(int u, int p) {
    tin[u] = counter++;
    parent[u] = p;
    size[u] = 1;
    for (int j = 0, v; j < (int)adj[u].size(); j++) {
      if ((v = adj[u][j]) == p) continue;
      precompute_dfs(v, u);
      size[u] += size[v];
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
    for (int j = 0, v; j < (int)adj[u].size(); j++) {
      if ((v = adj[u][j]) == parent[u]) continue;
      build_paths(v, 2*size[v] >= size[u] ? path : new_path(v));
    }
  }

  inline T join_value_with_delta0(int path, int i) {
    return join_value_with_delta(value[path][i],
              delta_on_segment(delta[path][i], len[path][i]));
  }

  void push_delta(int path, int i) {
    int d = 0;
    while ((i >> d) > 0) d++;
    for (d -= 2; d >= 0; d--) {
      int x = i >> d;
      value[path][x >> 1] = join_value_with_delta0(path, x >> 1);
      delta[path][x] = join_deltas(delta[path][x], delta[path][x >> 1]);
      delta[path][x ^ 1] = join_deltas(delta[path][x ^ 1], delta[path][x >> 1]);
      delta[path][x >> 1] = null_delta();
    }
  }

  T query(int path, int a, int b) {
    push_delta(path, a += value[path].size() >> 1);
    push_delta(path, b += value[path].size() >> 1);
    T res = null_value();
    for (; a <= b; a = (a + 1) >> 1, b = (b - 1) >> 1) {
      if ((a & 1) != 0)
        res = query_op(res, join_value_with_delta0(path, a));
      if ((b & 1) == 0)
        res = query_op(res, join_value_with_delta0(path, b));
    }
    return res;
  }

  void modify(int path, int a, int b, const T & delta) {
    push_delta(path, a += value[path].size() >> 1);
    push_delta(path, b += value[path].size() >> 1);
    int ta = -1, tb = -1;
    for (; a <= b; a = (a + 1) >> 1, b = (b - 1) >> 1) {
      if ((a & 1) != 0) {
        this->delta[path][a] = join_deltas(this->delta[path][a], delta);
        if (ta == -1) ta = a;
      }
      if ((b & 1) == 0) {
        this->delta[path][b] = join_deltas(this->delta[path][b], delta);
        if (tb == -1) tb = b;
      }
    }
    for (int i = ta; i > 1; i >>= 1)
      value[path][i >> 1] = query_op(join_value_with_delta0(path, i),
                                     join_value_with_delta0(path, i ^ 1));
    for (int i = tb; i > 1; i >>= 1)
      value[path][i >> 1] = query_op(join_value_with_delta0(path, i),
                                     join_value_with_delta0(path, i ^ 1));
  }

  inline bool is_ancestor(int p, int ch) {
    return tin[p] <= tin[ch] && tout[ch] <= tout[p];
  }

 public:
  heavy_light(int N, std::vector<int> tree[]): size(N), parent(N),
   tin(N), tout(N), path(N), pathlen(N), pathpos(N), pathroot(N) {
    adj = tree;
    counter = paths = 0;
    precompute_dfs(0, -1);
    build_paths(0, new_path(0));
    value.resize(paths);
    delta.resize(paths);
    len.resize(paths);
    for (int i = 0; i < paths; i++) {
      int m = pathlen[i];
      value[i].assign(2*m, init_value());
      delta[i].assign(2*m, null_delta());
      len[i].assign(2*m, 1);
      for (int j = 2*m - 1; j > 1; j -= 2) {
        value[i][j >> 1] = query_op(value[i][j], value[i][j ^ 1]);
        len[i][j >> 1] = len[i][j] + len[i][j ^ 1];
      }
    }
  }

  T query(int a, int b) {
    T res = null_value();
    for (int root; !is_ancestor(root = pathroot[path[a]], b); a = parent[root])
      res = query_op(res, query(path[a], 0, pathpos[a]));
    for (int root; !is_ancestor(root = pathroot[path[b]], a); b = parent[root])
      res = query_op(res, query(path[b], 0, pathpos[b]));
    if (VALUES_ON_EDGES && a == b) return res;
    return query_op(res, query(path[a], std::min(pathpos[a], pathpos[b]) +
                            VALUES_ON_EDGES, std::max(pathpos[a], pathpos[b])));
  }

  void modify(int a, int b, const T & delta) {
    for (int root; !is_ancestor(root = pathroot[path[a]], b); a = parent[root])
      modify(path[a], 0, pathpos[a], delta);
    for (int root; !is_ancestor(root = pathroot[path[b]], a); b = parent[root])
      modify(path[b], 0, pathpos[b], delta);
    if (VALUES_ON_EDGES && a == b) return;
    modify(path[a], std::min(pathpos[a], pathpos[b]) + VALUES_ON_EDGES,
            std::max(pathpos[a], pathpos[b]), delta);
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

const int MAXN = 1000;
vector<int> adj[MAXN];

/*
     w=10      w=20      w=40
  0---------1---------2---------3
                      \
                       ---------4
                         w=30
*/
int main() {
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[2].push_back(3);
  adj[3].push_back(2);
  adj[2].push_back(4);
  adj[4].push_back(2);
  heavy_light<int> hld(5, adj);
  hld.modify(0, 1, 10);
  hld.modify(1, 2, 20);
  hld.modify(2, 3, 40);
  hld.modify(2, 4, 30);
  cout << hld.query(0, 3) << "\n"; //40
  cout << hld.query(2, 4) << "\n"; //30
  hld.modify(3, 4, 50); //w[every edge from 3 to 4] += 50
  cout << hld.query(1, 4) << "\n"; //80
  return 0;
}
