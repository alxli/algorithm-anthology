/*

Given a simple undirected graph with maximum degree $D$, assign a color to every edge so that no two
edges sharing an endpoint receive the same color. Vizing's theorem guarantees that $D + 1$ colors
always suffice for a simple graph (while $D$ colors are clearly necessary), and the Misra-Gries
algorithm constructs such a coloring. Deciding between $D$ and $D + 1$ colors is NP-hard in general,
though bipartite graphs can always be edge-colored with exactly $D$ colors.

The algorithm colors edges one at a time. To color an edge $(u, v)$ it grows a maximal "fan" of $u$:
a sequence of neighbors starting at $v$ in which each successive edge's color is free at the
previous fan vertex. It then picks a color $c$ free at $u$ and a color $d$ free at the fan's last
vertex, and flips the colors along the maximal alternating $c$/$d$ path leaving $u$. This frees $d$
at $u$, after which rotating the fan shifts colors over by one and assigns $d$ to the new edge. Each
step keeps the partial coloring proper and never introduces a $(D + 2)$-th color.

The graph must be simple: no self-loops and no repeated edges.

- `edge_coloring(n, edges)` returns a vector of colors, one per entry of `edges`, using colors in
  the range $[0, D]$ where $D$ is the maximum degree. Nodes are numbered from 0 to `n - 1`, and
  `edges[i]` is an undirected edge `u`-`v`.

Time Complexity:
- O(n^2 * m) in the worst case for `edge_coloring()`, where $n$ is the number of nodes and $m$ the
  number of edges; substantially faster in practice.

Space Complexity:
- O(n^2) to store the per-edge colors.

*/

#include <utility>
#include <vector>

class EdgeColoring {
  int n, max_deg;
  std::vector<std::vector<int>> color;  // color[u][v] is the color of edge (u, v), or 0 if none.

  bool is_free(int x, int c) const {
    for (int w = 0; w < n; w++) {
      if (color[x][w] == c) {
        return false;
      }
    }
    return true;
  }

  int free_color(int x) const {
    int c = 1;
    while (!is_free(x, c)) {
      c++;
    }
    return c;
  }

  void set_color(int u, int v, int c) {
    color[u][v] = c;
    color[v][u] = c;
  }

  // Flip colors c and d along the maximal alternating path of those colors starting at x.
  void invert_path(int x, int d, int c) {
    for (int w = 0; w < n; w++) {
      if (color[x][w] == d) {
        set_color(x, w, 0);
        invert_path(w, c, d);
        set_color(x, w, c);
        return;
      }
    }
  }

 public:
  EdgeColoring(int n, const std::vector<std::pair<int, int>> &edges)
      : n(n), max_deg(0), color(n, std::vector<int>(n, 0)) {
    std::vector<int> deg(n, 0);
    for (const auto &e : edges) {
      max_deg = std::max(max_deg, std::max(++deg[e.first], ++deg[e.second]));
    }
    for (const auto &e : edges) {
      add_edge(e.first, e.second);
    }
  }

  void add_edge(int u, int v) {
    // Build a maximal fan of u starting at v: fan[0] = v, and color(u, fan[i+1]) is free at fan[i].
    std::vector<int> fan{v};
    std::vector<bool> used(n, false);
    used[v] = true;
    while (true) {
      int last = fan.back(), next = -1;
      for (int w = 0; w < n; w++) {
        if (!used[w] && color[u][w] != 0 && is_free(last, color[u][w])) {
          next = w;
          break;
        }
      }
      if (next == -1) {
        break;
      }
      fan.push_back(next);
      used[next] = true;
    }
    int c = free_color(u), d = free_color(fan.back());
    invert_path(u, d, c);
    // Find the prefix of the fan to rotate: the first vertex on which d is now free.
    int k = 0;
    while (k < static_cast<int>(fan.size()) && color[u][fan[k]] != d && !is_free(fan[k], d)) {
      k++;
    }
    for (int j = 0; j < k; j++) {
      set_color(u, fan[j], color[u][fan[j + 1]]);
    }
    set_color(u, fan[k], d);
  }

  std::vector<int> coloring(const std::vector<std::pair<int, int>> &edges) const {
    std::vector<int> result;
    result.reserve(edges.size());
    for (const auto &e : edges) {
      result.push_back(color[e.first][e.second] - 1);  // Shift internal 1..D+1 colors to 0..D.
    }
    return result;
  }
};

std::vector<int> edge_coloring(int n, const std::vector<std::pair<int, int>> &edges) {
  return EdgeColoring(n, edges).coloring(edges);
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
#include <set>
using namespace std;

int main() {
  // A 5-cycle has maximum degree 2 but is odd, so it needs 3 colors (D + 1).
  vector<pair<int, int>> cycle{{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 0}};
  vector<int> col = edge_coloring(5, cycle);
  set<int> palette(col.begin(), col.end());
  assert(palette.size() == 3);

  // Verify the coloring is proper: edges sharing an endpoint differ in color.
  int m = static_cast<int>(cycle.size());
  for (int i = 0; i < m; i++) {
    for (int j = i + 1; j < m; j++) {
      bool share = cycle[i].first == cycle[j].first || cycle[i].first == cycle[j].second ||
                   cycle[i].second == cycle[j].first || cycle[i].second == cycle[j].second;
      if (share) {
        assert(col[i] != col[j]);
      }
    }
  }

  // A bipartite graph (here K_{2,3}) is edge-colored with exactly D colors.
  vector<pair<int, int>> bip{{0, 2}, {0, 3}, {0, 4}, {1, 2}, {1, 3}, {1, 4}};
  vector<int> bcol = edge_coloring(5, bip);
  set<int> bpal(bcol.begin(), bcol.end());
  assert(static_cast<int>(bpal.size()) == 3);  // Max degree is 3.
  return 0;
}
