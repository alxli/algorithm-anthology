/*

Given an undirected graph, determine a maximum matching: a maximum subset of its edges such that no
node is shared between different edges in the resulting subset. `edmonds()` applies to a global,
pre-populated adjacency list `adj` which must only consist of nodes numbered with integers between 0
(inclusive) and the total number of nodes (exclusive), as inferred from `adj.size()`. The adjacency
list must be symmetric: for every edge `u`-`v`, both `v` is in `adj[u]` and `u` is in `adj[v]`.

Time Complexity:
- O(n^3) per call to `edmonds()`, where $n$ is the number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary heap space for `edmonds()`, where $n$ is the number of nodes.

*/

#include <algorithm>
#include <numeric>
#include <queue>
#include <vector>

std::vector<std::vector<int>> adj;
std::vector<int> p, base, match;

int lca(int u, int v) {
  int nodes = adj.size();
  std::vector<bool> used(nodes);
  for (;;) {
    u = base[u];
    used[u] = true;
    if (match[u] == -1) {
      break;
    }
    u = p[match[u]];
  }
  for (;;) {
    v = base[v];
    if (used[v]) {
      return v;
    }
    v = p[match[v]];
  }
}

void mark_path(std::vector<bool> &blossom, int u, int b, int child) {
  for (; base[u] != b; u = p[match[u]]) {
    blossom[base[u]] = true;
    blossom[base[match[u]]] = true;
    p[u] = child;
    child = match[u];
  }
}

int find_path(int root) {
  int nodes = adj.size();
  std::vector<bool> used(nodes);
  p.assign(nodes, -1);
  base.resize(nodes);
  std::iota(base.begin(), base.end(), 0);
  used[root] = true;
  std::queue<int> q;
  q.push(root);
  while (!q.empty()) {
    int u = q.front();
    q.pop();
    for (int v : adj[u]) {
      if (base[u] == base[v] || match[u] == v) {
        continue;
      }
      if (v == root || (match[v] != -1 && p[match[v]] != -1)) {
        int curr_base = lca(u, v);
        std::vector<bool> blossom(nodes);
        mark_path(blossom, u, curr_base, v);
        mark_path(blossom, v, curr_base, u);
        for (int i = 0; i < nodes; i++) {
          if (blossom[base[i]]) {
            base[i] = curr_base;
            if (!used[i]) {
              used[i] = true;
              q.push(i);
            }
          }
        }
      } else if (p[v] == -1) {
        p[v] = u;
        if (match[v] == -1) {
          return v;
        }
        v = match[v];
        used[v] = true;
        q.push(v);
      }
    }
  }
  return -1;
}

int edmonds() {
  int nodes = adj.size();
  match.assign(nodes, -1);
  for (int i = 0; i < nodes; i++) {
    if (match[i] == -1) {
      int u, pu, ppu;
      for (u = find_path(i); u != -1; u = ppu) {
        pu = p[u];
        ppu = match[pu];
        match[u] = pu;
        match[pu] = u;
      }
    }
  }
  int matches = 0;
  for (int i = 0; i < nodes; i++) {
    if (match[i] != -1) {
      matches++;
    }
  }
  return matches / 2;
}

/*** Example Usage and Output:

Matched 2 pair(s):
0 1
2 3

***/

#include <iostream>
using namespace std;

int main() {
  int nodes = 4;
  adj.resize(nodes);
  adj[0].push_back(1);
  adj[1].push_back(0);
  adj[1].push_back(2);
  adj[2].push_back(1);
  adj[2].push_back(3);
  adj[3].push_back(2);
  adj[3].push_back(0);
  adj[0].push_back(3);
  cout << "Matched " << edmonds() << " pair(s):" << endl;
  for (int i = 0; i < nodes; i++) {
    if (match[i] != -1 && i < match[i]) {
      cout << i << " " << match[i] << endl;
    }
  }
  return 0;
}
