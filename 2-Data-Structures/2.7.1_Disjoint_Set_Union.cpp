/*

Maintain a set of elements partitioned into non-overlapping subsets. Each partition is assigned a
unique representative known as the parent, or root. Each subset is stored as a tree whose nodes
point toward its root: finding a representative follows parent pointers, redirecting visited nodes
straight to the root along the way (path compression), while unions attach the smaller tree beneath
the root of the larger one (union-by-size). Together these keep the trees nearly flat. Union-by-size
is interchangeable with the union-by-rank strategy used by Sparse Disjoint Set Union and yields the
same complexity bounds, but union-by-size additionally supports `set_size()` queries.

This version is simplified to work only on the fixed set of integer elements 0 through `n - 1`,
chosen at construction. For arbitrary element types, see Sparse Disjoint Set Union.

- `DSU(n)` constructs `n` singleton partitions over elements 0 through `n - 1`.
- `find_root(u)` returns the unique representative of the partition containing `u`.
- `sets()` returns the current number of partitions.
- `set_size(u)` returns the number of elements in the partition containing `u`.
- `is_united(u, v)` returns whether elements `u` and `v` belong to the same partition.
- `unite(u, v)` replaces the partitions containing `u` and `v` with a single new partition
  consisting of the union of elements in the original partitions, returning `true` if a merge
  occurred or `false` if `u` and `v` already belonged to the same partition.

All element arguments must lie in the range $[0, n)$.

Time Complexity:
- O(n) per call to `DSU(n)`, and O(1) per call to `sets()`.
- O(alpha(n)) per call to `find_root()`, `set_size()`, `is_united()`, and `unite()`, where $n$ is
  the number of elements, and $\alpha(n)$ is the extremely slow growing inverse of the Ackermann
  function (effectively a very small constant for all practical values of $n$).

Space Complexity:
- O(n) for storage of the disjoint set forest elements.
- O(1) auxiliary for all operations.

*/

#include <numeric>
#include <utility>
#include <vector>

class DSU {
  std::vector<int> root, size;
  int num_sets;

 public:
  explicit DSU(int n) : root(n), size(n, 1), num_sets(n) { std::iota(root.begin(), root.end(), 0); }

  int find_root(int u) {
    if (root[u] != u) {
      root[u] = find_root(root[u]);
    }
    return root[u];
  }

  int sets() const { return num_sets; }
  int set_size(int u) { return size[find_root(u)]; }
  bool is_united(int u, int v) { return find_root(u) == find_root(v); }

  bool unite(int u, int v) {
    int ru = find_root(u), rv = find_root(v);
    if (ru == rv) {
      return false;
    }
    num_sets--;
    if (size[ru] < size[rv]) {
      std::swap(ru, rv);
    }
    root[rv] = ru;
    size[ru] += size[rv];
    return true;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  DSU dsu(7);
  assert(dsu.unite(0, 1));
  assert(dsu.unite(1, 5));
  assert(dsu.unite(3, 4));
  assert(dsu.unite(3, 6));
  assert(!dsu.unite(0, 5));  // Already united.
  assert(dsu.sets() == 3);
  assert(dsu.set_size(1) == 3);
  assert(dsu.set_size(2) == 1);
  assert(dsu.is_united(0, 1));
  assert(!dsu.is_united(0, 2));
  assert(!dsu.is_united(1, 6));
  assert(dsu.is_united(4, 6));
  return 0;
}
