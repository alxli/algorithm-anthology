/*

Maintain the number of bridges in an undirected graph while edges are inserted
one at a time. The graph starts with isolated nodes. After each call to
`add_edge()`, `bridges` stores the current number of bridges.

This data structure maintains two disjoint-set forests:

- `dsu_2ecc[]` tracks the current $2$-edge-connected components.
- `dsu_cc[]` tracks the ordinary connected components.

When an inserted edge joins two different connected components, it creates a new
bridge. When it joins two nodes already in the same connected component, it
creates a cycle and every bridge on the path between the two endpoints stops
being a bridge.

Time Complexity:
- O(log n) amortized per call to `add_edge()` in this implementation, where $n$
  is the number of nodes.

Space Complexity:
- O(n) for the disjoint-set arrays and auxiliary parent links.

*/

#include <algorithm>
#include <vector>

struct online_bridges {
  std::vector<int> dsu_2ecc, dsu_cc, dsu_cc_size, parent, last_visit;
  int lca_iteration, bridges;

  online_bridges(int nodes = 0) { init(nodes); }

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

  void merge_path(int a, int b) {
    lca_iteration++;
    std::vector<int> path_a, path_b;
    int lca = -1;
    while (lca == -1) {
      if (a != -1) {
        a = find_2ecc(a);
        path_a.push_back(a);
        if (last_visit[a] == lca_iteration) {
          lca = a;
          break;
        }
        last_visit[a] = lca_iteration;
        a = parent[a];
      }
      if (b != -1) {
        b = find_2ecc(b);
        path_b.push_back(b);
        if (last_visit[b] == lca_iteration) {
          lca = b;
          break;
        }
        last_visit[b] = lca_iteration;
        b = parent[b];
      }
    }
    for (int i = 0; i < (int)path_a.size(); i++) {
      dsu_2ecc[path_a[i]] = lca;
      if (path_a[i] == lca) {
        break;
      }
      bridges--;
    }
    for (int i = 0; i < (int)path_b.size(); i++) {
      dsu_2ecc[path_b[i]] = lca;
      if (path_b[i] == lca) {
        break;
      }
      bridges--;
    }
  }

  void add_edge(int a, int b) {
    a = find_2ecc(a);
    b = find_2ecc(b);
    if (a == b) {
      return;
    }
    int ca = find_cc(a), cb = find_cc(b);
    if (ca != cb) {
      bridges++;
      if (dsu_cc_size[ca] > dsu_cc_size[cb]) {
        std::swap(a, b);
        std::swap(ca, cb);
      }
      make_root(a);
      parent[a] = b;
      dsu_cc[a] = b;
      dsu_cc_size[cb] += dsu_cc_size[a];
    } else {
      merge_path(a, b);
    }
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  online_bridges graph(4);
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
