/*

Maintains a dynamic set of lines $y = mx + b$ and answers minimum value queries at integer points.
Lines and queries may arrive in arbitrary order, making this useful for dynamic programming
recurrences of the form `dp[i] = min(m[j] * x[i] + b[j])` without monotone slopes or query
coordinates.

A Li Chao tree fixes an inclusive coordinate domain $[`lo`, `hi`]$ and stores one line at each node
of a dynamic segment tree. Inserting a line keeps the line that wins at the midpoint and recurses
only into the half where the other line may still win; a query takes the best value along one
root-to-leaf path. Prefer this structure when the domain is known and manageable. The fully dynamic
convex hull in the previous section needs no fixed domain and also supports maximum queries, while a
Li Chao tree has a simpler invariant and is easier to extend to lines active only on subintervals.

- `LiChaoTree(lo, hi)` constructs an empty tree over integer domain $[`lo`, `hi`]$.
- `add_line(m, b)` inserts line $y = mx + b$. Lines may be added in any order.
- `query(x)` returns the minimum $y$-value among all inserted lines at coordinate `x`. At least one
  line must have been inserted, and query coordinates may be supplied in any order.

Overflow warning: each comparison and query evaluates `m * x + b`, which must fit in `int64_t`. The
difference `hi - lo` must also fit in `int64_t`.

Time Complexity:
- O(log d) per call to `add_line()` and `query()`, where $d$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) node storage for $n$ lines inserted, since each insertion creates at most one node.
- O(log d) auxiliary stack space per operation.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>

const int64_t INF = INT64_MAX / 4;

class LiChaoTree {
  struct Line {
    int64_t m, b;
    Line(int64_t m = 0, int64_t b = INF) : m(m), b(b) {}
    int64_t eval(int64_t x) const { return m * x + b; }  // Overflow warning.
  };

  struct Node {
    Line f;
    Node *left, *right;
    explicit Node(const Line &f) : f(f), left(nullptr), right(nullptr) {}
  };

  Node *root;
  int64_t lo, hi;

  static void clean_up(Node *n) {
    if (n == nullptr) {
      return;
    }
    clean_up(n->left);
    clean_up(n->right);
    delete n;
  }

  static void add_line(Node *&n, int64_t lo, int64_t hi, Line f) {
    if (n == nullptr) {
      n = new Node(f);
      return;
    }
    int64_t mid = lo + (hi - lo) / 2;
    bool left_better = f.eval(lo) < n->f.eval(lo);
    bool mid_better = f.eval(mid) < n->f.eval(mid);
    if (mid_better) {
      std::swap(f, n->f);
    }
    if (lo == hi) {
      return;
    }
    if (left_better != mid_better) {
      add_line(n->left, lo, mid, f);
    } else {
      add_line(n->right, mid + 1, hi, f);
    }
  }

  static int64_t query(Node *n, int64_t lo, int64_t hi, int64_t x) {
    if (n == nullptr) {
      return INF;
    }
    int64_t res = n->f.eval(x);
    if (lo == hi) {
      return res;
    }
    int64_t mid = lo + (hi - lo) / 2;
    if (x <= mid) {
      return std::min(res, query(n->left, lo, mid, x));
    }
    return std::min(res, query(n->right, mid + 1, hi, x));
  }

 public:
  LiChaoTree(int64_t lo, int64_t hi) : root(nullptr), lo(lo), hi(hi) { assert(lo <= hi); }

  ~LiChaoTree() { clean_up(root); }
  LiChaoTree(const LiChaoTree &) = delete;
  LiChaoTree &operator=(const LiChaoTree &) = delete;
  void add_line(int64_t m, int64_t b) { add_line(root, lo, hi, Line(m, b)); }

  int64_t query(int64_t x) const {
    assert(root != nullptr);
    assert(lo <= x && x <= hi);
    return query(root, lo, hi, x);
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  LiChaoTree h(-10, 10);
  h.add_line(3, 0);
  h.add_line(0, 6);
  h.add_line(1, 2);
  h.add_line(2, 1);
  // Minimize among y = 3x, 6, x + 2, and 2x + 1.
  assert(h.query(0) == 0);
  assert(h.query(2) == 4);
  assert(h.query(1) == 3);
  assert(h.query(3) == 5);
  return 0;
}
