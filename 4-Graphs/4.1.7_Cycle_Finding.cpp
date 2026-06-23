/*

Given a directed or undirected graph, find simple cycles encountered by a depth-first search.
Cycles are returned as edge IDs rather than vertex lists, which makes the representation robust for
multigraphs with parallel edges. In an undirected graph, the cycles found this way form a DFS cycle
basis: one cycle for each non-tree edge. In a directed graph, this finds cycles created by back
edges to vertices still on the recursion stack.

- `CycleFinder(n, directed)` constructs a graph of `n` nodes numbered [$0$, `n`). The graph is
  directed if `directed` is true, or undirected otherwise.
- `add_edge(u, v)` adds an edge and returns its edge ID.
- `find_cycles(max_cycles, max_total_size)` returns simple cycles as vectors of edge IDs, stopping
  once either optional limit is reached.
- `cycle_vertices(edge_cycle)` converts one edge-ID cycle into a corresponding cyclic list of
  vertices.

Time Complexity:
- O(max(n, m + s)) per call to `find_cycles()`, where $n$ is the number of nodes, $m$ is the number
  of edges, and $s$ is the total size of the returned cycles.
- O(k) per call to `cycle_vertices()`, where $k$ is the number of edges in the cycle.

Space Complexity:
- O(max(n, m + s)) for graph storage, DFS state, and returned cycles.
- O(n) auxiliary stack space for the DFS.

*/

#include <cassert>
#include <vector>

class CycleFinder {
  std::vector<std::pair<int, int>> edges;
  std::vector<std::vector<int>> adj;
  bool directed;

  void dfs_cycles(
      int u, int parent_edge, int max_cycles, int max_total_size, std::vector<int> &state,
      std::vector<int> &stack, std::vector<std::vector<int>> &cycles, int &total_size
  ) const {
    if (static_cast<int>(cycles.size()) >= max_cycles || total_size >= max_total_size) {
      return;
    }
    state[u] = static_cast<int>(stack.size());
    for (int id : adj[u]) {
      if (!directed && id == parent_edge) {
        continue;
      }
      const auto &[eu, ev] = edges[id];
      int v = directed ? ev : eu ^ ev ^ u;
      if (state[v] >= 0) {
        std::vector<int> cycle{id};
        for (int i = state[v]; i < static_cast<int>(stack.size()); i++) {
          cycle.push_back(stack[i]);
        }
        cycles.push_back(cycle);
        total_size += static_cast<int>(cycle.size());
        if (static_cast<int>(cycles.size()) >= max_cycles || total_size >= max_total_size) {
          return;
        }
      } else if (state[v] == -1) {
        stack.push_back(id);
        dfs_cycles(v, id, max_cycles, max_total_size, state, stack, cycles, total_size);
        stack.pop_back();
      }
    }
    state[u] = -2;
  }

 public:
  CycleFinder(int n, bool directed) : adj(n), directed(directed) {}

  int add_edge(int u, int v) {
    int id = static_cast<int>(edges.size());
    edges.emplace_back(u, v);
    adj[u].push_back(id);
    if (!directed) {
      adj[v].push_back(id);
    }
    return id;
  }

  std::vector<std::vector<int>> find_cycles(
      int max_cycles = 1 << 30, int max_total_size = 1 << 30
  ) const {
    int n = static_cast<int>(adj.size());
    std::vector<int> state(n, -1), stack;
    std::vector<std::vector<int>> cycles;
    int total_size = 0;
    for (int u = 0; u < n; u++) {
      if (state[u] == -1) {
        dfs_cycles(u, -1, max_cycles, max_total_size, state, stack, cycles, total_size);
      }
    }
    return cycles;
  }

  std::vector<int> cycle_vertices(const std::vector<int> &edge_cycle) const {
    int size = static_cast<int>(edge_cycle.size());
    std::vector<int> vertices;
    if (size == 0) {
      return vertices;
    }
    if (size <= 2) {
      vertices.push_back(edges[edge_cycle[0]].first);
      if (size == 2) {
        vertices.push_back(edges[edge_cycle[0]].second);
      }
      return vertices;
    }
    for (int i = 0; i < size; i++) {
      const auto &[au, av] = edges[edge_cycle[i]];
      const auto &[bu, bv] = edges[edge_cycle[(i + 1) % size]];
      vertices.push_back((bu == au || bv == au) ? av : au);
    }
    return vertices;
  }
};

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
using namespace std;

int main() {
  {
    CycleFinder g(4, false);
    int a = g.add_edge(0, 1);
    int b = g.add_edge(1, 2);
    int c = g.add_edge(2, 0);
    g.add_edge(2, 3);
    vector<vector<int>> cycles = g.find_cycles();
    assert(cycles.size() == 1);
    sort(cycles[0].begin(), cycles[0].end());
    assert((cycles[0] == vector<int>{a, b, c}));
    vector<int> vertices = g.cycle_vertices(cycles[0]);
    assert(vertices.size() == 3);
  }
  {
    CycleFinder g(2, false);
    int a = g.add_edge(0, 1);
    int b = g.add_edge(0, 1);
    vector<vector<int>> cycles = g.find_cycles();
    assert(cycles.size() == 1);
    sort(cycles[0].begin(), cycles[0].end());
    assert((cycles[0] == vector<int>{a, b}));
  }
  {
    CycleFinder g(3, true);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    assert(g.find_cycles(1).size() == 1);
  }
  return 0;
}
