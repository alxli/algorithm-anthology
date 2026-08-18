/*

Maintain immutable versions of a fixed-size array while supporting range updates and range aggregate
queries. Each update creates a new version by copying only the O(log n) nodes on the path it
descends, sharing every unchanged subtree with the version it came from. Earlier versions are never
disturbed, and an update may branch from any of them, so versions form a tree rather than a line.

Lazy propagation normally works by pushing a pending update down into both children before
descending, which rewrites nodes that older versions still point at and so would destroy them here.
The fix is to never push. A node keeps its pending delta permanently, and a query accumulates the
deltas it passes on the way down, adding each one's contribution to the span it covers. Since a node
is only ever read together with its ancestors, an update that stops at a node is still seen by every
query that reaches below it, and the nodes above are the only ones that need copying.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
code below assumes a numerical array type, defining queries for the "sum" of the target range.
Another possible query operation is "min", in which case `combine(a, b)` should return `min(a, b)`.

Range updates are defined by `apply_delta(v, d, len)`, which applies an update delta `d` to an
aggregate summary `v` representing `len` array values, and by `compose_deltas(old, d)`, which
combines a pending older delta with a newer delta in that order. These functions do not support
arbitrary combinations: applying a delta to a combined segment must be equivalent to applying it to
each child segment and then combining the results, and composed deltas must be equivalent to
performing their updates sequentially. The default code below defines range increment for range-sum
queries. For range-min/range-max queries, `apply_delta(v, d, len)` should return `v + d`.

Deltas here must also commute, which they need not for usual lazy propagation. Namely, for every
value and length, `apply_delta(apply_delta(v, d1, len), d2, len)` must equal
`apply_delta(apply_delta(v, d2, len), d1, len)`, so that `compose_deltas(d1, d2)` and
`compose_deltas(d2, d1)` act identically on every aggregate. The reason is that an update stopping
high in the tree leaves the older deltas below it untouched, so a query composes the deltas it
passes in order of depth rather than in the order they were applied. Range increment satisfies this
and range assignment does not, which is why the latter is the default there and is unavailable here.
Pushing each delta into fresh copies of both children instead of leaving it in place lifts the
restriction, at the cost of copying two nodes per level rather than one.

- `PersistentSegTree<T>(n, v = T{})` constructs version $0$ of an array of size `n` with indices
  $[0, `n`)$, and all values initialized to `v`.
- `PersistentSegTree<T>(lo, hi)` constructs version $0$ from the half-open random-access iterator
  range $[`lo`, `hi`)$.
- `size()` returns the size of every version of the array, and `versions()` the number of stored
  versions, which occupy $[0, `versions()`)$.
- `query(version, lo, hi)` returns the aggregate of the values at indices in $[`lo`, `hi`]$ in the
  given version. If `lo == hi`, then the single specified value is returned.
- `update(version, lo, hi, d)` creates and returns a new version in which the delta `d` has been
  applied to the value at each array index in $[`lo`, `hi`]$. The given version is unchanged.

Time Complexity:
- O(n) per call to either constructor, where $n$ is the size of the array.
- O(1) per call to `size()` and `versions()`.
- O(log n) per call to `query()` and `update()`.

Space Complexity:
- O(n + q log n) for storage after $q$ updates.
- O(log n) auxiliary stack space per call to `query()` and `update()`.

*/

#include <cassert>
#include <optional>
#include <vector>

template<typename T>
class PersistentSegTree {
  static T combine(const T &a, const T &b) { return a + b; }
  static T apply_delta(const T &v, const T &d, int len) { return v + d * len; }
  static T compose_deltas(const T &d1, const T &d2) { return d1 + d2; }

  struct Node {
    T value;
    std::optional<T> delta;  // Empty when no update is pending on this node's descendants.
    int left, right;

    Node(const T &value, const std::optional<T> &delta, int left = -1, int right = -1)
        : value(value), delta(delta), left(left), right(right) {}
  };

  int len;
  std::vector<Node> nodes;
  std::vector<int> roots;

  int make(const T &value, const std::optional<T> &delta, int left, int right) {
    nodes.emplace_back(value, delta, left, right);
    return static_cast<int>(nodes.size()) - 1;
  }

  template<typename Gen>
  int build(int lo, int hi, const Gen &gen) {
    if (lo == hi) {
      return make(gen(lo), std::nullopt, -1, -1);
    }
    int mid = lo + (hi - lo) / 2;
    int left = build(lo, mid, gen), right = build(mid + 1, hi, gen);
    return make(combine(nodes[left].value, nodes[right].value), std::nullopt, left, right);
  }

