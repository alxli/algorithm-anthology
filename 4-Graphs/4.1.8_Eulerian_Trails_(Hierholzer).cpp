/*

A Eulerian trail is a path in a graph which contains every edge exactly once. A Eulerian cycle or
circuit is an Eulerian trail which begins and ends on the same node. A directed graph has a
Eulerian trail when all nonzero-degree nodes belong to one connected part of the underlying graph,
and either every node has equal in-degree and out-degree or exactly one node has one extra outgoing
edge and exactly one node has one extra incoming edge.

Hierholzer's algorithm walks unused edges until stuck, then backtracks to splice each closed detour
into the final trail. For a directed graph known to have a trail from `start`, the core algorithm
can simply consume outgoing edges by popping them from a local copy of the adjacency list.

- `known_eulerian_path_directed(adj, start)` returns a vertex trail using every directed edge in
  `adj` exactly once, assuming such a trail exists and begins at `start`.
- `eulerian_path_directed(adj, start)` returns a vertex trail using every directed edge in `adj`
  exactly once, or an empty vector if no such trail exists. If `start` $= -1$, a valid start is
  chosen automatically; otherwise the trail must begin at `start`.

Parallel directed edges are supported by storing duplicate neighbors in `adj`. Since these functions
return only vertices, use `EulerianGraph` below when edge IDs are needed.

Time Complexity:
- O(max(n, m)) per call to `known_eulerian_path_directed()` and `eulerian_path_directed()`, where
  $n$ is the number of nodes and $m$ is the number of edges.

Space Complexity:
- O(max(n, m)) auxiliary stack and heap space for `known_eulerian_path_directed()` and
  `eulerian_path_directed()`.

*/

#include <algorithm>
#include <cassert>
#include <vector>

std::vector<int> known_eulerian_path_directed(std::vector<std::vector<int>> adj, int start) {
  assert(0 <= start && start < static_cast<int>(adj.size()));
  std::vector<int> stack{start}, path;
  while (!stack.empty()) {
    int u = stack.back();
    if (adj[u].empty()) {
      path.push_back(u);
      stack.pop_back();
    } else {
      stack.push_back(adj[u].back());
      adj[u].pop_back();
    }
  }
  std::reverse(path.begin(), path.end());
  return path;
}

std::vector<int> eulerian_path_directed(const std::vector<std::vector<int>> &adj, int start = -1) {
  int n = static_cast<int>(adj.size()), m = 0;
  if (n == 0) {
    return {};
  }
  assert(-1 <= start && start < n);
  std::vector<int> indeg(n), outdeg(n);
  for (int u = 0; u < n; u++) {
    outdeg[u] = static_cast<int>(adj[u].size());
    m += outdeg[u];
    for (int v : adj[u]) {
      indeg[v]++;
    }
  }
  if (m == 0) {
    return {start == -1 ? 0 : start};
  }
  int source = -1, sources = 0, sinks = 0;
  for (int u = 0; u < n; u++) {
    int diff = outdeg[u] - indeg[u];
    if (diff == 1) {
      source = u;
      sources++;
    } else if (diff == -1) {
      sinks++;
    } else if (diff != 0) {
      return {};
    }
  }
  if (!((sources == 0 && sinks == 0) || (sources == 1 && sinks == 1))) {
    return {};
  }
  if (start == -1) {
    if (source != -1) {
      start = source;
    } else {
      for (int u = 0; u < n && start == -1; u++) {
        if (outdeg[u] > 0) {
          start = u;
        }
      }
    }
  } else if ((source != -1 && start != source) || (source == -1 && outdeg[start] == 0 && m > 0)) {
    return {};
  }
  std::vector<int> path = known_eulerian_path_directed(adj, start);
  return static_cast<int>(path.size()) == m + 1 ? path : std::vector<int>();
}

