/*

Maintain a two-dimensional array over a huge grid while supporting dynamic queries of rectangular
subarrays and dynamic updates of individual indices. This is a sparse (a.k.a. dynamic or implicit)
2D segment tree: row and column nodes are allocated lazily as cells are touched, so large coordinate
bounds are supported without allocating the full grid.

The query operation is defined by a commutative associative aggregate function `combine(a, b)`.
Because untouched regions are implicit, `repeat_value(v, area)` must return the aggregate summary of
`area` copies of the initial value `v`. The default code below assumes a numerical array type,
defining queries for the "min" of the target range. For rectangle-sum queries, `combine(a, b)`
should return `a + b` and `repeat_value(v, area)` should return `v * area`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at one
updated cell. The default code below defines updates that "set" the chosen cell to a new value.
Another possible update operation is "increment", in which case `apply_delta(v, d)` should return
`v + d`.

Compared with the quadtrees in the previous sections, this structure splits rows and columns
independently, so every rectangle query decomposes into O(log(R)*log(C)) canonical rectangles. That
makes it preferable when queries may be thin, off-center, or adversarially placed: a quadtree can
degrade to visiting O(R + C) boundary nodes for such rectangles. The trade-off is a larger tree and
less benefit from queries that happen to align with big square quadrants.

For dense additive rectangle sums, prefer the simple 2D Fenwick tree in 2.6.5. A dense vector-backed
2D segment tree can be simpler and faster on modest grids with custom aggregates such as min/max,
but it needs O(R*C) storage with large constants, so this sparse version is usually the safer
codebook default.

- `SegTree2D<T, R, C>(v)` constructs a two-dimensional array with rows from 0 to $R$ (inclusive)
  and columns from 0 to $C$ (inclusive).
  All array values are implicitly initialized to `v`. Nodes are allocated lazily as indices are
  touched.
- `at(r, c)` returns the value at row `r`, column `c`.
- `query(r1, c1, r2, c2)` returns the result of `combine()` applied to every value in the
  rectangular region consisting of rows from `r1` to `r2`, inclusive, and columns from `c1` to `c2`,
  inclusive.
- `update(r, c, d)` assigns the value `v` at (`r`, `c`) to `apply_delta(v, d)`.

Time Complexity:
- O(1) per call to the constructor.
- O(log(R)*log(C)) per call to `at()`, `update()`, and `query()`.

Space Complexity:
- O(n log(R) log(C)) for storage after $n$ point updates.
- O(log(R) + log(C)) auxiliary stack space for `update()`, `query()`, and `at()`.

*/

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>

template<typename T, int R = 1000000000, int C = 1000000000>
class SegTree2D {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T repeat_value(const T &v, int64_t area) { return v; }
  static T apply_delta(const T &v, const T &d) { return d; }

  struct InnerNode {
    T value;
    int low, high;
    InnerNode *left, *right;

    InnerNode(int lo, int hi, const T &v)
        : value(v), low(lo), high(hi), left(nullptr), right(nullptr) {}
  };

  struct OuterNode {
    InnerNode root;
    int low, high;
    OuterNode *left, *right;

    OuterNode(int lo, int hi, const T &v)
        : root(0, C, v), low(lo), high(hi), left(nullptr), right(nullptr) {}
  } *root;

  T init;

  // Helper variables for query() and update().
  int tgt_r1, tgt_c1, tgt_r2, tgt_c2;
  int64_t width;

  static int64_t length(int lo, int hi) { return hi - lo + 1LL; }

  void append_result(std::optional<T> &res, const T &v) { res = res ? combine(*res, v) : v; }

  T query(InnerNode *n, int qlo, int qhi, int64_t rows) {
    if (n == nullptr) {
      return repeat_value(init, rows * length(qlo, qhi));
    }
    int lo = n->low, hi = n->high, mid = lo + (hi - lo) / 2;
    std::optional<T> res;
    if (qlo < lo) {
      int missing_hi = std::min(qhi, lo - 1);
      append_result(res, repeat_value(init, rows * length(qlo, missing_hi)));
    }
    int overlap_lo = std::max(qlo, lo), overlap_hi = std::min(qhi, hi);
    if (overlap_lo <= overlap_hi) {
      if (overlap_lo == lo && overlap_hi == hi) {
        append_result(res, n->value);
      } else {
        if (overlap_lo <= mid) {
          append_result(res, query(n->left, overlap_lo, std::min(overlap_hi, mid), rows));
        }
        if (mid < overlap_hi) {
          append_result(res, query(n->right, std::max(overlap_lo, mid + 1), overlap_hi, rows));
        }
      }
    }
    if (hi < qhi) {
      int missing_lo = std::max(qlo, hi + 1);
      append_result(res, repeat_value(init, rows * length(missing_lo, qhi)));
    }
    return *res;
  }

