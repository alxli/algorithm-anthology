/*

Maintain the number of bridges in an undirected graph while edges are inserted one at a time, along
with the 2-edge-connected components and ordinary connected components. When an inserted edge joins
two different connected components, it creates a new bridge. When it joins two nodes already in the
same connected component, it creates a cycle and every bridge on the path between the two endpoints
stops being a bridge.

- `OnlineBridges(n)` constructs a graph with `n` isolated nodes numbered $[0, `n`)$.
- `add_edge(u, v)` adds the undirected edge `u`-`v` and updates: `bridges` with the current number
  of bridges, and the disjoint-set parent arrays `dsu_2ecc[]` and `dsu_cc[]`, which partition the
  nodes into 2-edge-connected components and ordinary connected components respectively.
- `find_2ecc(u)` returns node `u`'s representative in the 2-edge-connected components partition.
- `find_cc(u)` returns node `u`'s representative in the ordinary connected components partition.

Parallel edges are supported. Adding a second edge between two nodes that are already in the same
connected component dissolves every bridge on the path between them, merging those nodes into the
same 2-edge-connected component.

Time Complexity:
- O(log n) amortized per call to `add_edge()` in this implementation, where $n$ is the number of
  nodes.

Space Complexity:
- O(n) for the disjoint-set arrays and auxiliary parent links.

*/

#include <algorithm>
#include <vector>

struct OnlineBridges {
  std::vector<int> dsu_2ecc, dsu_cc, dsu_cc_size, parent, last_visit;
  int lca_iteration, bridges;

  OnlineBridges(int n = 0) {
    dsu_2ecc.resize(n);
    dsu_cc.resize(n);
    dsu_cc_size.assign(n, 1);
    parent.assign(n, -1);
    last_visit.assign(n, 0);
    lca_iteration = 0;
    bridges = 0;
    for (int i = 0; i < n; i++) {
      dsu_2ecc[i] = dsu_cc[i] = i;
    }
  }

  int find_2ecc(int u) {
    if (u == -1) {
      return -1;
    }
    if (dsu_2ecc[u] == u) {
      return u;
    }
    return dsu_2ecc[u] = find_2ecc(dsu_2ecc[u]);
  }

  int find_cc(int u) {
    u = find_2ecc(u);
    if (dsu_cc[u] == u) {
      return u;
    }
    return dsu_cc[u] = find_cc(dsu_cc[u]);
  }

  void make_root(int u) {
    u = find_2ecc(u);
    int root = u, child = -1;
    while (u != -1) {
      int p = find_2ecc(parent[u]);
      parent[u] = child;
      dsu_cc[u] = root;
      child = u;
      u = p;
    }
    dsu_cc_size[root] = dsu_cc_size[child];
  }

  void merge_path(int u, int v) {
    lca_iteration++;
    std::vector<int> path_a, path_b;
    int lca = -1;
    while (lca == -1) {
      if (u != -1) {
        u = find_2ecc(u);
        path_a.push_back(u);
        if (last_visit[u] == lca_iteration) {
          lca = u;
          break;
        }
        last_visit[u] = lca_iteration;
        u = parent[u];
      }
      if (v != -1) {
        v = find_2ecc(v);
        path_b.push_back(v);
        if (last_visit[v] == lca_iteration) {
          lca = v;
          break;
        }
        last_visit[v] = lca_iteration;
        v = parent[v];
      }
    }
    for (int a : path_a) {
      dsu_2ecc[a] = lca;
      if (a == lca) {
        break;
      }
      bridges--;
    }
    for (int b : path_b) {
      dsu_2ecc[b] = lca;
      if (b == lca) {
        break;
      }
      bridges--;
    }
  }

  void add_edge(int u, int v) {
    u = find_2ecc(u);
    v = find_2ecc(v);
    if (u == v) {
      return;
    }
    int ca = find_cc(u), cb = find_cc(v);
    if (ca != cb) {
      bridges++;
      if (dsu_cc_size[ca] > dsu_cc_size[cb]) {
        std::swap(u, v);
        std::swap(ca, cb);
      }
      make_root(u);
      parent[u] = v;
      dsu_cc[u] = v;
      dsu_cc_size[cb] += dsu_cc_size[u];
    } else {
      merge_path(u, v);
    }
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  OnlineBridges g(4);
  g.add_edge(0, 1);
  assert(g.bridges == 1);
  g.add_edge(1, 2);
  assert(g.bridges == 2);
  g.add_edge(2, 0);
  assert(g.bridges == 0);
  g.add_edge(2, 3);
  assert(g.bridges == 1);
  return 0;
}