  // The carried argument accumulates every ancestor's delta, since none are pushed down.
  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi, const std::optional<T> &carried) const {
    if (tgt_lo <= lo && hi <= tgt_hi) {
      return carried ? apply_delta(nodes[i].value, *carried, hi - lo + 1) : nodes[i].value;
    }
    int mid = lo + (hi - lo) / 2;
    std::optional<T> below = carried;
    if (nodes[i].delta) {
      below = below ? compose_deltas(*below, *nodes[i].delta) : *nodes[i].delta;
    }
    if (tgt_hi <= mid) {
      return query(nodes[i].left, lo, mid, tgt_lo, tgt_hi, below);
    }
    if (tgt_lo > mid) {
      return query(nodes[i].right, mid + 1, hi, tgt_lo, tgt_hi, below);
    }
    return combine(
        query(nodes[i].left, lo, mid, tgt_lo, mid, below),
        query(nodes[i].right, mid + 1, hi, mid + 1, tgt_hi, below)
    );
  }

  int update(int i, int lo, int hi, int tgt_lo, int tgt_hi, const T &d) {
    if (tgt_lo <= lo && hi <= tgt_hi) {
      // The delta stays here; every query below this node will pick it up on the way past.
      return make(
          apply_delta(nodes[i].value, d, hi - lo + 1),
          nodes[i].delta ? compose_deltas(*nodes[i].delta, d) : d, nodes[i].left, nodes[i].right
      );
    }
    int mid = lo + (hi - lo) / 2;
    int left = nodes[i].left, right = nodes[i].right;
    if (tgt_lo <= mid) {
      left = update(left, lo, mid, tgt_lo, tgt_hi, d);
    }
    if (tgt_hi > mid) {
      right = update(right, mid + 1, hi, tgt_lo, tgt_hi, d);
    }
    T merged = combine(nodes[left].value, nodes[right].value);
    return make(
        nodes[i].delta ? apply_delta(merged, *nodes[i].delta, hi - lo + 1) : merged, nodes[i].delta,
        left, right
    );
  }

 public:
  explicit PersistentSegTree(int n, const T &v = T{}) : len(n) {
    assert(n > 0);
    roots.push_back(build(0, n - 1, [&](int) { return v; }));
  }

  template<typename It>
  PersistentSegTree(It lo, It hi) : len(static_cast<int>(hi - lo)) {
    assert(len > 0);
    roots.push_back(build(0, len - 1, [&](int i) { return *(lo + i); }));
  }

  int size() const { return len; }
  int versions() const { return static_cast<int>(roots.size()); }

  T query(int version, int lo, int hi) const {
    assert(version >= 0 && version < versions() && lo >= 0 && lo <= hi && hi < len);
    return query(roots[version], 0, len - 1, lo, hi, std::nullopt);
  }

  int update(int version, int lo, int hi, const T &d) {
    assert(version >= 0 && version < versions() && lo >= 0 && lo <= hi && hi < len);
    roots.push_back(update(roots[version], 0, len - 1, lo, hi, d));
    return versions() - 1;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{1, 2, 3, 4, 5};
  PersistentSegTree<int> t(a.begin(), a.end());
  assert(t.size() == 5 && t.versions() == 1);
  assert(t.query(0, 0, 4) == 15 && t.query(0, 1, 3) == 9);

  int v1 = t.update(0, 1, 3, 10);  // Adds 10 to indices 1, 2, and 3.
  assert(v1 == 1);
  assert(t.query(1, 1, 3) == 39 && t.query(1, 0, 4) == 45);
  assert(t.query(1, 0, 0) == 1 && t.query(1, 4, 4) == 5);
  assert(t.query(0, 1, 3) == 9);  // Version 0 is untouched.

  // Updates compose, and any version may be branched from more than once.
  int v2 = t.update(1, 0, 4, 1);
  assert(t.query(2, 0, 4) == 50);
  int v3 = t.update(1, 0, 1, 100);
  assert(v3 == 3 && t.query(3, 0, 1) == 213 && t.query(2, 0, 1) == 15);

  // A partially overlapping update reaches the same leaves through different nodes.
  int v4 = t.update(3, 1, 2, 5);
  assert(t.query(4, 1, 1) == 117 && t.query(4, 2, 2) == 18 && t.query(4, 3, 3) == 14);
  assert(t.versions() == 5 && v2 == 2 && v4 == 4);
  return 0;
}
