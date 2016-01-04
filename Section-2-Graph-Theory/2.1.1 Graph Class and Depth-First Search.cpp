/*

A graph can be represented as a set of objects (a.k.a. vertices, or
nodes) and connections (a.k.a. edges) between pairs of objects. It can
also be stored as an adjacency matrix or adjacency list, the latter of
which is more space efficient but less time efficient for particular
operations such as checking whether a connection exists. A fundamental
task to perform on graphs is traversal, where all reachable vertices
are visited and actions are performed. Given any arbitrary starting
node, depth-first search (DFS) recursively explores each "branch" from
the current node as deep as possible before backtracking and following
other branches. Depth-first search has many applications, including
detecting cycles and solving generic puzzles.

The following implements a simple graph class using adjacency lists,
along with with depth-first search and a few applications. The nodes of
the graph are identified by integers indices numbered consecutively
starting from 0. The total number nodes will automatically increase
based upon the maximum argument ever passed to add_edge().

Time Complexity:
- add_edge() is O(1) amortized per call, or O(n) for n calls where each
  node index added is at most n.
- dfs(), has_cycle(), is_tree(), and is_dag() are each O(n) per call on
  the number of edges added so far.
- All other public member functions are O(1).

Space Complexity:
- O(n) to store a graph of n edges.
- dfs(), has_cycle(), is_tree(), and is_dag() each require O(n)
  auxiliary on the number of edges.
- All other public member functions require O(1) auxiliary.

*/

#include <algorithm> /* std::max */
#include <cstddef>   /* size_t */
#include <vector>

class graph {
  std::vector< std::vector<int> > adj;
  bool _is_directed;

  template<class Action>
  void dfs(int n, std::vector<bool> & vis, Action act);

  bool has_cycle(int n, int prev, std::vector<bool> & vis,
                 std::vector<bool> & onstack);

 public:
  graph(bool is_directed = true) {
    this->_is_directed = is_directed;
  }

  bool is_directed() const {
    return _is_directed;
  }

  size_t nodes() const {
    return adj.size();
  }

  std::vector<int>& operator [] (int n) {
    return adj[n];
  }

  void add_edge(int u, int v);
  template<class Action> void dfs(int start, Action act);
  bool has_cycle();
  bool is_tree();
  bool is_dag();
};

void graph::add_edge(int u, int v) {
  if (u >= (int)adj.size() || v >= (int)adj.size())
    adj.resize(std::max(u, v) + 1);
  adj[u].push_back(v);
  if (!is_directed())
    adj[v].push_back(u);
}

template<class Action>
void graph::dfs(int n, std::vector<bool> & vis, Action act) {
  act(n);
  vis[n] = true;
  std::vector<int>::iterator it;
  for (it = adj[n].begin(); it != adj[n].end(); ++it) {
    if (!vis[*it])
      dfs(*it, vis, act);
  }
}

template<class Action> void graph::dfs(int start, Action act) {
  std::vector<bool> vis(nodes(), false);
  dfs(start, vis, act);
}

bool graph::has_cycle(int n, int prev, std::vector<bool> & vis,
                      std::vector<bool> & onstack) {
  vis[n] = true;
  onstack[n] = true;
  std::vector<int>::iterator it;
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

bool graph::has_cycle() {
  std::vector<bool> vis(nodes(), false), onstack(nodes(), false);
  for (int i = 0; i < (int)adj.size(); i++)
    if (!vis[i] && has_cycle(i, -1, vis, onstack))
      return true;
  return false;
}

bool graph::is_tree() {
  return !is_directed() && !has_cycle();
}

bool graph::is_dag() {
  return is_directed() && !has_cycle();
}

/*** Example Usage

Sample Output:
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
    assert(!g.has_cycle());
  }
  {
    graph tree(false);
    tree.add_edge(0, 1);
    tree.add_edge(0, 2);
    tree.add_edge(1, 3);
    tree.add_edge(1, 4);
    assert(tree.is_tree());
    tree.add_edge(2, 3);
    assert(!tree.is_tree());
  }
  return 0;
}