  T query(OuterNode *n, int qlo, int qhi) {
    if (n == nullptr) {
      return repeat_value(init, width * length(qlo, qhi));
    }
    int lo = n->low, hi = n->high, mid = lo + (hi - lo) / 2;
    std::optional<T> res;
    if (qlo < lo) {
      int missing_hi = std::min(qhi, lo - 1);
      append_result(res, repeat_value(init, width * length(qlo, missing_hi)));
    }
    int overlap_lo = std::max(qlo, lo), overlap_hi = std::min(qhi, hi);
    if (overlap_lo <= overlap_hi) {
      if (overlap_lo == lo && overlap_hi == hi) {
        append_result(res, query(&(n->root), tgt_c1, tgt_c2, length(lo, hi)));
      } else {
        if (overlap_lo <= mid) {
          append_result(res, query(n->left, overlap_lo, std::min(overlap_hi, mid)));
        }
        if (mid < overlap_hi) {
          append_result(res, query(n->right, std::max(overlap_lo, mid + 1), overlap_hi));
        }
      }
    }
    if (hi < qhi) {
      int missing_lo = std::max(qlo, hi + 1);
      append_result(res, repeat_value(init, width * length(missing_lo, qhi)));
    }
    return *res;
  }

  void update(InnerNode *n, int c, const T &d, bool leaf_row, int64_t rows) {
    int lo = n->low, hi = n->high, mid = lo + (hi - lo) / 2;
    if (lo == hi) {
      if (leaf_row) {
        n->value = apply_delta(n->value, d);
      } else {
        n->value = d;
      }
      return;
    }
    InnerNode *&target = (c <= mid) ? n->left : n->right;
    if (target == nullptr) {
      target = new InnerNode(c, c, repeat_value(init, rows));
    }
    if (target->low <= c && c <= target->high) {
      update(target, c, d, leaf_row, rows);
    } else {
      int split_lo = lo, split_hi = hi, split_mid = mid;
      do {
        if (c <= split_mid) {
          split_hi = split_mid;
        } else {
          split_lo = split_mid + 1;
        }
        split_mid = split_lo + (split_hi - split_lo) / 2;
      } while ((c <= split_mid) == (target->low <= split_mid));
      InnerNode *tmp =
          new InnerNode(split_lo, split_hi, repeat_value(init, rows * length(split_lo, split_hi)));
      if (target->low <= split_mid) {
        tmp->left = target;
      } else {
        tmp->right = target;
      }
      target = tmp;
      update(tmp, c, d, leaf_row, rows);
    }
    T left_value =
        (n->left != nullptr) ? n->left->value : repeat_value(init, rows * length(lo, mid));
    T right_value =
        (n->right != nullptr) ? n->right->value : repeat_value(init, rows * length(mid + 1, hi));
    n->value = combine(left_value, right_value);
  }

  void update(OuterNode *n, int r, int c, const T &d) {
    int lo = n->low, hi = n->high, mid = lo + (hi - lo) / 2;
    int64_t rows = length(lo, hi);
    if (lo == hi) {
      update(&(n->root), c, d, true, 1);
      return;
    }
    if (r <= mid) {
      if (n->left == nullptr) {
        n->left = new OuterNode(lo, mid, repeat_value(init, length(lo, mid) * length(0, C)));
      }
      update(n->left, r, c, d);
    } else {
      if (n->right == nullptr) {
        n->right =
            new OuterNode(mid + 1, hi, repeat_value(init, length(mid + 1, hi) * length(0, C)));
      }
      update(n->right, r, c, d);
    }
    T value = repeat_value(init, rows);
    if (n->left != nullptr || n->right != nullptr) {
      tgt_c1 = tgt_c2 = c;
      T left_value = (n->left != nullptr) ? query(&(n->left->root), c, c, length(lo, mid))
                                          : repeat_value(init, length(lo, mid));
      T right_value = (n->right != nullptr) ? query(&(n->right->root), c, c, length(mid + 1, hi))
                                            : repeat_value(init, length(mid + 1, hi));
      value = combine(left_value, right_value);
    }
    update(&(n->root), c, value, false, rows);
  }

  static void clean_up(InnerNode *n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

  static void clean_up(OuterNode *n) {
    if (n != nullptr) {
      clean_up(n->root.left);
      clean_up(n->root.right);
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  explicit SegTree2D(const T &v = T())
      : root(new OuterNode(0, R, repeat_value(v, length(0, R) * length(0, C)))), init(v) {}

  ~SegTree2D() { clean_up(root); }
  SegTree2D(const SegTree2D &) = delete;
  SegTree2D &operator=(const SegTree2D &) = delete;
  T at(int r, int c) { return query(r, c, r, c); }

  T query(int r1, int c1, int r2, int c2) {
    tgt_r1 = r1;
    tgt_c1 = c1;
    tgt_r2 = r2;
    tgt_c2 = c2;
    width = length(c1, c2);
    return query(root, r1, r2);
  }

  void update(int r, int c, const T &d) { update(root, r, c, d); }
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
  SegTree2D<int> t(0);
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
