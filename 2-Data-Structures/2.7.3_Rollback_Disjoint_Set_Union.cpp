/*

Maintains disjoint sets while supporting rollback to previous snapshots. Rollback DSU is useful for
offline dynamic connectivity, divide-and-conquer over time, and backtracking search where unions
must be undone.

Path compression is intentionally not used, because it is hard to undo. Union by size keeps tree
height logarithmic, and every successful union records enough information to restore the previous
state. Specifically, a union attaches the smaller root below the larger one and records the attached
root, its new parent, and that parent's old size. A snapshot is just the current number of history
records. Rolling back pops records until reaching that position, detaching each child, restoring the
parent size, and increasing the set count. Unions of elements already in the same set change nothing
and therefore add no history record.

- `RollbackDSU(n)` constructs `n` singleton sets over elements $[0, `n`)$.
- `find(u)` returns the representative of the set containing `u`.
- `sets()` returns the current number of disjoint sets.
- `is_united(u, v)` returns whether `u` and `v` are in the same set.
- `unite(u, v)` merges two sets and returns whether a merge occurred.
- `snapshot()` returns a token representing the current history size.
- `rollback(snapshot)` undoes all changes made after `snapshot`.

Time Complexity:
- O(n) per call to the constructor.
- O(1) per call to `sets()`.
- O(log n) worst-case per call to `find()`, `is_united()`, and `unite()`.
- O(1) per undone union during `rollback()`.

Space Complexity:
- O(n + q) for $q$ successful unions stored in the rollback history.

*/

#include <cassert>
#include <numeric>
#include <tuple>
#include <utility>
#include <vector>

class RollbackDSU {
  std::vector<int> root, size;
  std::vector<std::tuple<int, int, int>> history;  // (child, parent, old parent size)
  int num_sets;

 public:
  explicit RollbackDSU(int n) : root(n), size(n, 1), num_sets(n) {
    std::iota(root.begin(), root.end(), 0);
  }

  int find(int u) const {
    assert(0 <= u && u < static_cast<int>(root.size()));
    while (root[u] != u) {
      u = root[u];
    }
    return u;
  }

  int sets() const { return num_sets; }
  bool is_united(int u, int v) const { return find(u) == find(v); }

  bool unite(int u, int v) {
    u = find(u);
    v = find(v);
    if (u == v) {
      return false;
    }
    if (size[u] < size[v]) {
      std::swap(u, v);
    }
    history.emplace_back(v, u, size[u]);
    root[v] = u;
    size[u] += size[v];
    num_sets--;
    return true;
  }

  int snapshot() const { return static_cast<int>(history.size()); }

  void rollback(int snapshot) {
    assert(0 <= snapshot && snapshot <= static_cast<int>(history.size()));
    while (static_cast<int>(history.size()) > snapshot) {
      auto [child, parent, parent_size] = history.back();
      history.pop_back();
      root[child] = child;
      size[parent] = parent_size;
      num_sets++;
    }
  }
};

/*** Example Usage ***/

int main() {
  RollbackDSU dsu(5);
  dsu.unite(0, 1);
  int s = dsu.snapshot();
  dsu.unite(1, 2);
  dsu.unite(3, 4);
  assert(dsu.is_united(0, 2));
  assert(dsu.sets() == 2);
  dsu.rollback(s);
  assert(dsu.is_united(0, 1));
  assert(!dsu.is_united(0, 2));
  assert(!dsu.is_united(3, 4));
  assert(dsu.sets() == 4);
  return 0;
}
