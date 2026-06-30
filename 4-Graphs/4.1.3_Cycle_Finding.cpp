/*

Given a directed or undirected graph, find simple cycles encountered by a depth-first search. The
free functions return one cycle of a simple graph as a list of vertices. For undirected graphs, an
edge to a visited non-parent vertex closes a cycle. For directed graphs, only an edge to a vertex
still on the recursion stack closes a cycle; edges to already-finished vertices do not.

Both functions remember the DFS parent of each vertex. When a closing edge `u` $\to$ `cycle_start`
is found, the answer is reconstructed by walking parents backward from `u` to `cycle_start`. In
`find_cycle_directed()`, `state[v]` is $0$ for unvisited vertices, $1$ for vertices currently on the
recursion stack, and $2$ for finished vertices.

- `find_cycle_undirected(adj)` returns one vertex cycle in a simple undirected graph, or an empty
  vector if no cycle exists.
- `find_cycle_directed(adj)` returns one vertex cycle in a simple directed graph, or an empty vector
  if no cycle exists.

Time Complexity:
- O(max(n, m)) per call to `find_cycle_undirected()` and `find_cycle_directed()`, where $n$ is the
  number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) for storage of the graph.
- O(n) auxiliary stack space for the DFS, plus O(n) auxiliary heap space for the returned cycle.

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

std::vector<int> find_cycle_undirected(const std::vector<std::vector<int>> &adj) {
  int n = static_cast<int>(adj.size());
  if (n == 0) {
    return {};
  }
  std::vector<char> visit(n, false);
  std::vector<int> parent(n, -1), cycle;
  int cycle_start = -1, cycle_end = -1;
  auto dfs = [&](auto &&dfs, int u, int p) -> bool {
    visit[u] = true;
    for (int v : adj[u]) {
      if (v == p) {
        continue;
      }
      if (visit[v]) {
        cycle_start = v;
        cycle_end = u;
        return true;
      }
      parent[v] = u;
      if (dfs(dfs, v, u)) {
        return true;
      }
    }
    return false;
  };
  for (int u = 0; u < n && cycle_start == -1; u++) {
    if (!visit[u]) {
      dfs(dfs, u, -1);
    }
  }
  if (cycle_start == -1) {
    return {};
  }
  cycle.push_back(cycle_start);
  for (int u = cycle_end; u != cycle_start; u = parent[u]) {
    cycle.push_back(u);
  }
  std::reverse(cycle.begin() + 1, cycle.end());
  return cycle;
}

std::vector<int> find_cycle_directed(const std::vector<std::vector<int>> &adj) {
  int n = static_cast<int>(adj.size());
  if (n == 0) {
    return {};
  }
  std::vector<char> state(n, 0);  // 0 = unvisited, 1 = currently on DFS stack, 2 = finished.
  std::vector<int> parent(n, -1), cycle;
  int cycle_start = -1, cycle_end = -1;
  auto dfs = [&](auto &&dfs, int u) -> bool {
    state[u] = 1;
    for (int v : adj[u]) {
      if (state[v] == 0) {
        parent[v] = u;
        if (dfs(dfs, v)) {
          return true;
        }
      } else if (state[v] == 1) {
        cycle_start = v;
        cycle_end = u;
        return true;
      }
    }
    state[u] = 2;
    return false;
  };
  for (int u = 0; u < n && cycle_start == -1; u++) {
    if (state[u] == 0) {
      dfs(dfs, u);
    }
  }
  if (cycle_start == -1) {
    return {};
  }
  cycle.push_back(cycle_start);
  for (int u = cycle_end; u != cycle_start; u = parent[u]) {
    cycle.push_back(u);
  }
  std::reverse(cycle.begin() + 1, cycle.end());
  return cycle;
}

/*

The `CycleFinder` class is the more general multigraph variant. Its cycles are returned as edge IDs
rather than vertex lists, which makes the representation robust for parallel edges. In an undirected
graph, the cycles found this way form a DFS cycle basis: one cycle for each non-tree edge. In a
directed graph, this finds cycles created by back edges to vertices still on the recursion stack.
Here `state[v]` is $-1$ before discovery, $-2$ after finishing, and otherwise the index where the
vertex first appeared in the current edge stack; that index marks which stack suffix forms a cycle.

- `CycleFinder(n, directed)` constructs a graph of `n` nodes numbered $[0, `n`)$. The graph is
  directed if `directed` is true, or undirected otherwise.
- `add_edge(u, v)` adds an edge and returns its edge ID.
- `find_cycles(max_cycles, max_total_size)` returns simple cycles as vectors of edge IDs, stopping
  once either optional limit is reached.
- `cycle_vertices(edge_cycle)` converts one edge-ID cycle into a corresponding cyclic list of
  vertices.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the number of nodes.
- O(1) per call to `add_edge()`.
- O(max(n, m + s)) per call to `find_cycles()`, where $n$ is the number of nodes, $m$ is the number
  of edges, and $s$ is the total size of the returned cycles.
- O(k) per call to `cycle_vertices()`, where $k$ is the number of edges in the cycle.

Space Complexity:
- O(max(n, m)) for storage of the graph.
- O(n) auxiliary stack space plus O(s) auxiliary heap space for `find_cycles()`, where $s$ is the
  total size of the returned cycles.
- O(k) auxiliary heap space for `cycle_vertices()`, where $k$ is the number of edges in the cycle.

*/

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
    // -1 = unvisited, -2 = finished, otherwise the vertex's index in the current edge stack.
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
    vector<vector<int>> adj(5);
    auto add_edge = [&](int u, int v) {
      adj[u].push_back(v);
      adj[v].push_back(u);
    };
    add_edge(0, 1);
    add_edge(1, 2);
    add_edge(2, 0);
    add_edge(3, 4);
    // DFS encounters the triangle component before the acyclic 3-4 component.
    assert((find_cycle_undirected(adj) == vector<int>{0, 1, 2}));
  }
  {
    vector<vector<int>> adj(6);
    adj[0].push_back(1);
    adj[1].push_back(2);
    adj[3].push_back(4);
    adj[4].push_back(5);
    adj[5].push_back(3);
    assert((find_cycle_directed(adj) == vector<int>{3, 4, 5}));
    assert(find_cycle_directed({{1}, {2}, {}}).empty());
  }
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
    // Parallel undirected edges form a 2-edge cycle; edge IDs make this representable.
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
