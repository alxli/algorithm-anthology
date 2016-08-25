/*

A graph consists of a set of objects (a.k.a vertices, or nodes) and a set of
connections (a.k.a. edges) between pairs of said objects. A graph may be stored
as an adjacency list, which is a space efficient representation that is also
time-efficient for traversals.

The following class implements a simple graph using adjacency lists, along with
depth-first search and a few applications. The constructor takes a boolean
argument which specifies whether the instance is a directed or undirected graph.
The nodes of the graph are identified by integers indices numbered consecutively
starting from 0. The total number nodes will automatically increase based upon
the maximum argument passed to add_edge() so far.

Time Complexity:
- O(1) amortized per call to add_edge(), or O(max(n, m)) for n calls where the
  maximum node index passed as an argument is m.
- O(max(n, m)) per call for dfs(), has_cycle(), is_tree(), or is_dag(), where n
  and m are the number of nodes and edges respectively.
- O(1) per call to all other public member functions.

Space Complexity:
- O(max(n, m)) on the number of nodes and edges for storage of the graph.
- O(n) auxiliary per call to dfs(), has_cycle(), is_tree(), and is_dag(), where
  n is the number of nodes.
- O(1) auxiliary per call to all other public member functions.

*/

#include <algorithm>  // std::max()
#include <vector>

class graph {
  std::vector<std::vector<int> > adj;
  bool _is_directed;

  template<class Action>
  void dfs(int n, std::vector<bool> &vis, Action act) const {
    act(n);
    vis[n] = true;
    std::vector<int>::const_iterator it;
    for (it = adj[n].begin(); it != adj[n].end(); ++it) {
      if (!vis[*it])
        dfs(*it, vis, act);
    }
  }

  bool has_cycle(int n, int prev, std::vector<bool> &vis,
                 std::vector<bool> &onstack) const {
    vis[n] = true;
    onstack[n] = true;
    std::vector<int>::const_iterator it;
    for (it = adj[n].begin(); it != adj[n].end(); ++it) {
      if (is_directed() && onstack[*it])
        return true;
      if (!is_directed() && vis[*it] && *it != prev)
        return true;
      if (!vis[*it] && has_cycle(*it, n, vis, onstack))
        return true;
    }
    onstack[n] = false;
    return false;
  }

 public:
  graph(bool is_directed = true) {
    this->_is_directed = is_directed;
  }

  int nodes() const {
    return (int)adj.size();
  }

  std::vector<int>& operator[](int n) {
    return adj[n];
  }

  void add_edge(int u, int v) {
    if (u >= (int)adj.size() || v >= (int)adj.size()) {
      adj.resize(std::max(u, v) + 1);
    }
    adj[u].push_back(v);
    if (!is_directed()) {
      adj[v].push_back(u);
    }
  }

  bool is_directed() const {
    return _is_directed;
  }

  bool has_cycle() const {
    std::vector<bool> vis(adj.size(), false);
    std::vector<bool> onstack(adj.size(), false);
    for (int i = 0; i < (int)adj.size(); i++) {
      if (!vis[i] && has_cycle(i, -1, vis, onstack))
        return true;
    }
    return false;
  }

  bool is_tree() const {
    return !is_directed() && !has_cycle();
  }

  bool is_dag() const {
    return is_directed() && !has_cycle();
  }

  template<class Action>
  void dfs(int start, Action act) const {
    std::vector<bool> vis(adj.size(), false);
    dfs(start, vis, act);
  }
};

/*** Example Usage and Output:

DFS order: 0 1 2 3 4 5 6 7 8 9 10 11

***/

#include <cassert>
#include <iostream>
using namespace std;

void print_node(int n) {
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
    g.dfs(0, print_node);
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
