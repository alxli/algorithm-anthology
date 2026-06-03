/*

Maintains a dynamic set of lines and answers minimum value queries at points in a fixed integer
domain. A Li Chao tree is useful for dynamic programming recurrences of the form
`dp[i] = min(m[j] * x[i] + b[j])`, especially when line slopes and query points arrive in arbitrary
order.

The implementation below stores the lower envelope of lines over the inclusive domain
`[X_MIN, X_MAX]`. It supports adding arbitrary lines and querying arbitrary integer x-coordinates in
O(log C), where $C =$ `X_MAX` $-$ `X_MIN` $+ 1$.

- `LiChaoTree(lo, hi)` constructs an empty tree over integer domain `[lo, hi]`.
- `add_line(m, b)` inserts line $y = mx + b$.
- `query(x)` returns the minimum y-value among all inserted lines at coordinate `x`.

Time Complexity:
- O(log C) per call to `add_line(m, b)` and `query(x)`, where $C$ is the domain size.

Space Complexity:
- O(n log C) worst-case node storage for $n$ inserted lines, usually much less.
- O(log C) auxiliary stack space per operation.

*/

#include <algorithm>
#include <cassert>
#include <limits>

const long long INF = std::numeric_limits<long long>::max() / 4;

struct Line {
  long long m, b;

  Line(long long m = 0, long long b = INF) : m(m), b(b) {}
  long long eval(long long x) const { return m * x + b; }
};

class LiChaoTree {
  struct node {
    Line f;
    node *left, *right;

    node(const Line &f) : f(f), left(nullptr), right(nullptr) {}
  };

  node *root;
  long long lo, hi;

  static void clean_up(node *n) {
    if (n == nullptr) {
      return;
    }
    clean_up(n->left);
    clean_up(n->right);
    delete n;
  }

  static void add_line(node *&n, long long l, long long r, Line f) {
    if (n == nullptr) {
      n = new node(f);
      return;
    }
    long long mid = l + (r - l) / 2;
    bool left_better = f.eval(l) < n->f.eval(l);
    bool mid_better = f.eval(mid) < n->f.eval(mid);
    if (mid_better) {
      std::swap(f, n->f);
    }
    if (l == r) {
      return;
    }
    if (left_better != mid_better) {
      add_line(n->left, l, mid, f);
    } else {
      add_line(n->right, mid + 1, r, f);
    }
  }

  static long long query(node *n, long long l, long long r, long long x) {
    if (n == nullptr) {
      return INF;
    }
    long long res = n->f.eval(x);
    if (l == r) {
      return res;
    }
    long long mid = l + (r - l) / 2;
    if (x <= mid) {
      return std::min(res, query(n->left, l, mid, x));
    }
    return std::min(res, query(n->right, mid + 1, r, x));
  }

 public:
  LiChaoTree(long long lo, long long hi) : root(nullptr), lo(lo), hi(hi) {}

  ~LiChaoTree() { clean_up(root); }

  void add_line(long long m, long long b) { add_line(root, lo, hi, Line(m, b)); }

  long long query(long long x) const {
    assert(lo <= x && x <= hi);
    return query(root, lo, hi, x);
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  LiChaoTree cht(-10, 10);
  cht.add_line(3, 0);
  cht.add_line(1, 2);
  cht.add_line(-1, 10);
  assert(cht.query(0) == 0);
  assert(cht.query(2) == 4);
  assert(cht.query(10) == 0);
  return 0;
}
