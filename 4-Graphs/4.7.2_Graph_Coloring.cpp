/*

Given an undirected graph, assign a color to every node such that no pair of
adjacent nodes have the same color, and that the total number of colors used is
minimized. color_graph() applies to a global, pre-populated adjacency matrix
adj[][] which must satisfy the condition that adj[u][v] is true if and only if
adj[v][u] is true, for all pairs of nodes u and v respectively between 0
(inclusive) and the total number of nodes (exclusive) as passed in the function
argument.

Time Complexity:
- Exponential on the number of nodes per call to color_graph().

Space Complexity:
- O(n^2) for storage of the graph, where n is the number of nodes.
- O(n) auxiliary stack and heap space for color_graph().

*/

#include <algorithm>
#include <vector>

const int MAXN = 30;
int adj[MAXN][MAXN], min_colors, color[MAXN];
int curr[MAXN], id[MAXN + 1], degree[MAXN + 1];

void rec(int lo, int hi, int n, int used_colors) {
  if (used_colors >= min_colors) {
    return;
  }
  if (n == hi) {
    for (int i = lo; i < hi; i++) {
      color[id[i]] = curr[i];
    }
    min_colors = used_colors;
    return;
  }
  std::vector<bool> used(used_colors + 1);
  for (int i = 0; i < n; i++) {
    if (adj[id[n]][id[i]]) {
      used[curr[i]] = true;
    }
  }
  for (int i = 0; i <= used_colors; i++) {
    if (!used[i]) {
      int tmp = curr[n];
      curr[n] = i;
      rec(lo, hi, n + 1, std::max(used_colors, i + 1));
      curr[n] = tmp;
    }
  }
}

int color_graph(int nodes) {
  for (int i = 0; i <= nodes; i++) {
    id[i] = i;
    degree[i] = 0;
  }
  int res = 1, lo = 0;
  for (int hi = 1; hi <= nodes; hi++) {
    int best = hi;
    for (int i = hi; i < nodes; i++) {
      if (adj[id[hi - 1]][id[i]]) {
        degree[id[i]]++;
      }
      if (degree[id[best]] < degree[id[i]]) {
        best = i;
      }
    }
    std::swap(id[hi], id[best]);
    if (degree[id[hi]] == 0) {
      min_colors = nodes + 1;
      std::fill(curr, curr + nodes, 0);
      rec(lo, hi, lo, 0);
      lo = hi;
      res = std::max(res, min_colors);
    }
  }
  return res;
}

/*** Example Usage and Output:

Colored using 3 color(s):
Color 1: 0 3
Color 2: 1 2
Color 3: 4

***/

#include <cassert>
#include <iostream>
using namespace std;

void add_edge(int u, int v) {
  adj[u][v] = true;
  adj[v][u] = true;
}

int main() {
  add_edge(0, 1);
  add_edge(0, 4);
  add_edge(1, 3);
  add_edge(1, 4);
  add_edge(2, 3);
  add_edge(2, 4);
  add_edge(3, 4);
  int colors = color_graph(5);
  cout << "Colored using " << colors << " color(s):" << endl;
  for (int i = 0; i < colors; i++) {
    cout << "Color " << i + 1 << ":";
    for (int j = 0; j < 5; j++) {
      if (color[j] == i) {
        cout << " " << j;
      }
    }
    cout << endl;
  }
  return 0;
}