/*

The `EulerianGraph` class is the more general edge-ID variant. An undirected graph has a Eulerian
trail when all nonzero-degree nodes are connected and either zero or two nodes have odd degree. This
class stores and returns edge IDs, which supports directed graphs, undirected graphs, and
multigraphs: parallel edges are distinct because each edge receives its own ID.

- `EulerianGraph(n, directed)` constructs a graph of `n` nodes numbered $[0, `n`)$. The graph is
  directed if `directed` is true, or undirected otherwise.
- `add_edge(u, v)` adds an edge and returns its edge ID.
- `eulerian_path(start)` returns a trail using every edge exactly once, or a result with
  `start` $= -1$ if no such trail exists. If `start` $= -1$, a valid start is chosen automatically.
- `EulerianTrail::is_cycle()` returns whether the trail begins and ends on the same node.

Time Complexity:
- O(max(n, m)) per call to `eulerian_path()`, where $n$ is the number of nodes and $m$ is the
  number of edges.

Space Complexity:
- O(max(n, m)) for graph storage and auxiliary arrays.

*/

class EulerianGraph {
  std::vector<std::pair<int, int>> edges;
  std::vector<std::vector<int>> adj;
  bool directed;

  bool valid_degrees(int start) const {
    int n = static_cast<int>(adj.size());
    if (directed) {
      std::vector<int> indeg(n), outdeg(n);
      for (const auto &[eu, ev] : edges) {
        outdeg[eu]++;
        indeg[ev]++;
      }
      int source = -1, sink = -1;
      for (int u = 0; u < n; u++) {
        int diff = outdeg[u] - indeg[u];
        if (diff == 1) {
          if (source != -1) {
            return false;
          }
          source = u;
        } else if (diff == -1) {
          if (sink != -1) {
            return false;
          }
          sink = u;
        } else if (diff != 0) {
          return false;
        }
      }
      if ((source == -1) != (sink == -1)) {
        return false;
      }
      if (start != -1) {
        return source == -1 ? outdeg[start] > 0 : start == source;
      }
      return true;
    }
    std::vector<int> degree(n);
    for (const auto &[eu, ev] : edges) {
      degree[eu]++;
      degree[ev]++;
    }
    std::vector<int> odd;
    for (int u = 0; u < n; u++) {
      if (degree[u] % 2 == 1) {
        odd.push_back(u);
      }
    }
    if (!odd.empty() && odd.size() != 2) {
      return false;
    }
    if (start != -1) {
      return odd.empty() ? degree[start] > 0 : (start == odd[0] || start == odd[1]);
    }
    return true;
  }

  int choose_start() const {
    int n = static_cast<int>(adj.size());
    if (directed) {
      std::vector<int> indeg(n), outdeg(n);
      for (const auto &[eu, ev] : edges) {
        outdeg[eu]++;
        indeg[ev]++;
      }
      for (int u = 0; u < n; u++) {
        if (outdeg[u] - indeg[u] == 1) {
          return u;
        }
      }
      for (int u = 0; u < n; u++) {
        if (outdeg[u] > 0) {
          return u;
        }
      }
    } else {
      std::vector<int> degree(n);
      for (const auto &[eu, ev] : edges) {
        degree[eu]++;
        degree[ev]++;
      }
      for (int u = 0; u < n; u++) {
        if (degree[u] % 2 == 1) {
          return u;
        }
      }
      for (int u = 0; u < n; u++) {
        if (degree[u] > 0) {
          return u;
        }
      }
    }
    return 0;
  }

  std::vector<int> build_vertices(int start, const std::vector<int> &trail_edges) const {
    std::vector<int> vertices{start};
    int u = start;
    for (int id : trail_edges) {
      const auto &[eu, ev] = edges[id];
      u = directed ? ev : eu ^ ev ^ u;
      vertices.push_back(u);
    }
    return vertices;
  }

 public:
  struct EulerianTrail {
    int start = -1;
    std::vector<int> edges;
    std::vector<int> vertices;

    bool is_cycle() const {
      return start != -1 && vertices.size() > 1 && vertices.front() == vertices.back();
    }
  };

  EulerianGraph(int n, bool directed) : adj(n), directed(directed) {}

  int add_edge(int u, int v) {
    int id = static_cast<int>(edges.size());
    edges.emplace_back(u, v);
    adj[u].push_back(id);
    if (!directed) {
      adj[v].push_back(id);
    }
    return id;
  }

