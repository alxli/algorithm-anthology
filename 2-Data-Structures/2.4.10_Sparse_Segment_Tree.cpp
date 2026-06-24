/*

A sparse segment tree (also commonly called a dynamic or implicit segment tree) maintains an array
over a large index range while supporting both dynamic queries and updates of contiguous subarrays
via the lazy propagation technique. This implementation uses lazy initialization of nodes to
conserve memory: only the nodes covering touched indices are ever allocated, so indices up to `N`
are supported without preallocating the whole tree.

The query operation is defined by an associative aggregate function `combine(a, b)`. Since untouched
nodes are implicit, `repeat_value(v, len)` must return the aggregate summary of `len` copies of the
initial value `v`. The default code below assumes a numerical array type, defining queries for the
"min" of the target range. For range-sum queries, `combine(a, b)` should return `a + b` and
`repeat_value(v, len)` should return `v * len`.

Range updates are defined by `apply_delta(v, d, len)`, which applies an update delta `d` to an
aggregate summary `v` representing `len` array values, and by `compose_deltas(old, d)`, which
combines a pending older delta with a newer delta in that order. These functions do not support
arbitrary combinations: applying a delta to a combined segment must be equivalent to applying it to
each child segment and then combining the results, and composed deltas must be equivalent to
performing their updates sequentially. The default code below defines range assignment. For range
increment, `compose_deltas(old, d)` should return `old + d`; `apply_delta(v, d, len)` should return
`v + d` for range-min/range-max queries, and `v + d * len` for range-sum queries.

- `SparseSegTree<T, N>(v)` constructs an array over indices $[0, `N`]$, with every value implicitly
  initialized to `v`. Nodes are allocated lazily as indices are touched.
- `at(i)` returns the value at index `i`, where `i` is between $0$ and `N`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive. If `lo == hi`, then the single specified value is returned.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.
- `update(lo, hi, d)` modifies the value at each array index from `lo` to `hi`, inclusive, by
  applying the delta `d` to each value.
- `max_right(lo, pred)` returns the largest boundary `hi` such that the aggregate over the half-open
  range $[`lo`, `hi`)$ satisfies `pred`. It returns $N + 1$ if `pred` remains true to the end.
- `min_left(hi, pred)` returns the smallest boundary `lo` such that the aggregate over the half-open
  range $[`lo`, `hi`)$ satisfies `pred`. It returns $0$ if `pred` remains true to the beginning.

For the boundary-search functions, `pred` takes aggregate `T` values of candidate ranges. As a range
grows, `pred` may change from true to false but never back to true; The empty range is considered
valid. E.g. for `combine = min`, use `pred(mn) = (mn > x)` to find the first value `<= x`, or for
`combine = sum`, use `pred(sum) = (sum <= x)` with nonnegative values to find the longest range
within the limit `x`.

Time Complexity:
- O(1) per call to the constructor.
- O(log N) per call to `at()`, `update()`, `query()`, `max_right()`, and `min_left()`.

Space Complexity:
- O(k log(N)) for storage after $k$ index updates.
- O(log N) auxiliary stack space for `update()`, `query()`, `max_right()`, and `min_left()`.

*/

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>

template<typename T, int N = 1000000000>
class SparseSegTree {
  static_assert(N >= 0);

  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T repeat_value(const T &v, int64_t len) { return v; }
  static T apply_delta(const T &v, const T &d, int64_t len) { return d; }
  static T compose_deltas(const T &d1, const T &d2) { return d2; }

  struct Node {
    T value, delta;
    bool pending;
    Node *left, *right;

    explicit Node(const T &v) : value(v), pending(false), left(nullptr), right(nullptr) {}
  } *root;

  T init;

  void update_delta(Node *&n, const T &d, int64_t len) {
    if (n == nullptr) {
      n = new Node(repeat_value(init, len));
    }
    n->delta = n->pending ? compose_deltas(n->delta, d) : d;
    n->pending = true;
  }

  void push_delta(Node *n, int lo, int hi) {
    if (n == nullptr) {
      return;
    }
    if (n->pending) {
      n->value = apply_delta(n->value, n->delta, hi - lo + 1);
      if (lo != hi) {
        int mid = lo + (hi - lo) / 2;
        update_delta(n->left, n->delta, mid - lo + 1);
        update_delta(n->right, n->delta, hi - mid);
      }
    }
    n->pending = false;
  }

  T query(Node *n, int lo, int hi, int tgt_lo, int tgt_hi) {
    if (n == nullptr) {
      return repeat_value(init, hi - lo + 1);
    }
    push_delta(n, lo, hi);
    if (lo == tgt_lo && hi == tgt_hi) {
      return n->value;
    }
    int mid = lo + (hi - lo) / 2;
    if (tgt_lo <= mid && mid < tgt_hi) {
      return combine(
          query(n->left, lo, mid, tgt_lo, std::min(tgt_hi, mid)),
          query(n->right, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi)
      );
    }
    if (tgt_lo <= mid) {
      return query(n->left, lo, mid, tgt_lo, std::min(tgt_hi, mid));
    }
    return query(n->right, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi);
  }

