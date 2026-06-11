/*

Maintain a set of elements partitioned into non-overlapping subsets. Each partition is assigned a
unique representative known as the parent, or root. Each subset is stored as a tree whose nodes
point toward its root: finding a representative follows parent pointers, redirecting visited nodes
straight to the root along the way (path compression), while unions attach the shallower tree
beneath the root of the deeper one (union-by-rank). Together these keep the trees nearly flat. This
version is simplified to only work on integer elements.

- `initialize()` resets the data structure.
- `make_set(u)` creates a new partition consisting of the single element `u`, which must not have
  been previously added to the data structure.
- `find_root(u)` returns the unique representative of the partition containing `u`.
- `is_united(u, v)` returns whether elements `u` and `v` belong to the same partition.
- `unite(u, v)` replaces the partitions containing `u` and `v` with a single new partition
  consisting of the union of elements in the original partitions.

A precondition to the last three operations is that `make_set()` must have been previously called on
their arguments.

Time Complexity:
- O(1) per call to `initialize()` and `make_set()`.
- O(alpha(n)) per call to `find_root()`, `is_united()`, and `unite()`, where $n$ is the number of
  elements that has been added via `make_set()` so far, and $\alpha(n)$ is the extremely slow
  growing inverse of the Ackermann function (effectively a very small constant for all practical
  values of $n$).

Space Complexity:
- O(n) for storage of the disjoint set forest elements.
- O(1) auxiliary for all operations.

*/

#include <vector>

int num_sets;
std::vector<int> root, rank;

void initialize() {
  num_sets = 0;
  root.clear();
  rank.clear();
}

void make_set(int u) {
  if (u >= static_cast<int>(root.size())) {
    root.resize(u + 1);
    rank.resize(u + 1);
  }
  root[u] = u;
  rank[u] = 0;
  num_sets++;
}

int find_root(int u) {
  if (root[u] != u) {
    root[u] = find_root(root[u]);
  }
  return root[u];
}

bool is_united(int u, int v) {
  return find_root(u) == find_root(v);
}

void unite(int u, int v) {
  int ru = find_root(u), rv = find_root(v);
  if (ru == rv) {
    return;
  }
  num_sets--;
  if (rank[ru] < rank[rv]) {
    root[ru] = rv;
  } else {
    root[rv] = ru;
    if (rank[ru] == rank[rv]) {
      rank[ru]++;
    }
  }
}

/*** Example Usage ***/

#include <cassert>

int main() {
  initialize();
  for (char c = 'a'; c <= 'g'; c++) {
    make_set(c);
  }
  unite('a', 'b');
  unite('b', 'f');
  unite('d', 'e');
  unite('d', 'g');
  assert(num_sets == 3);
  assert(is_united('a', 'b'));
  assert(!is_united('a', 'c'));
  assert(!is_united('b', 'g'));
  assert(is_united('e', 'g'));
  return 0;
}
