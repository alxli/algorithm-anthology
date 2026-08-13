/*

Maintain a two-dimensional array over a huge grid while supporting dynamic queries of rectangular
subarrays and dynamic updates of individual indices. This is a sparse (a.k.a. dynamic or implicit)
2D segment tree: row and column nodes are allocated lazily as cells are touched, so large coordinate
bounds are supported without allocating the full grid.

The query operation is defined by a commutative associative aggregate function `combine(a, b)`.
Because untouched regions are implicit, `combine_n(v, area)` must return the aggregate summary of
`area` copies of the initial value `v`. The default code below assumes a numerical array type,
defining queries for the "min" of the target range. For rectangle-sum queries, `combine(a, b)`
should return `a + b` and `combine_n(v, area)` should return `v * area`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at one
updated cell. The default code below defines updates that "set" the chosen cell to a new value.
Another possible update operation is "increment", in which case `apply_delta(v, d)` should return
`v + d`.

Rows and columns are split independently, so every rectangle query decomposes into O(log(R)*log(C))
canonical rectangles regardless of whether it is thin, off-center, or otherwise adversarially
placed.

Use the dense 2D segment tree in the previous section when the grid is modest enough for O(R*C)
storage; it is simpler and faster. Use this sparse version when the coordinate range is huge but
only a small fraction of cells are updated. For dense additive rectangle sums, prefer the 2D Fenwick
tree in 2.6.5.

- `SparseSegTree2D<T, R, C>(v = T{})` constructs a two-dimensional array over rows $[0, `R`)$ and
  columns $[0, `C`)$. All array values are implicitly initialized to `v`. Nodes are allocated lazily
  as indices are touched.
- `at(r, c)` returns the value at row `r`, column `c`.
- `query(r1, c1, r2, c2)` returns the result of `combine()` applied to every value in the
  rectangular region consisting of rows in $[`r1`, `r2`]$ and columns in $[`c1`, `c2`]$.
- `update(r, c, d)` assigns the value `v` at (`r`, `c`) to `apply_delta(v, d)`.

Time Complexity:
- O(1) per call to the constructor.
- O(log(R)*log(C)) per call to `at()`, `query()`, and `update()`.

Space Complexity:
- O(n*log(R)*log(C)) for storage after $n$ point updates.
- O(log(R) + log(C)) auxiliary stack space for `at()`, `query()`, and `update()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>

template<typename T, int R = 1000000001, int C = 1000000001>
class SparseSegTree2D {
  static_assert(R > 0 && C > 0);

  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T combine_n(const T &v, int64_t area) { return v; }
  static T apply_delta(const T &v, const T &d) { return d; }

  struct InnerNode {
    T value;
    int lo, hi;
    InnerNode *left, *right;

    InnerNode(int lo, int hi, const T &v)
        : value(v), lo(lo), hi(hi), left(nullptr), right(nullptr) {}
  };

  struct OuterNode {
    InnerNode inner;
    int lo, hi;
    OuterNode *left, *right;

    OuterNode(int lo, int hi, const T &v)
        : inner(0, C - 1, v), lo(lo), hi(hi), left(nullptr), right(nullptr) {}
  } *root;

  T init;

  static int64_t length(int lo, int hi) { return hi - lo + 1LL; }
  static void append_result(std::optional<T> &res, const T &v) { res = res ? combine(*res, v) : v; }

  template<typename Node, typename Get>
  T query_nodes(const Node *n, int qlo, int qhi, int64_t span, const Get &get) const {
    if (n == nullptr) {
      return combine_n(init, span * length(qlo, qhi));
    }
    int lo = n->lo, hi = n->hi, mid = lo + (hi - lo) / 2;
    std::optional<T> res;
    if (qlo < lo) {
      append_result(res, combine_n(init, span * length(qlo, std::min(qhi, lo - 1))));
    }
    int ql = std::max(qlo, lo), qr = std::min(qhi, hi);
    if (ql <= qr) {
      if (ql == lo && qr == hi) {
        append_result(res, get(n));
      } else {
        if (ql <= mid) {
          append_result(res, query_nodes(n->left, ql, std::min(qr, mid), span, get));
        }
        if (mid < qr) {
          append_result(res, query_nodes(n->right, std::max(ql, mid + 1), qr, span, get));
        }
      }
    }
    if (hi < qhi) {
      append_result(res, combine_n(init, span * length(std::max(qlo, hi + 1), qhi)));
    }
    return *res;
  }

  T query_inner(const InnerNode *n, int c1, int c2, int64_t rows) const {
    return query_nodes(n, c1, c2, rows, [](const InnerNode *node) { return node->value; });
  }

  T query_outer(const OuterNode *n, int r1, int r2, int c1, int c2) const {
    return query_nodes(n, r1, r2, length(c1, c2), [&](const OuterNode *node) {
      return query_inner(&node->inner, c1, c2, length(node->lo, node->hi));
    });
  }

  template<typename Apply>
  void update_inner(InnerNode *n, int c, int64_t rows, const Apply &apply) {
    int lo = n->lo, hi = n->hi, mid = lo + (hi - lo) / 2;
    if (lo == hi) {
      n->value = apply(n->value);
      return;
    }
    InnerNode *&target = (c <= mid) ? n->left : n->right;
    if (target == nullptr) {
      target = new InnerNode(c, c, combine_n(init, rows));
    }
    if (target->lo <= c && c <= target->hi) {
      update_inner(target, c, rows, apply);
    } else {
      int split_lo = lo, split_hi = hi, split_mid = mid;
      do {
        if (c <= split_mid) {
          split_hi = split_mid;
        } else {
          split_lo = split_mid + 1;
        }
        split_mid = split_lo + (split_hi - split_lo) / 2;
      } while ((c <= split_mid) == (target->lo <= split_mid));
      InnerNode *tmp =
          new InnerNode(split_lo, split_hi, combine_n(init, rows * length(split_lo, split_hi)));
      if (target->lo <= split_mid) {
        tmp->left = target;
      } else {
        tmp->right = target;
      }
      target = tmp;
      update_inner(tmp, c, rows, apply);
    }
    T lval = (n->left != nullptr) ? n->left->value : combine_n(init, rows * length(lo, mid));
    T rval = (n->right != nullptr) ? n->right->value : combine_n(init, rows * length(mid + 1, hi));
    n->value = combine(lval, rval);
  }

  void update(OuterNode *n, int r, int c, const T &d) {
    int lo = n->lo, hi = n->hi, mid = lo + (hi - lo) / 2;
    int64_t rows = length(lo, hi);
    if (lo == hi) {
      update_inner(&n->inner, c, 1, [&](const T &v) { return apply_delta(v, d); });
      return;
    }
    if (r <= mid) {
      if (n->left == nullptr) {
        n->left = new OuterNode(lo, mid, combine_n(init, length(lo, mid) * C));
      }
      update(n->left, r, c, d);
    } else {
      if (n->right == nullptr) {
        n->right = new OuterNode(mid + 1, hi, combine_n(init, length(mid + 1, hi) * C));
      }
      update(n->right, r, c, d);
    }
    T value = combine_n(init, rows);
    if (n->left != nullptr || n->right != nullptr) {
      T lval = (n->left != nullptr) ? query_inner(&n->left->inner, c, c, length(lo, mid))
                                    : combine_n(init, length(lo, mid));
      T rval = (n->right != nullptr) ? query_inner(&n->right->inner, c, c, length(mid + 1, hi))
                                     : combine_n(init, length(mid + 1, hi));
      value = combine(lval, rval);
    }
    update_inner(&n->inner, c, rows, [&](const T &) { return value; });
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
      clean_up(n->inner.left);
      clean_up(n->inner.right);
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  explicit SparseSegTree2D(const T &v = T{})
      : root(new OuterNode(0, R - 1, combine_n(v, static_cast<int64_t>(R) * C))), init(v) {}

  ~SparseSegTree2D() { clean_up(root); }
  SparseSegTree2D(const SparseSegTree2D &) = delete;
  SparseSegTree2D &operator=(const SparseSegTree2D &) = delete;

  T at(int r, int c) const {
    assert(0 <= r && r < R && 0 <= c && c < C);
    return query(r, c, r, c);
  }

  T query(int r1, int c1, int r2, int c2) const {
    assert(0 <= r1 && r1 <= r2 && r2 < R);
    assert(0 <= c1 && c1 <= c2 && c2 < C);
    return query_outer(root, r1, r2, c1, c2);
  }

  void update(int r, int c, const T &d) {
    assert(0 <= r && r < R && 0 <= c && c < C);
    update(root, r, c, d);
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
  SparseSegTree2D<int> t(0);
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

  SparseSegTree2D<int, 1, 2> rectangular(0);
  rectangular.update(0, 0, 5);
  rectangular.update(0, 1, 6);
  assert(rectangular.query(0, 0, 0, 1) == 5);
  return 0;
}
