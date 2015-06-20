/*

2.6.2 - Backtracking - Graph Coloring

Description: Given an undirected graph, assign colors to each
of the vertices such that no pair of adjacent vertices have the
same color. Furthermore, do so using the minimum # of colors.

Complexity: Exponential on the number of vertices. The exact
running time is difficult to calculate due to several pruning
optimizations used here.

=~=~=~=~= Sample Input =~=~=~=~=
5 7
0 1
0 4
1 3
1 4
2 3
2 4
3 4

=~=~=~=~= Sample Output =~=~=~=~=
Colored using 3 color(s). The colorings are:
Color 1: 0 3
Color 2: 1 2
Color 3: 4

*/

#include <algorithm> /* std::fill(), std::max() */
#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 30;
int cols[MAXN], adj[MAXN][MAXN];
int id[MAXN + 1], deg[MAXN + 1];
int min_cols, best_cols[MAXN];

void dfs(int from, int to, int cur, int used_cols) {
  if (used_cols >= min_cols) return;
  if (cur == to) {
    for (int i = from; i < to; i++)
      best_cols[id[i]] = cols[i];
    min_cols = used_cols;
    return;
  }
  vector<bool> used(used_cols + 1);
  for (int i = 0; i < cur; i++)
    if (adj[id[cur]][id[i]]) used[cols[i]] = true;
  for (int i = 0; i <= used_cols; i++) {
    if (!used[i]) {
      int tmp = cols[cur];
      cols[cur] = i;
      dfs(from, to, cur + 1, max(used_cols, i + 1));
      cols[cur] = tmp;
    }
  }
}

int color_graph(int nodes) {
  for (int i = 0; i <= nodes; i++) {
    id[i] = i;
    deg[i] = 0;
  }
  int res = 1;
  for (int from = 0, to = 1; to <= nodes; to++) {
    int best = to;
    for (int i = to; i < nodes; i++) {
      if (adj[id[to - 1]][id[i]]) deg[id[i]]++;
      if (deg[id[best]] < deg[id[i]]) best = i;
    }
    int tmp = id[to];
    id[to] = id[best];
    id[best] = tmp;
    if (deg[id[to]] == 0) {
      min_cols = nodes + 1;
      fill(cols, cols + nodes, 0);
      dfs(from, to, from, 0);
      from = to;
      res = max(res, min_cols);
    }
  }
  return res;
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u][v] = adj[v][u] = true;
  }
  cout << "Colored using " << color_graph(nodes);
  cout << " color(s). The colorings are:\n";
  for (int i = 0; i < min_cols; i++) {
    cout << "Color " << i + 1 << ":";
    for (int j = 0; j < nodes; j++)
      if (best_cols[j] == i) cout << " " << j;
    cout << "\n";
  }
  return 0;
}
