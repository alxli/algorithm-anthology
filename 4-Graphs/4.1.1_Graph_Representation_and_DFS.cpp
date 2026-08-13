/*

Stores a directed or undirected simple graph as adjacency lists. The list for node `u` contains its
out-neighbors; an undirected edge is stored in both endpoints' lists. Nodes are nonnegative integer
indices in $[0, `size()`)$, and `add_edge()` grows the graph when either endpoint is new. The class
also provides depth-first search, cycle detection, and forest/DAG checks.

A simple graph must not have self-loops or parallel edges, although the class does not enforce these
restrictions. In particular, undirected `has_cycle()` cannot distinguish a second edge to the parent
from the tree edge.

- `Graph(directed = true)` constructs an empty graph, directed if `directed` is true and undirected
  otherwise.
- `size()` returns the current number of nodes.
- `operator[u]` returns a mutable or const reference to the adjacency list (`std::vector<int>`) of
  node `u`.
- `add_edge(u, v)` adds an edge from `u` to `v`, plus the reverse edge if the graph is undirected,
  growing the node count to accommodate the larger index if necessary.
- `dfs(start, f)` runs a depth-first search from node `start`, calling function `f` on each node in
  the order it is first visited.
- `has_cycle()` returns whether the graph contains a cycle.
- `is_directed()` returns whether the graph is directed.
- `is_forest()` returns whether the graph is undirected and acyclic.
- `is_dag()` returns whether the graph is directed and acyclic.

Time Complexity:
- O(1) amortized per call to `add_edge()` when no resize is required; up to O(n) for a call that
  grows the graph to $n$ nodes.
- O(max(n, m)) per call for `dfs()`, `has_cycle()`, `is_forest()`, or `is_dag()`, where $n$ is the
  number of nodes and $m$ is the number of edges.
- O(1) per call to all other public member functions.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary stack space for `dfs()`, `has_cycle()`, `is_forest()`, and `is_dag()`.
- O(1) auxiliary for all other public member functions.

*/

#include <algorithm>
#include <vector>

class Graph {
  std::vector<std::vector<int>> adj;
  bool directed;

 public:
  explicit Graph(bool directed = true) : directed(directed) {}

  int size() const { return static_cast<int>(adj.size()); }
  std::vector<int> &operator[](int u) { return adj[u]; }
  const std::vector<int> &operator[](int u) const { return adj[u]; }

  void add_edge(int u, int v) {
    int n = static_cast<int>(adj.size());
    if (u >= n || v >= n) {
      adj.resize(std::max(u, v) + 1);
    }
    adj[u].push_back(v);
    if (!directed) {
      adj[v].push_back(u);
    }
  }

  template<typename Fn>
  void dfs(int start, Fn f) const {
    std::vector<char> visit(adj.size());
    auto dfs = [&](auto &&dfs, int u) -> void {
      f(u);
      visit[u] = true;
      for (int v : adj[u]) {
        if (!visit[v]) {
          dfs(dfs, v);
        }
      }
    };
    dfs(dfs, start);
  }

  bool has_cycle() const {
    int n = static_cast<int>(adj.size());
    std::vector<char> visit(n), onstack(n);
    auto dfs = [&](auto &&dfs, int u, int p) -> bool {
      visit[u] = true;
      onstack[u] = true;
      for (int v : adj[u]) {
        if ((directed && onstack[v]) ||           // back edge in directed graph
            (!directed && visit[v] && v != p) ||  // back edge in undirected graph
            (!visit[v] && dfs(dfs, v, u))) {      // tree edge to unvisited node
          return true;
        }
      }
      onstack[u] = false;
      return false;
    };
    for (int i = 0; i < n; i++) {
      if (!visit[i] && dfs(dfs, i, -1)) {
        return true;
      }
    }
    return false;
  }

  bool is_directed() const { return directed; }
  bool is_forest() const { return !directed && !has_cycle(); }
  bool is_dag() const { return directed && !has_cycle(); }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  {
    //           0
    //       /  /  |
    //      v  v   v
    //      1  6   7
    //    / |     / |
    //   v  v    v  v
    //   2  5    8  11
    //  / |     /|
    // v  v    v v
    // 3  4    9 10
    Graph g;
    g.add_edge(0, 1);
    g.add_edge(0, 6);
    g.add_edge(0, 7);
    g.add_edge(1, 2);
    g.add_edge(1, 5);
    g.add_edge(2, 3);
    g.add_edge(2, 4);
    g.add_edge(7, 8);
    g.add_edge(7, 11);
    g.add_edge(8, 9);
    g.add_edge(8, 10);
    vector<int> order;
    g.dfs(0, [&](int u) { order.push_back(u); });
    assert((order == vector<int>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11}));
    assert(g.size() == 12 && g[0].size() == 3);
    assert(g.is_dag());
    assert(!g.has_cycle());
  }
  {
    //       0
    //     /  |
    //    1   2
    //  / |
    // 3  4
    Graph tree(false);
    tree.add_edge(0, 1);
    tree.add_edge(0, 2);
    tree.add_edge(1, 3);
    tree.add_edge(1, 4);
    assert(tree.is_forest());
    assert(!tree.is_dag());
    tree.add_edge(2, 3);
    assert(!tree.is_forest());
  }
  return 0;
}
