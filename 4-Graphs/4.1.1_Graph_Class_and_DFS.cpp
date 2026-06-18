/*

A graph consists of a set of objects (a.k.a. vertices, or nodes) and a set of connections (a.k.a.
edges) between pairs of said objects. A graph may be stored as an adjacency list, which is a space
efficient representation that is also time-efficient for traversals.

The following class implements a simple graph using adjacency lists, along with depth-first search
and a few other applications. The constructor takes a flag which specifies whether the instance is a
directed or undirected graph. The nodes of the graph are identified by integer indices numbered
consecutively starting from 0. The total number of nodes automatically increases based on the
maximum node index passed to `add_edge()` so far.

- `Graph(directed)` constructs an empty graph, directed if `directed` is true (the default) and
  undirected otherwise.
- `nodes()` returns the current number of nodes.
- `operator[n]` returns a reference to the adjacency list (a `std::vector<int>`) of node `n`.
- `add_edge(u, v)` adds an edge from `u` to `v`, plus the reverse edge if the graph is undirected,
  growing the node count to accommodate the larger index if necessary.
- `dfs(start, f)` runs a depth-first search from node `start`, calling `f(n)` on each node `n` in
  the order it is first visited.
- `has_cycle()` returns whether the graph contains a cycle.
- `is_directed()` returns whether the graph is directed.
- `is_forest()` returns whether the graph is undirected and acyclic.
- `is_dag()` returns whether the graph is directed and acyclic.

For undirected graphs, `has_cycle()` assumes a simple graph; parallel edges are not distinguished
from the tree edge back to the parent.

Time Complexity:
- O(1) amortized per call to `add_edge()`, or O(max(n, m)) for $n$ calls where the maximum node
  index passed as an argument is $m$.
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

  template<typename Fn>
  void dfs(int n, std::vector<bool> &visit, Fn f) const {
    f(n);
    visit[n] = true;
    for (int v : adj[n]) {
      if (!visit[v]) {
        dfs(v, visit, f);
      }
    }
  }

  bool has_cycle(int n, int prev, std::vector<bool> &visit, std::vector<bool> &onstack) const {
    visit[n] = true;
    onstack[n] = true;
    for (int v : adj[n]) {
      if (directed && onstack[v]) {
        return true;
      }
      if (!directed && visit[v] && v != prev) {
        return true;
      }
      if (!visit[v] && has_cycle(v, n, visit, onstack)) {
        return true;
      }
    }
    onstack[n] = false;
    return false;
  }

 public:
  Graph(bool directed = true) : directed(directed) {}

  int nodes() const { return static_cast<int>(adj.size()); }
  std::vector<int> &operator[](int n) { return adj[n]; }

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

  bool has_cycle() const {
    int n = static_cast<int>(adj.size());
    std::vector<bool> visit(n, false), onstack(n, false);
    for (int i = 0; i < n; i++) {
      if (!visit[i] && has_cycle(i, -1, visit, onstack)) {
        return true;
      }
    }
    return false;
  }

  bool is_directed() const { return directed; }
  bool is_forest() const { return !directed && !has_cycle(); }
  bool is_dag() const { return directed && !has_cycle(); }

  template<typename Fn>
  void dfs(int start, Fn f) const {
    std::vector<bool> visit(adj.size(), false);
    dfs(start, visit, f);
  }
};

/*** Example Usage and Output:

DFS order: 0 1 2 3 4 5 6 7 8 9 10 11

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  {
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
    cout << "DFS order: ";
    g.dfs(0, [](int n) { cout << n << " "; });
    cout << endl;
    assert(g[0].size() == 3);
    assert(g.is_dag());
    assert(!g.has_cycle());
  }
  {
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
