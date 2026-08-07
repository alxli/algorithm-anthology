/*

Given a simple undirected graph, assign a color to every node such that no pair of adjacent nodes
have the same color, and that the total number of colors used is minimized.

This implementation finds the chromatic number by backtracking: nodes (ordered by degree to prune
sooner) are colored one at a time, each tried with every color already in use plus one new color,
while the fewest colors seen in a complete coloring so far bounds the search and cuts off any branch
at least as costly.

- `color_graph()` populates `color` and returns minimum color count for a global, bidirectionally
  pre-populated adjacency matrix `adj`, whose row and column indices represent the nodes.

Time Complexity:
- O(n^n) per call in the worst case, where $n$ is the number of nodes.

Space Complexity:
- O(n^2) for storage of the graph, where $n$ is the number of nodes.
- O(n) auxiliary stack space and O(n) auxiliary heap space.

*/

#include <algorithm>
#include <numeric>
#include <vector>

std::vector<std::vector<char>> adj;
std::vector<int> color, curr, order;
int min_colors;

void color_rec(int pos, int used_colors) {
  if (used_colors >= min_colors) {
    return;
  }
  if (pos == static_cast<int>(order.size())) {
    color = curr;
    min_colors = used_colors;
    return;
  }
  int u = order[pos];
  for (int c = 0; c < used_colors; c++) {
    bool valid = true;
    for (int i = 0; i < pos; i++) {
      int v = order[i];
      if (adj[u][v] && curr[v] == c) {
        valid = false;
        break;
      }
    }
    if (valid) {
      curr[u] = c;
      color_rec(pos + 1, used_colors);
      curr[u] = -1;
    }
  }
  curr[u] = used_colors;
  color_rec(pos + 1, used_colors + 1);
  curr[u] = -1;
}

int color_graph() {
  int n = static_cast<int>(adj.size());
  if (n == 0) {
    color.clear();
    return 0;
  }
  std::vector<int> degree(n);
  for (int u = 0; u < n; u++) {
    for (int v = 0; v < n; v++) {
      degree[u] += adj[u][v];
    }
  }
  order.resize(n);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int u, int v) {
    if (degree[u] != degree[v]) {
      return degree[u] > degree[v];
    }
    return u < v;
  });
  min_colors = n + 1;
  color.assign(n, -1);
  curr.assign(n, -1);
  color_rec(0, 0);
  std::vector<int> remap(min_colors, -1);
  int colors = 0;
  for (int &c : color) {
    if (remap[c] == -1) {
      remap[c] = colors++;
    }
    c = remap[c];
  }
  return min_colors;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void add_edge(int u, int v) {
  adj[u][v] = true;
  adj[v][u] = true;
}

int main() {
  //     0
  //   / |
  // 1---4---2
  //   \ | /
  //     3
  int nodes = 5;
  adj.assign(nodes, vector<char>(nodes));
  add_edge(0, 1);
  add_edge(0, 4);
  add_edge(1, 3);
  add_edge(1, 4);
  add_edge(2, 3);
  add_edge(2, 4);
  add_edge(3, 4);
  assert(color_graph() == 3);
  for (int u = 0; u < nodes; u++) {
    for (int v = u + 1; v < nodes; v++) {
      if (adj[u][v]) {
        assert(color[u] != color[v]);
      }
    }
  }
  assert((color == vector<int>{0, 1, 1, 0, 2}));
  return 0;
}
