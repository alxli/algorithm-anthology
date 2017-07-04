/*

Given a directed graph, determine a maximal subset of its edges such that no
node is shared between different edges from the resulting subset.

Time Complexity:
- O(n^3) per call to edmonds(), where n is the number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where n the number of nodes and m is
  the number of edges
- O(n) auxiliary heap space for edmonds(), where n is the number of nodes.

*/

#include <queue>
#include <vector>

const int MAXN = 100;
std::vector<int> adj[MAXN];
int p[MAXN], base[MAXN], match[MAXN];

int lca(int nodes, int a, int b) {
  std::vector<bool> used(nodes);
  for (;;) {
    a = base[a];
    used[a] = true;
    if (match[a] == -1) {
      break;
    }
    a = p[match[a]];
  }
  for (;;) {
    b = base[b];
    if (used[b]) {
      return b;
    }
    b = p[match[b]];
  }
}

void mark_path(std::vector<bool>& blossom, int v, int b, int children) {
  for (; base[v] != b; v = p[match[v]]) {
    blossom[base[v]] = true;
    blossom[base[match[v]]] = true;
    p[v] = children;
    children = match[v];
  }
}

int find_path(int nodes, int root) {
  std::vector<bool> used(nodes);
  for (int i = 0; i < nodes; ++i) {
    p[i] = -1;
    base[i] = i;
  }
  used[root] = true;
  std::queue<int> q;
  q.push(root);
  while (!q.empty()) {
    int v = q.front();
    q.pop();
    for (int j = 0, to; j < (int)adj[v].size(); j++) {
      to = adj[v][j];
      if (base[v] == base[to] || match[v] == to) {
        continue;
      }
      if (to == root || (match[to] != -1 && p[match[to]] != -1)) {
        int currbase = lca(nodes, v, to);
        std::vector<bool> blossom(nodes);
        mark_path(blossom, v, currbase, to);
        mark_path(blossom, to, currbase, v);
        for (int i = 0; i < nodes; i++) {
          if (blossom[base[i]]) {
            base[i] = currbase;
            if (!used[i]) {
              used[i] = true;
              q.push(i);
            }
          }
        }
      } else if (p[to] == -1) {
        p[to] = v;
        if (match[to] == -1) {
          return to;
        }
        to = match[to];
        used[to] = true;
        q.push(to);
      }
    }
  }
  return -1;
}

int edmonds(int nodes) {
  for (int i = 0; i < nodes; i++) {
    match[i] = -1;
  }
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
  for (int i = 0; i < nodes; i++) {
    if (match[i] != -1) {
      matches++;
    }
  }
  return matches/2;
}

/*** Example Usage and Output:

Matched 2 pair(s). Matchings are:
0 1
2 3

***/

#include <iostream>
using namespace std;

int main() {
  int nodes = 4;
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[2].push_back(3);
  adj[3].push_back(2);
  adj[3].push_back(0);
  adj[0].push_back(3);
  cout << "Matched " << edmonds(nodes) << " pair(s). "
       << "Matchings are:" << endl;
  for (int i = 0; i < nodes; i++) {
    if (match[i] != -1 && i < match[i]) {
      cout << i << " " << match[i] << endl;
    }
  }
  return 0;
}
