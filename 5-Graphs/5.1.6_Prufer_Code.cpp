/*

Encode and decode labeled trees using Prufer codes. A Prufer code is a sequence of length $n - 2$
that uniquely represents a labeled tree on nodes $0, 1, \ldots, n - 1$. It is useful for counting
labeled trees, generating test cases, and converting between trees and compact sequences.

- `encode_prufer()` takes a tree adjacency list and returns its Prufer code.
- `decode_prufer()` takes a Prufer code and returns the corresponding tree edges.

Both functions choose the smallest available leaf at each step. This makes the implementation
deterministic and matches the usual textbook convention.

Time Complexity:
- O(n log n) per call to `encode_prufer()` or `decode_prufer()`, where $n$ is the number of nodes in
  the tree.

Space Complexity:
- O(n) auxiliary space per call to `encode_prufer()` or `decode_prufer()`.

*/

#include <set>
#include <utility>
#include <vector>

std::vector<int> encode_prufer(const std::vector<std::vector<int> > &tree) {
  int nodes = tree.size();
  std::vector<int> degree(nodes), parent(nodes, -1), code;
  std::set<int> leaves;
  if (nodes <= 2) {
    return code;
  }
  int root = 0;
  for (int u = 0; u < nodes; u++) {
    degree[u] = tree[u].size();
    if (degree[u] == 1) {
      leaves.insert(u);
    } else {
      root = u;
    }
  }
  std::vector<int> stack(1, root);
  parent[root] = -2;
  while (!stack.empty()) {
    int u = stack.back();
    stack.pop_back();
    for (int i = 0; i < (int)tree[u].size(); i++) {
      int v = tree[u][i];
      if (parent[v] == -1) {
        parent[v] = u;
        stack.push_back(v);
      }
    }
  }
  for (int i = 0; i < nodes - 2; i++) {
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

std::vector<std::pair<int, int> > decode_prufer(const std::vector<int> &code) {
  int nodes = code.size() + 2;
  std::vector<int> degree(nodes, 1);
  std::set<int> leaves;
  std::vector<std::pair<int, int> > edges;
  for (int i = 0; i < (int)code.size(); i++) {
    degree[code[i]]++;
  }
  for (int u = 0; u < nodes; u++) {
    if (degree[u] == 1) {
      leaves.insert(u);
    }
  }
  for (int i = 0; i < (int)code.size(); i++) {
    int leaf = *leaves.begin();
    leaves.erase(leaves.begin());
    int u = code[i];
    edges.push_back(std::make_pair(leaf, u));
    if (--degree[u] == 1) {
      leaves.insert(u);
    }
  }
  edges.push_back(std::make_pair(*leaves.begin(), *leaves.rbegin()));
  return edges;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<int> > tree(5);
  tree[0].push_back(3);
  tree[1].push_back(3);
  tree[2].push_back(3);
  tree[3].push_back(0);
  tree[3].push_back(1);
  tree[3].push_back(2);
  tree[3].push_back(4);
  tree[4].push_back(3);
  vector<int> code = encode_prufer(tree);
  assert(code.size() == 3 && code[0] == 3 && code[1] == 3 && code[2] == 3);
  vector<pair<int, int> > edges = decode_prufer(code);
  assert(edges.size() == 4);
  return 0;
}
