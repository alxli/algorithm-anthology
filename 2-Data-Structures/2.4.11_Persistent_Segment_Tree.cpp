/*

Maintain immutable versions of a fixed-size array while supporting point updates and range aggregate
queries. Each update creates a new root by copying only the O(log n) nodes on the path from the root
to the updated leaf. All unchanged subtrees are shared with previous versions, and updates may
branch from any stored version.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case `combine(a, b)` should return `a + b`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at a
single updated index. The default definition below supports updates that "set" the chosen array
index to a new value. Another possible update operation is "increment", in which case
`apply_delta(v, d)` should return `v + d`.

- `PersistentSegTree<T>(n, v)` constructs version $0$ of an array of size `n` with indices
  [$0$, `n`), and all values initialized to `v`.
- `PersistentSegTree<T>(lo, hi)` constructs version $0$ from two random-access iterators as a range
  [`lo`, `hi`), initialized to the elements of the range in the same order.
- `size()` returns the size of every version of the array.
- `versions()` returns the number of stored versions. Versions are numbered starting from $0$.
- `at(version, i)` returns the value at index `i` in the specified version.
- `query(version, lo, hi)` returns the result of `combine()` applied to all indices from `lo` to
  `hi` in the specified version, inclusive. If `lo == hi`, then the single specified value is
  returned.
- `update(version, i, d)` creates and returns a new version in which the value at index `i` is
  assigned to `apply_delta(v, d)`. The specified version remains unchanged.

The boundary-search routines `max_right(lo, pred)` and `min_left(hi, pred)` from the recursive
point-update segment tree in 2.4.5 use the same traversal here and may be adapted by starting at
`roots[version]` and following the child indices stored in each node.

Time Complexity:
- O(n) per call to either constructor, where $n$ is the size of the array.
- O(1) per call to `size()` and `versions()`.
- O(log n) per call to `at()`, `update()`, and `query()`.

Space Complexity:
- O(n + q log n) for storage after $q$ updates.
- O(log n) auxiliary stack space for construction, `at()`, `update()`, and `query()`.
- O(1) auxiliary for `size()` and `versions()`.

*/

#include <algorithm>
#include <cassert>
#include <vector>

template<typename T>
class PersistentSegTree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &, const T &d) { return d; }

  struct Node {
    T value;
    int left, right;

    explicit Node(const T &value, int left = -1, int right = -1)
        : value(value), left(left), right(right) {}
  };

  int len;
  std::vector<Node> nodes;
  std::vector<int> roots;

  template<typename Gen>
  int build(int lo, int hi, const Gen &gen) {
    if (lo == hi) {
      nodes.emplace_back(gen(lo));
      return static_cast<int>(nodes.size()) - 1;
    }
    int mid = lo + (hi - lo) / 2;
    int left = build(lo, mid, gen);
    int right = build(mid + 1, hi, gen);
    nodes.emplace_back(combine(nodes[left].value, nodes[right].value), left, right);
    return static_cast<int>(nodes.size()) - 1;
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) const {
    if (lo == tgt_lo && hi == tgt_hi) {
      return nodes[i].value;
    }
    int mid = lo + (hi - lo) / 2;
    if (tgt_lo <= mid && mid < tgt_hi) {
      return combine(
          query(nodes[i].left, lo, mid, tgt_lo, mid),
          query(nodes[i].right, mid + 1, hi, mid + 1, tgt_hi)
      );
    }
    if (tgt_lo <= mid) {
      return query(nodes[i].left, lo, mid, tgt_lo, tgt_hi);
    }
    return query(nodes[i].right, mid + 1, hi, tgt_lo, tgt_hi);
  }

  int update(int cur, int lo, int hi, int target, const T &d) {
    if (lo == hi) {
      nodes.emplace_back(apply_delta(nodes[cur].value, d));
      return static_cast<int>(nodes.size()) - 1;
    }
    int mid = lo + (hi - lo) / 2;
    int left = nodes[cur].left, right = nodes[cur].right;
    if (target <= mid) {
      left = update(left, lo, mid, target, d);
    } else {
      right = update(right, mid + 1, hi, target, d);
    }
    nodes.emplace_back(combine(nodes[left].value, nodes[right].value), left, right);
    return static_cast<int>(nodes.size()) - 1;
  }

 public:
  explicit PersistentSegTree(int n, const T &v = T()) : len(n) {
    assert(len > 0);
    roots.push_back(build(0, len - 1, [&](int) { return v; }));
  }

  template<typename It>
  PersistentSegTree(It lo, It hi) : len(hi - lo) {
    assert(len > 0);
    roots.push_back(build(0, len - 1, [&](int i) { return *(lo + i); }));
  }

  int size() const { return len; }
  int versions() const { return static_cast<int>(roots.size()); }

  T at(int version, int i) const {
    assert(0 <= version && version < versions());
    assert(0 <= i && i < len);
    return query(version, i, i);
  }

  T query(int version, int lo, int hi) const {
    assert(0 <= version && version < versions());
    assert(0 <= lo && lo <= hi && hi < len);
    return query(roots[version], 0, len - 1, lo, hi);
  }

  int update(int version, int i, const T &d) {
    assert(0 <= version && version < versions());
    assert(0 <= i && i < len);
    roots.push_back(update(roots[version], 0, len - 1, i, d));
    return static_cast<int>(roots.size()) - 1;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  PersistentSegTree<int> t(a.begin(), a.end());
  int v1 = t.update(0, 2, 4);
  int v2 = t.update(0, 1, 7);  // Branch from version 0 rather than version 1.

  assert(t.size() == 5);
  assert(t.versions() == 3);
  assert(t.at(0, 2) == 1);
  assert(t.at(v1, 2) == 4);
  assert(t.query(0, 0, 3) == -2);
  assert(t.query(v1, 0, 3) == -2);
  assert(t.query(v2, 0, 3) == 1);
  return 0;
}