  EulerianTrail eulerian_path(int start = -1) const {
    int n = static_cast<int>(adj.size());
    int m = static_cast<int>(edges.size());
    if (m == 0) {
      int s = (start == -1 ? 0 : start);
      return EulerianTrail{s, {}, {s}};
    }
    if (!valid_degrees(start)) {
      return EulerianTrail{};
    }
    if (start == -1) {
      start = choose_start();
    }
    std::vector<char> used(m, false);
    std::vector<int> ptr(n), vertex_stack{start}, edge_stack{-1}, trail_edges;
    while (!vertex_stack.empty()) {
      int u = vertex_stack.back();
      while (ptr[u] < static_cast<int>(adj[u].size()) && used[adj[u][ptr[u]]]) {
        ptr[u]++;
      }
      if (ptr[u] == static_cast<int>(adj[u].size())) {
        if (edge_stack.back() != -1) {
          trail_edges.push_back(edge_stack.back());
        }
        vertex_stack.pop_back();
        edge_stack.pop_back();
      } else {
        int id = adj[u][ptr[u]++];
        used[id] = true;
        const auto &[eu, ev] = edges[id];
        int v = directed ? ev : eu ^ ev ^ u;
        vertex_stack.push_back(v);
        edge_stack.push_back(id);
      }
    }
    if (static_cast<int>(trail_edges.size()) != m) {
      return EulerianTrail{};
    }
    std::reverse(trail_edges.begin(), trail_edges.end());
    return EulerianTrail{start, trail_edges, build_vertices(start, trail_edges)};
  }
};

/*** Example Usage and Output:

Simple directed path: 0->1->2->0->3
Directed Eulerian cycle: 0->1->3->4->1->2->0
Undirected Eulerian cycle: 0->1->4->3->1->2->0
Parallel-edge cycle: 0->1->0
Directed Eulerian path: 0->1->2

***/

#include <cassert>
#include <iostream>
using namespace std;

void print_vertices(const string &label, const vector<int> &vertices) {
  cout << label << ": ";
  for (int i = 0; i < static_cast<int>(vertices.size()); i++) {
    if (i > 0) {
      cout << "->";
    }
    cout << vertices[i];
  }
  cout << endl;
}

int main() {
  {
    // 3 <-- 0 --> 1
    //        ^    |
    //         \   v
    //          +--2
    vector<vector<int>> adj(4);
    adj[0].push_back(1);
    adj[1].push_back(2);
    adj[2].push_back(0);
    adj[0].push_back(3);
    assert((known_eulerian_path_directed(adj, 0) == vector<int>{0, 1, 2, 0, 3}));
    vector<int> path = eulerian_path_directed(adj);
    assert((path == vector<int>{0, 1, 2, 0, 3}));
    assert(eulerian_path_directed(adj, 1).empty());
    print_vertices("Simple directed path", path);
  }
  {
    // 0 <--- 2
    // |    ^ ^
    // |   /  |
    // v /    |
    // 1 ---> 3 ---> 4
    // ^_____________|
    EulerianGraph g(5, true);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    g.add_edge(1, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 1);
    auto trail = g.eulerian_path(0);
    assert(trail.edges.size() == 6);
    assert(trail.vertices.front() == 0 && trail.vertices.back() == 0);
    assert(trail.is_cycle());
    print_vertices("Directed Eulerian cycle", trail.vertices);
  }
  {
    // 0 ---- 2
    // |     /|
    // |   /  |
    // | /    |
    // 1 ---- 3 ---- 4
    // |_____________|
    EulerianGraph g(5, false);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    g.add_edge(2, 0);
    g.add_edge(1, 3);
    g.add_edge(3, 4);
    g.add_edge(4, 1);
    auto trail = g.eulerian_path();
    assert(trail.edges.size() == 6);
    assert(trail.vertices.front() == trail.vertices.back());
    assert(trail.is_cycle());
    print_vertices("Undirected Eulerian cycle", trail.vertices);
  }
  {
    EulerianGraph g(2, false);
    int a = g.add_edge(0, 1);
    int b = g.add_edge(0, 1);
    auto trail = g.eulerian_path(0);
    assert(trail.is_cycle());
    assert((trail.edges == vector<int>{a, b} || trail.edges == vector<int>{b, a}));
    print_vertices("Parallel-edge cycle", trail.vertices);
  }
  {
    EulerianGraph g(3, true);
    g.add_edge(0, 1);
    g.add_edge(1, 2);
    auto trail = g.eulerian_path();
    assert(trail.start == 0);
    assert((trail.vertices == vector<int>{0, 1, 2}));
    assert(!trail.is_cycle());
    print_vertices("Directed Eulerian path", trail.vertices);
  }
  return 0;
}
