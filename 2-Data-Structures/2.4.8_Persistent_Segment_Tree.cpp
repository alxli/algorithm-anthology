/*

Maintains immutable versions of a segment tree under point updates. Each update creates a new root
by copying only the nodes on the path from the root to the updated leaf, while all unchanged
subtrees are shared with previous versions.

The implementation below stores sums over a fixed index range $[0, n)$. Persistent segment trees are
useful for offline order-statistics queries, rollback-like histories, and functional dynamic
programming states.

- `PersistentSegTree(a)` constructs version 0 from array `a`.
- `update(version, index, value)` returns a new version where `a[index]` is set to `value`, leaving
  the old version unchanged.
- `query(version, lo, hi)` returns the sum of the inclusive range `[lo, hi]` in the specified
  version.
- `versions()` returns the number of stored roots.

Time Complexity:
- O(n) for construction.
- O(log n) per call to `update(version, index, value)` and `query(version, lo, hi)`.

Space Complexity:
- O(n + q log n) for $q$ updates.
- O(log n) auxiliary stack space per update and query.

*/

#include <cstdint>
#include <vector>

class PersistentSegTree {
  struct node {
    int64_t sum;
    int left, right;

    node(int64_t sum = 0, int left = -1, int right = -1) : sum(sum), left(left), right(right) {}
  };

  int n;
  std::vector<node> tree;
  std::vector<int> root;

  int build(const std::vector<int> &a, int lo, int hi) {
    if (lo == hi) {
      tree.emplace_back(a[lo]);
      return static_cast<int>(tree.size()) - 1;
    }
    int mid = lo + (hi - lo) / 2;
    int left = build(a, lo, mid);
    int right = build(a, mid + 1, hi);
    tree.emplace_back(tree[left].sum + tree[right].sum, left, right);
    return static_cast<int>(tree.size()) - 1;
  }

  int update(int cur, int lo, int hi, int index, int value) {
    if (lo == hi) {
      tree.emplace_back(value);
      return static_cast<int>(tree.size()) - 1;
    }
    int mid = lo + (hi - lo) / 2;
    int left = tree[cur].left, right = tree[cur].right;
    if (index <= mid) {
      left = update(left, lo, mid, index, value);
    } else {
      right = update(right, mid + 1, hi, index, value);
    }
    tree.emplace_back(tree[left].sum + tree[right].sum, left, right);
    return static_cast<int>(tree.size()) - 1;
  }

  int64_t query(int cur, int lo, int hi, int qlo, int qhi) const {
    if (qlo <= lo && hi <= qhi) {
      return tree[cur].sum;
    }
    int mid = lo + (hi - lo) / 2;
    int64_t res = 0;
    if (qlo <= mid) {
      res += query(tree[cur].left, lo, mid, qlo, qhi);
    }
    if (mid < qhi) {
      res += query(tree[cur].right, mid + 1, hi, qlo, qhi);
    }
    return res;
  }

 public:
  explicit PersistentSegTree(const std::vector<int> &a) : n(a.size()) {
    if (n > 0) {
      root.push_back(build(a, 0, n - 1));
    }
  }

  int versions() const { return static_cast<int>(root.size()); }

  int update(int version, int index, int value) {
    root.push_back(update(root[version], 0, n - 1, index, value));
    return static_cast<int>(root.size()) - 1;
  }

  int64_t query(int version, int lo, int hi) const {
    return query(root[version], 0, n - 1, lo, hi);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{1, 2, 3, 4};
  PersistentSegTree t(a);
  int v1 = t.update(0, 1, 10);
  int v2 = t.update(v1, 3, -1);
  assert(t.query(0, 0, 3) == 10);
  assert(t.query(v1, 0, 3) == 18);
  assert(t.query(v2, 1, 3) == 12);
  assert(t.versions() == 3);
  return 0;
}
