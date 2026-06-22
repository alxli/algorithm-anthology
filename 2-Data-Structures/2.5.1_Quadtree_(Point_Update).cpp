/*

Maintain a two-dimensional array over a huge grid while supporting point updates and rectangle
queries. This is a sparse (a.k.a. dynamic or implicit) quadtree: it recursively splits each
rectangle into four quadrants, and lazily allocates only the nodes touched by updates or needed to
summarize queried regions.

The query operation is defined by a commutative associative aggregate function `combine(a, b)`.
Because untouched regions are implicit, `repeat_value(v, area)` must return the aggregate summary of
`area` copies of the initial value `v`. The default code below assumes a numerical array type,
defining queries for the "min" of the target range. For rectangle-sum queries, `combine(a, b)`
should return `a + b` and `repeat_value(v, area)` should return `v * area`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at one
updated cell. The default code below defines updates that "set" the chosen cell to a new value.
Another possible update operation is "increment", in which case `apply_delta(v, d)` should return
`v + d`.

Choose this over a sparse 2D segment tree when the grid is huge and touched cells or queried
rectangles are sparse or naturally align with large quadrants. Choose the sparse 2D segment tree
instead when many thin or adversarial rectangles are expected and a predictable O(log(R)*log(C))
bound matters more than lower constant factors on aligned regions.

- `Quadtree<T, R, C>(v)` constructs a two-dimensional array with rows from 0 to $R$ (inclusive) and
  columns from 0 to $C$ (inclusive). All array values are initialized to `v`.
- `at(r, c)` returns the value at row `r`, column `c`.
- `query(r1, c1, r2, c2)` returns the result of `combine()` applied to every value in the
  rectangular region consisting of rows from `r1` to `r2`, inclusive, and columns from `c1` to `c2`,
  inclusive.
- `update(r, c, d)` assigns the value `v` at (`r`, `c`) to `apply_delta(v, d)`.

Time Complexity:
- O(1) per call to the constructor.
- O(log(max(R, C))) per call to `at()` and `update()`, which descend a single root-to-cell path.
- O(R + C) worst case per call to `query()`, attained when the queried rectangle straddles the
  quadrant boundaries at every level (for example a one-cell-thick full-width strip); rectangles
  that align with a few large quadrants are far cheaper.

Space Complexity:
- O(min(n*log(max(R, C)), R*C)) for storage of quadtree nodes after $n$ point updates. More
  precisely, since bounds are inclusive, the dense-grid ceiling is $(R + 1)(C + 1)$ cells.
- O(log(max(R, C))) auxiliary stack space for `update()`, `query()`, and `at()`.

*/

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>

template<typename T, int R = 1000000000, int C = 1000000000>
class Quadtree {
  static_assert(R >= 0 && C >= 0);

  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T repeat_value(const T &v, int64_t area) { return v; }
  static T apply_delta(const T &v, const T &d) { return d; }

  struct Node {
    T value;
    Node *child[4];

    explicit Node(const T &v) : value(v) {
      for (auto &c : child) {
        c = nullptr;
      }
    }
  };

  Node *root;
  T init;

  // Helper variables for query().
  int tgt_r1, tgt_c1, tgt_r2, tgt_c2;
  T res;
  bool found;

  static int64_t area(int r1, int c1, int r2, int c2) {
    return static_cast<int64_t>(r2 - r1 + 1) * (c2 - c1 + 1);
  }

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

  // Helper variables for update().
  int tgt_r, tgt_c;
  T delta;

  void update(Node *&n, int r1, int c1, int r2, int c2) {
    if (tgt_r < r1 || tgt_r > r2 || tgt_c < c1 || tgt_c > c2) {
      return;
    }
    if (n == nullptr) {
      n = new Node(repeat_value(init, area(r1, c1, r2, c2)));
    }
    if (r1 == r2 && c1 == c2) {
      n->value = apply_delta(n->value, delta);
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
  explicit Quadtree(const T &v = T()) : root(nullptr), init(v) {}

  ~Quadtree() { clean_up(root); }
  Quadtree(const Quadtree &) = delete;
  Quadtree &operator=(const Quadtree &) = delete;

  T at(int r, int c) {
    assert(0 <= r && r <= R && 0 <= c && c <= C);
    return query(r, c, r, c);
  }

  T query(int r1, int c1, int r2, int c2) {
    assert(0 <= r1 && r1 <= r2 && r2 <= R);
    assert(0 <= c1 && c1 <= c2 && c2 <= C);
    tgt_r1 = r1;
    tgt_c1 = c1;
    tgt_r2 = r2;
    tgt_c2 = c2;
    found = false;
    query(root, 0, 0, R, C);
    return found ? res : repeat_value(init, area(r1, c1, r2, c2));
  }

  void update(int r, int c, const T &d) {
    assert(0 <= r && r <= R && 0 <= c && c <= C);
    tgt_r = r;
    tgt_c = c;
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

#include <iostream>
using namespace std;

int main() {
  Quadtree<int> t(0);
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
