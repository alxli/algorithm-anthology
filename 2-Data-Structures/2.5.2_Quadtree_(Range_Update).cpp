/*

Maintain a two-dimensional array over a huge grid while supporting rectangle updates and rectangle
queries. This is a sparse (a.k.a. dynamic or implicit) quadtree: it recursively splits each
rectangle into four quadrants, lazily allocates touched nodes, and stores pending rectangle updates
with lazy propagation.

The query operation is defined by a commutative associative aggregate function `combine(a, b)`.
Because untouched regions are implicit, `repeat_value(v, area)` must return the aggregate summary of
`area` copies of the initial value `v`. The default code below assumes a numerical array type,
defining queries for the "min" of the target range. For rectangle-sum queries, `combine(a, b)`
should return `a + b` and `repeat_value(v, area)` should return `v * area`.

Range updates are defined by `apply_delta(v, d, area)`, which applies an update delta `d` to an
aggregate summary `v` representing `area` cells, and by `compose_deltas(old, d)`, which combines a
pending older delta with a newer delta in that order. These functions do not support arbitrary
combinations: applying a delta to a combined region must be equivalent to applying it to each child
region and then combining the results, and composed deltas must be equivalent to performing their
updates sequentially. The default code below defines rectangle assignment. For rectangle increment,
`compose_deltas(old, d)` should return `old + d`; `apply_delta(v, d, area)` should return `v + d`
for range-min/range-max queries, and `v + d * area` for range-sum queries.

Choose this over a sparse 2D segment tree when rectangle updates or queries often cover large
aligned regions of a huge sparse grid. Choose a sparse 2D segment tree when worst-case rectangles
may be long, thin, or otherwise poorly aligned with quadrant boundaries; the quadtree can visit many
boundary nodes in those cases.

- `LazyQuadtree<T, R, C>(v)` constructs a 2D array with rows 0 to $R$ (inclusive) and columns 0 to
  $C$ (inclusive), where template parameters `R` and `C`. All array values are implicitly
  initialized to `v`. Nodes are allocated lazily as indices are touched.
- `at(r, c)` returns the value at row `r`, column `c`.
- `query(r1, c1, r2, c2)` returns the result of `combine()` applied to every value in the
  rectangular region consisting of rows from `r1` to `r2` and columns from `c1` to `c2`, inclusive.
- `update(r, c, d)` assigns the value `v` at `(r, c)` to `apply_delta(v, d)`.
- `update(r1, c1, r2, c2)` modifies the value at each index of the rectangular region consisting of
  rows from `r1` to `r2` and columns from `c1` to `c2`, inclusive, by applying the delta `d` to each
  value.

Time Complexity:
- O(1) per call to the constructor.
- O(log(max(R, C))) per call to `at()`, which descends a single root-to-cell path.
- O(R + C) worst case per call to `update()` and `query()`, attained when the rectangle straddles
  quadrant boundaries at many levels; rectangles that align with a few large quadrants are far
  cheaper.

Space Complexity:
- O(n (R + C)) worst-case storage after $n$ rectangle updates, although aligned rectangles and point
  updates allocate far fewer nodes.
- O(log(max(R, C))) auxiliary stack space for `update()`, `query()`, and `at()`.

*/

#include <algorithm>
#include <cstddef>
#include <cstdint>

template<class T, int R = 1000000000, int C = 1000000000>
class LazyQuadtree {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T repeat_value(const T &v, int64_t area) { return v; }
  static T apply_delta(const T &v, const T &d, int64_t area) { return d; }
  static T compose_deltas(const T &d1, const T &d2) { return d2; }

  struct Node {
    T value, delta;
    bool pending;
    Node *child[4];

    explicit Node(const T &v) : value(v), pending(false) {
      for (auto &c : child) {
        c = nullptr;
      }
    }
  } *root;

  T init;

  static int64_t area(int r1, int c1, int r2, int c2) {
    return static_cast<int64_t>(r2 - r1 + 1) * (c2 - c1 + 1);
  }

  void update_delta(Node *&n, const T &d, int64_t area) {
    if (n == nullptr) {
      n = new Node(repeat_value(init, area));
    }
    n->delta = n->pending ? compose_deltas(n->delta, d) : d;
    n->pending = true;
  }

  void push_delta(Node *n, int r1, int c1, int r2, int c2) {
    if (n->pending) {
      int rmid = r1 + (r2 - r1) / 2, cmid = c1 + (c2 - c1) / 2;
      int64_t cur_area = area(r1, c1, r2, c2);
      n->value = apply_delta(n->value, n->delta, cur_area);
      if (cur_area > 1) {
        int rlen = r2 - r1 + 1, clen = c2 - c1 + 1;
        int rlen1 = rmid - r1 + 1, rlen2 = rlen - rlen1;
        int clen1 = cmid - c1 + 1, clen2 = clen - clen1;
        update_delta(n->child[0], n->delta, static_cast<int64_t>(rlen1) * clen1);
        update_delta(n->child[1], n->delta, static_cast<int64_t>(rlen2) * clen1);
        update_delta(n->child[2], n->delta, static_cast<int64_t>(rlen1) * clen2);
        update_delta(n->child[3], n->delta, static_cast<int64_t>(rlen2) * clen2);
      }
      n->pending = false;
    }
  }

