/*

Maintains disjoint sets while supporting rollback to previous snapshots. Rollback DSU is useful for
offline dynamic connectivity, divide-and-conquer over time, and backtracking search where unions
must be undone.

Path compression is intentionally not used, because it is hard to undo. Union by size keeps tree
height logarithmic, and every successful union records enough information to restore the previous
state.

- `initialize(n)` creates singleton sets over elements `0` through `n - 1`.
- `find_root(u)` returns the representative of the set containing `u`.
- `is_united(u, v)` returns whether `u` and `v` are in the same set.
- `unite(u, v)` merges two sets and returns whether a merge occurred.
- `snapshot()` returns a token representing the current history size.
- `rollback(s)` undoes all changes made after snapshot `s`.

Time Complexity:
- O(n) per call to `initialize(n)`.
- O(log n) worst-case per call to `find_root(u)`, `is_united(u, v)`, and `unite(u, v)`.
- O(1) per undone union during `rollback(s)`.

Space Complexity:
- O(n + q) for $q$ successful unions stored in the rollback history.

*/

#include <vector>

class rollback_dsu {
  struct change {
    int child, parent, parent_size;

    change(int child = -1, int parent = -1, int parent_size = 0)
        : child(child), parent(parent), parent_size(parent_size) {}
  };

  std::vector<int> root, size;
  std::vector<change> history;
  int sets;

 public:
  rollback_dsu() : sets(0) {}

  explicit rollback_dsu(int n) {
    initialize(n);
  }

  void initialize(int n) {
    root.resize(n);
    size.assign(n, 1);
    history.clear();
    sets = n;
    for (int i = 0; i < n; i++) {
      root[i] = i;
    }
  }

  int count_sets() const {
    return sets;
  }

  int find_root(int u) const {
    while (root[u] != u) {
      u = root[u];
    }
    return u;
  }

  bool is_united(int u, int v) const {
    return find_root(u) == find_root(v);
  }

  bool unite(int u, int v) {
    u = find_root(u);
    v = find_root(v);
    if (u == v) {
      return false;
    }
    if (size[u] > size[v]) {
      int tmp = u;
      u = v;
      v = tmp;
    }
    history.push_back(change(u, v, size[v]));
    root[u] = v;
    size[v] += size[u];
    sets--;
    return true;
  }

  int snapshot() const {
    return history.size();
  }

  void rollback(int snapshot) {
    while ((int)history.size() > snapshot) {
      change c = history.back();
      history.pop_back();
      root[c.child] = c.child;
      size[c.parent] = c.parent_size;
      sets++;
    }
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  rollback_dsu dsu(5);
  dsu.unite(0, 1);
  int s = dsu.snapshot();
  dsu.unite(1, 2);
  dsu.unite(3, 4);
  assert(dsu.is_united(0, 2));
  assert(dsu.count_sets() == 2);
  dsu.rollback(s);
  assert(dsu.is_united(0, 1));
  assert(!dsu.is_united(0, 2));
  assert(!dsu.is_united(3, 4));
  assert(dsu.count_sets() == 4);
  return 0;
}
