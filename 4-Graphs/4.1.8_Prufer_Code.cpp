/*

Encode and decode labeled trees using Prufer codes. A Prufer code is a sequence of length $n - 2$
that uniquely represents a labeled tree on nodes $0, 1, \ldots, n - 1$. It is useful for counting
labeled trees, generating test cases, and converting between trees and compact sequences. Encoding
repeatedly removes a leaf and records its neighbor until two nodes remain; decoding reverses this,
deducing each removed leaf from the node degrees implied by the code. Both functions choose the
smallest available leaf at each step. This makes the implementation deterministic and matches the
usual textbook convention.

- `encode_prufer()` returns the prufer code for the global, bidirectionally pre-populated adjacency
  list `adj` which must form a valid tree with nodes numbered $[0, `n`)$, where `n` is `adj.size()`.
- `decode_prufer()` takes a Prufer code and returns the corresponding tree edges.

Time Complexity:
- O(n log n) per call to `encode_prufer()` or `decode_prufer()`, where $n$ is the number of nodes in
  the tree.

Space Complexity:
- O(n) auxiliary per call to `encode_prufer()` or `decode_prufer()`.

*/

#include <set>
#include <utility>
#include <vector>

std::vector<int> encode_prufer(const std::vector<std::vector<int>> &adj) {
  int n = static_cast<int>(adj.size());
  std::vector<int> degree(n), parent(n, -1), code;
  std::set<int> leaves;
  if (n <= 2) {
    return code;
  }
  // Root at node n - 1, which is guaranteed to survive smallest-leaf removal (it is never the
  // smallest leaf while other nodes remain). Rooting elsewhere risks recording parent[root] == -2
  // if the chosen root itself gets stripped to a leaf (e.g. when n - 1 happens to be a leaf).
  int root = n - 1;
  for (int u = 0; u < n; u++) {
    degree[u] = static_cast<int>(adj[u].size());
    if (degree[u] == 1) {
      leaves.insert(u);
    }
  }
  std::vector<int> stack(1, root);
  parent[root] = -2;
  while (!stack.empty()) {
    int u = stack.back();
    stack.pop_back();
    for (int v : adj[u]) {
      if (parent[v] == -1) {
        parent[v] = u;
        stack.push_back(v);
      }
    }
  }
  for (int i = 0; i < n - 2; i++) {
    int leaf = *leaves.begin();
    leaves.erase(leaves.begin());
    int p = parent[leaf];
    code.push_back(p);
    if (--degree[p] == 1) {
      leaves.insert(p);
    }
  }
  return code;
}

std::vector<std::pair<int, int>> decode_prufer(const std::vector<int> &code) {
  int n = static_cast<int>(code.size()) + 2;
  std::vector<int> degree(n, 1);
  std::set<int> leaves;
  std::vector<std::pair<int, int>> edges;
  for (int c : code) {
    degree[c]++;
  }
  for (int u = 0; u < n; u++) {
    if (degree[u] == 1) {
      leaves.insert(u);
    }
  }
  for (int u : code) {
    int leaf = *leaves.begin();
    leaves.erase(leaves.begin());
    edges.emplace_back(leaf, u);
    if (--degree[u] == 1) {
      leaves.insert(u);
    }
  }
  edges.emplace_back(*leaves.begin(), *leaves.rbegin());
  return edges;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //     0
  //     |
  // 1---3---2
  //     |
  //     4
  vector<vector<int>> adj(5);
  auto add_edge = [&](int u, int v) {
    adj[u].push_back(v);
    adj[v].push_back(u);
  };
  add_edge(0, 3);
  add_edge(1, 3);
  add_edge(2, 3);
  add_edge(3, 4);
  vector<int> code = encode_prufer(adj);
  assert(code.size() == 3 && code[0] == 3 && code[1] == 3 && code[2] == 3);
  vector<pair<int, int>> edges = decode_prufer(code);
  assert(edges.size() == 4);
  return 0;
}
