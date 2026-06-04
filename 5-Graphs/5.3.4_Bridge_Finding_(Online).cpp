/*

Maintain the number of bridges in an undirected graph while edges are inserted one at a time. The
graph starts with isolated nodes. After each call to `add_edge()`, `bridges` stores the current
number of bridges.

This data structure maintains two disjoint-set forests:

- `dsu_2ecc[]` tracks the current $2$-edge-connected components.
- `dsu_cc[]` tracks the ordinary connected components.

When an inserted edge joins two different connected components, it creates a new bridge. When it
joins two nodes already in the same connected component, it creates a cycle and every bridge on the
path between the two endpoints stops being a bridge.

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

  OnlineBridges(int nodes = 0) { init(nodes); }

  void init(int nodes) {
    dsu_2ecc.resize(nodes);
    dsu_cc.resize(nodes);
    dsu_cc_size.assign(nodes, 1);
    parent.assign(nodes, -1);
    last_visit.assign(nodes, 0);
    lca_iteration = 0;
    bridges = 0;
    for (int i = 0; i < nodes; i++) {
      dsu_2ecc[i] = dsu_cc[i] = i;
    }
  }

  int find_2ecc(int v) {
    if (v == -1) {
      return -1;
    }
    if (dsu_2ecc[v] == v) {
      return v;
    }
    return dsu_2ecc[v] = find_2ecc(dsu_2ecc[v]);
  }

  int find_cc(int v) {
    v = find_2ecc(v);
    if (dsu_cc[v] == v) {
      return v;
    }
    return dsu_cc[v] = find_cc(dsu_cc[v]);
  }

  void make_root(int v) {
    v = find_2ecc(v);
    int root = v, child = -1;
    while (v != -1) {
      int p = find_2ecc(parent[v]);
      parent[v] = child;
      dsu_cc[v] = root;
      child = v;
      v = p;
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
    for (int v : path_a) {
      dsu_2ecc[v] = lca;
      if (v == lca) {
        break;
      }
      bridges--;
    }
    for (int v : path_b) {
      dsu_2ecc[v] = lca;
      if (v == lca) {
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
  OnlineBridges graph(4);
  graph.add_edge(0, 1);
  assert(graph.bridges == 1);
  graph.add_edge(1, 2);
  assert(graph.bridges == 2);
  graph.add_edge(2, 0);
  assert(graph.bridges == 0);
  graph.add_edge(2, 3);
  assert(graph.bridges == 1);
  return 0;
}
