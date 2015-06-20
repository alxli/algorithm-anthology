/*

2.7.3 - Maximum Matching for General Graphs (Edmonds's Algorithm)

Description: Given a general directed graph, determine a maximal
subset of the edges such that no vertex is repeated in the subset.

Complexity: O(V^3) on the number of vertices.

=~=~=~=~= Sample Input =~=~=~=~=
4 8
0 1
1 0
1 2
2 1
2 3
3 2
3 0
0 3

=~=~=~=~= Sample Output =~=~=~=~=
Matched 2 pair(s). Matchings are:
0 1
2 3

*/

#include <iostream>
#include <vector>
using namespace std;

const int MAXN = 100;
int p[MAXN], base[MAXN], match[MAXN];
vector<int> adj[MAXN];

int lca(int nodes, int a, int b) {
  vector<bool> used(nodes);
  for (;;) {
    a = base[a];
    used[a] = true;
    if (match[a] == -1) break;
    a = p[match[a]];
  }
  for (;;) {
    b = base[b];
    if (used[b]) return b;
    b = p[match[b]];
  }
}

void mark_path(vector<bool> & blossom, int v, int b, int children) {
  for (; base[v] != b; v = p[match[v]]) {
    blossom[base[v]] = blossom[base[match[v]]] = true;
    p[v] = children;
    children = match[v];
  }
}

int find_path(int nodes, int root) {
  vector<bool> used(nodes);
  for (int i = 0; i < nodes; ++i) {
    p[i] = -1;
    base[i] = i;
  }
  used[root] = true;
  int q[nodes], qh = 0, qt = 0;
  q[qt++] = root;
  while (qh < qt) {
    int v = q[qh++];
    for (int j = 0, to; j < adj[v].size(); j++) {
      to = adj[v][j];
      if (base[v] == base[to] || match[v] == to) continue;
      if (to == root || match[to] != -1 && p[match[to]] != -1) {
        int curbase = lca(nodes, v, to);
        vector<bool> blossom(nodes);
        mark_path(blossom, v, curbase, to);
        mark_path(blossom, to, curbase, v);
        for (int i = 0; i < nodes; i++)
          if (blossom[base[i]]) {
            base[i] = curbase;
            if (!used[i]) {
              used[i] = true;
              q[qt++] = i;
            }
          }
      } else if (p[to] == -1) {
        p[to] = v;
        if (match[to] == -1) return to;
        to = match[to];
        used[to] = true;
        q[qt++] = to;
      }
    }
  }
  return -1;
}

int edmonds(int nodes) {
  for (int i = 0; i < nodes; i++) match[i] = -1;
  for (int i = 0; i < nodes; i++) {
    if (match[i] == -1) {
      int v, pv, ppv;
      for (v = find_path(nodes, i); v != -1; v = ppv) {
        ppv = match[pv = p[v]];
        match[v] = pv;
        match[pv] = v;
      }
    }
  }
  int matches = 0;
  for (int i = 0; i < nodes; i++)
    if (match[i] != -1) matches++;
  return matches / 2;
}

int main() {
  int nodes, edges, u, v;
  cin >> nodes >> edges;
  for (int i = 0; i < edges; i++) {
    cin >> u >> v;
    adj[u].push_back(v);
  }
  cout << "Matched " << edmonds(nodes);
  cout << " pair(s). Matchings are:\n";
  for (int i = 0; i < nodes; i++) {
    if (match[i] != -1 && i < match[i])
      cout << i << " " << match[i] << "\n";
  }
  return 0;
}