  // Helper variables for query() and update().
  int tgt_r1, tgt_c1, tgt_r2, tgt_c2;
  T res, delta;
  bool found;

  void append_result(const T &v) {
    res = found ? combine(res, v) : v;
    found = true;
  }

  T child_value(Node *n, int r1, int c1, int r2, int c2) {
    return (n != nullptr) ? n->value : repeat_value(init, area(r1, c1, r2, c2));
  }

  void query(Node *n, int r1, int c1, int r2, int c2) {
    if (tgt_r2 < r1 || tgt_r1 > r2 || tgt_c2 < c1 || tgt_c1 > c2) {
      return;
    }
    if (n == nullptr) {
      int rlen = std::min(r2, tgt_r2) - std::max(r1, tgt_r1) + 1;
      int clen = std::min(c2, tgt_c2) - std::max(c1, tgt_c1) + 1;
      append_result(repeat_value(init, static_cast<int64_t>(rlen) * clen));
      return;
    }
    push_delta(n, r1, c1, r2, c2);
    if (tgt_r1 <= r1 && r2 <= tgt_r2 && tgt_c1 <= c1 && c2 <= tgt_c2) {
      append_result(n->value);
      return;
    }
    int rmid = r1 + (r2 - r1) / 2, cmid = c1 + (c2 - c1) / 2;
    query(n->child[0], r1, c1, rmid, cmid);
    query(n->child[1], rmid + 1, c1, r2, cmid);
    query(n->child[2], r1, cmid + 1, rmid, c2);
    query(n->child[3], rmid + 1, cmid + 1, r2, c2);
  }

  void update(Node *&n, int r1, int c1, int r2, int c2) {
    if (n == nullptr) {
      if (tgt_r2 < r1 || tgt_r1 > r2 || tgt_c2 < c1 || tgt_c1 > c2) {
        return;
      }
      n = new Node(repeat_value(init, area(r1, c1, r2, c2)));
    } else {
      push_delta(n, r1, c1, r2, c2);
    }
    if (tgt_r2 < r1 || tgt_r1 > r2 || tgt_c2 < c1 || tgt_c1 > c2) {
      return;
    }
    if (tgt_r1 <= r1 && r2 <= tgt_r2 && tgt_c1 <= c1 && c2 <= tgt_c2) {
      n->delta = delta;
      n->pending = true;
      push_delta(n, r1, c1, r2, c2);
      return;
    }
    int rmid = r1 + (r2 - r1) / 2, cmid = c1 + (c2 - c1) / 2;
    update(n->child[0], r1, c1, rmid, cmid);
    update(n->child[1], rmid + 1, c1, r2, cmid);
    update(n->child[2], r1, cmid + 1, rmid, c2);
    update(n->child[3], rmid + 1, cmid + 1, r2, c2);
    n->value = combine(
        combine(
            child_value(n->child[0], r1, c1, rmid, cmid),
            child_value(n->child[1], rmid + 1, c1, r2, cmid)
        ),
        combine(
            child_value(n->child[2], r1, cmid + 1, rmid, c2),
            child_value(n->child[3], rmid + 1, cmid + 1, r2, c2)
        )
    );
  }

  static void clean_up(Node *n) {
    if (n != nullptr) {
      for (auto *c : n->child) {
        clean_up(c);
      }
      delete n;
    }
  }

 public:
  explicit LazyQuadtree(const T &v = T()) : root(nullptr), init(v) {}

  ~LazyQuadtree() { clean_up(root); }
  LazyQuadtree(const LazyQuadtree &) = delete;
  LazyQuadtree &operator=(const LazyQuadtree &) = delete;
  T at(int r, int c) { return query(r, c, r, c); }

  T query(int r1, int c1, int r2, int c2) {
    tgt_r1 = r1;
    tgt_c1 = c1;
    tgt_r2 = r2;
    tgt_c2 = c2;
    found = false;
    query(root, 0, 0, R, C);
    return found ? res : repeat_value(init, area(r1, c1, r2, c2));
  }

  void update(int r, int c, const T &d) { update(r, c, r, c, d); }

  void update(int r1, int c1, int r2, int c2, const T &d) {
    tgt_r1 = r1;
    tgt_c1 = c1;
    tgt_r2 = r2;
    tgt_c2 = c2;
    delta = d;
    update(root, 0, 0, R, C);
  }
};

/*** Example Usage and Output:

Values:
7 6 0
5 4 0
0 1 9

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  LazyQuadtree<int> t(0);
  t.update(0, 0, 7);
  t.update(0, 1, 6);
  t.update(1, 0, 5);
  t.update(1, 1, 4);
  t.update(2, 1, 1);
  t.update(2, 2, 9);
  cout << "Values:" << endl;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      cout << t.at(i, j) << " ";
    }
    cout << endl;
  }
  assert(t.query(0, 0, 0, 1) == 6);
  assert(t.query(0, 0, 1, 0) == 5);
  assert(t.query(1, 1, 2, 2) == 0);
  assert(t.query(0, 0, 1000000000, 1000000000) == 0);
  t.update(500000000, 500000000, -100);
  assert(t.query(0, 0, 1000000000, 1000000000) == -100);
  return 0;
}
