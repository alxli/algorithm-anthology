/*

Given an undirected, unweighted graph, determine a maximum matching: a maximum subset of its edges
such that no node is shared between different edges in the resulting subset.

Edmonds' blossom algorithm extends augmenting-path search to general (non-bipartite) graphs, where
odd-length cycles called blossoms can conceal augmenting paths. Each blossom is contracted into a
single node so the search can proceed, and is expanded afterward to recover the matching. For
maximum-weight matching in bipartite graphs, use Hungarian or min-cost max-flow; for maximum-weight
matching in general graphs, use the weighted blossom algorithm in the next section.

- `blossom()` returns a matching for a global, bidirectionally pre-populated adjacency list `adj`
  which must consist of nodes numbered from 0 to `adj.size() - 1`. The returned vector `match` has
  `match[u] = v` and `match[v] = u` when `u` and `v` are matched, or `match[u] = -1` when `u` is
  unmatched.

Time Complexity:
- O(n^3) per call to `blossom()`, where $n$ is the number of nodes.

Space Complexity:
- O(max(n, m)) for storage of the graph, where $n$ is the number of nodes and $m$ is the number of
  edges.
- O(n) auxiliary heap space for `blossom()`, where $n$ is the number of nodes.

*/

#include <algorithm>
#include <numeric>
#include <queue>
#include <vector>

std::vector<int> blossom(const std::vector<std::vector<int>> &adj) {
  int nodes = static_cast<int>(adj.size());
  std::vector<int> match(nodes, -1), label(nodes), parent(nodes), base(nodes), aux(nodes, -1);
  std::queue<int> q;
  int aux_time = -1;
  auto lca = [&](int u, int v) {
    aux_time++;
    while (true) {
      if (u != -1) {
        u = base[u];
        if (aux[u] == aux_time) {
          return u;
        }
        aux[u] = aux_time;
        u = (match[u] == -1) ? -1 : parent[match[u]];
      }
      std::swap(u, v);
    }
  };
  auto mark_blossom = [&](int u, int v, int b) {
    while (base[u] != b) {
      parent[u] = v;
      v = match[u];
      if (label[v] == 1) {
        label[v] = 0;
        q.push(v);
      }
      base[u] = base[v] = b;
      u = parent[v];
    }
  };
  auto augment = [&](int u) {
    while (u != -1) {
      int p = parent[u];
      int next = match[p];
      match[u] = p;
      match[p] = u;
      u = next;
    }
  };
  auto find_path = [&](int root) {
    std::fill(label.begin(), label.end(), -1);
    std::fill(parent.begin(), parent.end(), -1);
    std::iota(base.begin(), base.end(), 0);
    while (!q.empty()) {
      q.pop();
    }
    label[root] = 0;
    q.push(root);
    while (!q.empty()) {
      int u = q.front();
      q.pop();
      for (int v : adj[u]) {
        if (base[u] == base[v] || match[u] == v) {
          continue;
        }
        if (label[v] == -1) {
          label[v] = 1;
          parent[v] = u;
          if (match[v] == -1) {
            augment(v);
            return true;
          }
          label[match[v]] = 0;
          q.push(match[v]);
        } else if (label[v] == 0) {
          int b = lca(u, v);
          mark_blossom(u, v, b);
          mark_blossom(v, u, b);
        }
      }
    }
    return false;
  };
  for (int i = 0; i < nodes; i++) {
    if (match[i] == -1) {
      for (int v : adj[i]) {
        if (i != v && match[v] == -1) {
          match[i] = v;
          match[v] = i;
          break;
        }
      }
    }
  }
  for (int i = 0; i < nodes; i++) {
    if (match[i] == -1) {
      find_path(i);
    }
  }
  return match;
}

/*** Example Usage and Output:

Matched 2 pair(s):
0 1
2 3

***/

#include <cassert>
#include <iostream>
using namespace std;

vector<vector<int>> adj;

void add_edge(int u, int v) {
  adj[u].push_back(v);
  adj[v].push_back(u);
};

int matching_size(const vector<int> &match) {
  return std::count_if(match.begin(), match.end(), [](int v) { return v != -1; }) / 2;
};

int main() {
  int nodes = 4;
  adj.assign(nodes, {});
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(2, 3);
  add_edge(3, 0);
  vector<int> match = blossom(adj);
  cout << "Matched " << matching_size(match) << " pair(s):" << endl;
  for (int i = 0; i < nodes; i++) {
    if (match[i] != -1 && i < match[i]) {
      cout << i << " " << match[i] << endl;
    }
  }
  nodes = 5;
  adj.assign(nodes, {});
  add_edge(0, 1);
  add_edge(1, 2);
  add_edge(2, 0);
  add_edge(0, 3);
  add_edge(1, 4);
  assert(matching_size(blossom(adj)) == 2);
  return 0;
}
