/*

A graph consists of a set of objects (a.k.a vertices, or nodes) and a set of
connections (a.k.a. edges) between pairs of said objects. A graph may be stored
as an adjacency list, which is a space efficient representation that is also
time-efficient for traversals.

The following class implements a simple graph using adjacency lists, along with
depth-first search and a few other applications. The constructor takes a Boolean
argument which specifies whether the instance is a directed or undirected graph.
The nodes of the graph are identified by integers indices numbered consecutively
starting from 0. The total number of nodes will automatically increase based on
the maximum node index passed to add_edge() so far.

Time Complexity:
- O(1) amortized per call to add_edge(), or O(max(n, m)) for n calls where the
  maximum node index passed as an argument is m.
- O(max(n, m)) per call for dfs(), has_cycle(), is_tree(), or is_dag(), where n
  is the number of nodes and and m is the number of edges.
- O(1) per call to all other public member functions.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n is the number of nodes and m
  is the number of edges.
- O(n) auxiliary stack space for dfs(), has_cycle(), is_tree(), and is_dag().
- O(1) auxiliary for all other public member functions.

*/

#include <algorithm>
#include <vector>

class graph {
  std::vector<std::vector<int> > adj;
  bool directed;

  template<class ReportFunction>
  void dfs(int n, std::vector<bool> &visit, ReportFunction f) const {
    f(n);
    visit[n] = true;
    std::vector<int>::const_iterator it;
    for (it = adj[n].begin(); it != adj[n].end(); ++it) {
      if (!visit[*it]) {
        dfs(*it, visit, f);
      }
    }
  }

  bool has_cycle(int n, int prev, std::vector<bool> &visit,
                 std::vector<bool> &onstack) const {
    visit[n] = true;
    onstack[n] = true;
    std::vector<int>::const_iterator it;
    for (it = adj[n].begin(); it != adj[n].end(); ++it) {
      if (directed && onstack[*it]) {
        return true;
      }
      if (!directed && visit[*it] && *it != prev) {
        return true;
      }
      if (!visit[*it] && has_cycle(*it, n, visit, onstack)) {
        return true;
      }
    }
    onstack[n] = false;
    return false;
  }

 public:
  graph(bool directed = true) : directed(directed) {}

  int nodes() const {
    return (int)adj.size();
  }

  std::vector<int>& operator[](int n) {
    return adj[n];
  }

  void add_edge(int u, int v) {
    int n = adj.size();
    if (u >= n || v >= n) {
      adj.resize(std::max(u, v) + 1);
    }
    adj[u].push_back(v);
    if (!directed) {
      adj[v].push_back(u);
    }
  }

  bool is_directed() const {
    return directed;
  }

  bool has_cycle() const {
    int n = adj.size();
    std::vector<bool> visit(n, false), onstack(n, false);
    for (int i = 0; i < n; i++) {
      if (!visit[i] && has_cycle(i, -1, visit, onstack)) {
        return true;
      }
    }
    return false;
  }

  bool is_tree() const {
    return !directed && !has_cycle();
  }

  bool is_dag() const {
    return directed && !has_cycle();
  }

  template<class ReportFunction>
  void dfs(int start, ReportFunction f) const {
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

void print(int n) {
  cout << n << " ";
}

int main() {
  {
    graph g;
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
    g.dfs(0, print);
    cout << endl;
    assert(g[0].size() == 3);
    assert(g.is_dag());
    assert(!g.has_cycle());
  }
  {
    graph tree(false);
    tree.add_edge(0, 1);
    tree.add_edge(0, 2);
    tree.add_edge(1, 3);
    tree.add_edge(1, 4);
    assert(tree.is_tree());
    assert(!tree.is_dag());
    tree.add_edge(2, 3);
    assert(!tree.is_tree());
  }
  return 0;
}
