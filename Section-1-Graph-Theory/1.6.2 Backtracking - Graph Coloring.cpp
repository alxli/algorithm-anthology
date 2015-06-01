/*
1.6.2 - Backtracking - Graph Coloring

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
3

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 30;
int nodes, edges, a, b, adj[MAXN][MAXN];
int min_colors, best_cols[MAXN];
int id[MAXN + 1], deg[MAXN + 1];

void dfs(int cols[], int from, int to, int cur, int used_colors) {
  if (used_colors >= min_colors) return;
  if (cur == to) {
    for (int i = from; i < to; i++)
      best_cols[id[i]] = cols[i];
    min_colors = used_colors;
    return;
  }
  vector<bool> used(used_colors + 1);
  for (int i = 0; i < cur; i++)
    if (adj[id[cur]][id[i]]) used[cols[i]] = true;
  for (int i = 0; i <= used_colors; i++) {
    if (!used[i]) {
      int tmp = cols[cur];
      cols[cur] = i;
      dfs(cols, from, to, cur + 1, max(used_colors, i + 1));
      cols[cur] = tmp;
    }
  }
}

int color_graph() {
  for (int i = 0; i <= nodes; i++) id[i] = i;
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
      min_colors = nodes + 1;
      int cols[nodes];
      dfs(cols, from, to, from, 0);
      from = to;
      res = max(res, min_colors);
    }
  }
  return res;
}

int main() {
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> a >> b; --a; --b;
    adj[a][b] = adj[b][a] = true;
  }
  cout << "Colored using " << color_graph();
  cout << " color(s).\nColorings are:\n";
  for (int i = 0; i < min_colors; i++) {
  	cout << "Color " << i + 1 << ":";
    for (int j = 0; j < nodes; j++)
      if (best_cols[j] == i) cout << " " << j + 1;
    cout << "\n";
  }
  return 0;
}