  void update(Node *&n, int lo, int hi, int tgt_lo, int tgt_hi, const T &d) {
    if (n == nullptr) {
      if (hi < tgt_lo || lo > tgt_hi) {
        return;
      }
      n = new Node(repeat_value(init, hi - lo + 1));
    } else {
      push_delta(n, lo, hi);
    }
    if (hi < tgt_lo || lo > tgt_hi) {
      return;
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      n->delta = d;
      n->pending = true;
      push_delta(n, lo, hi);
      return;
    }
    int mid = lo + (hi - lo) / 2;
    update(n->left, lo, mid, tgt_lo, tgt_hi, d);
    update(n->right, mid + 1, hi, tgt_lo, tgt_hi, d);
    T left_value = (n->left != nullptr) ? n->left->value : repeat_value(init, mid - lo + 1);
    T right_value = (n->right != nullptr) ? n->right->value : repeat_value(init, hi - mid);
    n->value = combine(left_value, right_value);
  }

  template<typename Pred>
  int max_right(Node *n, int lo, int hi, int tgt_lo, const Pred &pred, std::optional<T> &acc) {
    if (hi < tgt_lo) {
      return -1;
    }
    if (n != nullptr) {
      push_delta(n, lo, hi);
    }
    T node_value = n != nullptr ? n->value : repeat_value(init, hi - lo + 1);
    if (tgt_lo <= lo) {
      T next = acc ? combine(*acc, node_value) : node_value;
      if (pred(next)) {
        acc = next;
        return -1;
      }
      if (lo == hi) {
        return lo;
      }
    }
    int mid = lo + (hi - lo) / 2;
    int res = max_right(n == nullptr ? nullptr : n->left, lo, mid, tgt_lo, pred, acc);
    return res != -1 ? res
                     : max_right(n == nullptr ? nullptr : n->right, mid + 1, hi, tgt_lo, pred, acc);
  }

  template<typename Pred>
  int min_left(Node *n, int lo, int hi, int tgt_hi, const Pred &pred, std::optional<T> &acc) {
    if (tgt_hi <= lo) {
      return -1;
    }
    if (n != nullptr) {
      push_delta(n, lo, hi);
    }
    T node_value = n != nullptr ? n->value : repeat_value(init, hi - lo + 1);
    if (hi < tgt_hi) {
      T next = acc ? combine(node_value, *acc) : node_value;
      if (pred(next)) {
        acc = next;
        return -1;
      }
      if (lo == hi) {
        return lo + 1;
      }
    }
    int mid = lo + (hi - lo) / 2;
    int res = min_left(n == nullptr ? nullptr : n->right, mid + 1, hi, tgt_hi, pred, acc);
    return res != -1 ? res : min_left(n == nullptr ? nullptr : n->left, lo, mid, tgt_hi, pred, acc);
  }

  void clean_up(Node *n) {
    if (n != nullptr) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  explicit SparseSegTree(const T &v = T()) : root(nullptr), init(v) {}

  ~SparseSegTree() { clean_up(root); }
  SparseSegTree(const SparseSegTree &) = delete;
  SparseSegTree &operator=(const SparseSegTree &) = delete;

  T at(int i) {
    assert(0 <= i && i <= N);
    return query(i, i);
  }

  T query(int lo, int hi) {
    assert(0 <= lo && lo <= hi && hi <= N);
    return query(root, 0, N, lo, hi);
  }

  void update(int i, const T &d) {
    assert(0 <= i && i <= N);
    update(i, i, d);
  }

  void update(int lo, int hi, const T &d) {
    assert(0 <= lo && lo <= hi && hi <= N);
    update(root, 0, N, lo, hi, d);
  }

  template<typename Pred>
  int max_right(int lo, const Pred &pred) {
    assert(0 <= lo && lo <= N + 1);
    std::optional<T> acc;
    int res = max_right(root, 0, N, lo, pred, acc);
    return res == -1 ? N + 1 : res;
  }

  template<typename Pred>
  int min_left(int hi, const Pred &pred) {
    assert(0 <= hi && hi <= N + 1);
    std::optional<T> acc;
    int res = min_left(root, 0, N, hi, pred, acc);
    return res == -1 ? 0 : res;
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
Values: 5 5 5 1 5

***/

#include <iostream>
using namespace std;

int main() {
  SparseSegTree<int> t(0);
  t.update(0, 6);
  t.update(1, -2);
  t.update(2, 4);
  t.update(3, 8);
  t.update(4, 10);
  cout << "Values:";
  for (int i = 0; i < 5; i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == -2);
  t.update(0, 4, 5);
  t.update(3, 2);
  t.update(3, 1);
  cout << "Values:";
  for (int i = 0; i < 5; i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == 1);

  // Boundary search over the sparse domain; values at indices 0..4 are now {5, 5, 5, 1, 5}.
  assert(t.max_right(0, [](int mn) { return mn > 1; }) == 3);
  assert(t.min_left(5, [](int mn) { return mn > 1; }) == 4);
  return 0;
}
