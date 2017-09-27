/*

Maintain a fixed-size array while supporting both dynamic queries and updates of
contiguous subarrays via the lazy propagation technique. This implementation
uses lazy initialization of nodes to conserve memory while supporting large
indices.

The query operation is defined by an associative join_values() function which
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the array. The default code below assumes a
numerical array type, defining queries for the "min" of the target range.
Another possible query operation is "sum", in which case the join_values()
function should be defined to return "a + b".

The update operation is defined by the join_value_with_delta() and join_deltas()
functions, which determines the change made to array values. These must satisfy:
- join_deltas(d1, join_deltas(d2, d3)) = join_deltas(join_deltas(d1, d2), d3).
- join_value_with_delta(join_values(v, ...(m times)..., v), d, m)) should be
  equal to join_values(join_value_with_delta(v, d, 1), ...(m times)).
- if a sequence d_1, ..., d_m of deltas is used to update a value v, then
  join_value_with_delta(v, join_deltas(d_1, ..., d_m), 1) should be equivalent
  to m sequential calls to join_value_with_delta(v, d_i, 1) for i = 1..m.
The default code below defines updates that "set" the chosen array index to a
new value. Another possible update operation is "increment", in which case
join_value_with_delta(v, d, len) should be defined to return "v + d*len" and
join_deltas(d1, d2) should be defined to return "d1 + d2".

- segment_tree(n, v) constructs an array of size n with indices from 0 to n - 1,
  inclusive, and all values initialized to v.
- segment_tree(lo, hi) constructs an array from two RandomAccessIterators as a
  range [lo, hi), initialized to the elements of the range in the same order.
- size() returns the size of the array.
- at(i) returns the value at index i, where i is between 0 and size() - 1.
- query(lo, hi) returns the result of join_values() applied to all indices from
  lo to hi, inclusive. If the distance between lo and hi is 1, then the single
  specified value is returned.
- update(i, d) assigns the value v at index i to join_value_with_delta(v, d).
- update(lo, hi, d) modifies the value at each array index from lo to hi,
  inclusive, by respectively joining them with d using join_value_with_delta().

Time Complexity:
- O(n) per call to both constructors, where n is the size of the array.
- O(1) per call to size().
- O(log n) per call to at(), update(), and query().

Space Complexity:
- O(n) for storage of the array elements.
- O(log n) auxiliary stack space for update() and query().
- O(1) auxiliary for size().

*/

#include <algorithm>
#include <cstdlib>

template<class T>
class segment_tree {
  static const int MAXN = 1000000000;

  static T join_values(const T &a, const T &b) {
    return std::min(a, b);
  }

  static T join_segment(const T &v, int len) {
    return v;
  }

  static T join_value_with_delta(const T &v, const T &d, int len) {
    return d;
  }

  static T join_deltas(const T &d1, const T &d2) {
    return d2;  // For "set" updates, the more recent delta prevails.
  }

  struct node_t {
    T value, delta;
    bool pending;
    node_t *left, *right;

    node_t(const T &v) : value(v), pending(false), left(NULL), right(NULL) {}
  } *root;

  T init;

  void update_delta(node_t *&n, const T &d, int len) {
    if (n == NULL) {
      n = new node_t(join_segment(init, len));
    }
    n->delta = n->pending ? join_deltas(n->delta, d) : d;
    n->pending = true;
  }

  void push_delta(node_t *n, int lo, int hi) {
    if (n->pending) {
      n->value = join_value_with_delta(n->value, n->delta, hi - lo + 1);
      if (lo != hi) {
        int mid = lo + (hi - lo)/2;
        update_delta(n->left, n->delta, mid - lo + 1);
        update_delta(n->right, n->delta, hi - mid);
      }
    }
    n->pending = false;
  }

  T query(node_t *n, int lo, int hi, int tgt_lo, int tgt_hi) {
    push_delta(n, lo, hi);
    if (lo == tgt_lo && hi == tgt_hi) {
      return (n == NULL) ? join_segment(init, hi - lo + 1) : n->value;
    }
    int mid = lo + (hi - lo)/2;
    if (tgt_lo <= mid && mid < tgt_hi) {
      return join_values(
          query(n->left, lo, mid, tgt_lo, std::min(tgt_hi, mid)),
          query(n->right, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi));
    }
    if (tgt_lo <= mid) {
      return query(n->left, lo, mid, tgt_lo, std::min(tgt_hi, mid));
    }
    return query(n->right, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi);
  }

  void update(node_t *&n, int lo, int hi, int tgt_lo, int tgt_hi, const T &d) {
    if (n == NULL) {
      n = new node_t(join_segment(init, hi - lo + 1));
    }
    push_delta(n, lo, hi);
    if (hi < tgt_lo || lo > tgt_hi) {
      return;
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      n->delta = d;
      n->pending = true;
      push_delta(n, lo, hi);
      return;
    }
    int mid = lo + (hi - lo)/2;
    update(n->left, lo, mid, tgt_lo, tgt_hi, d);
    update(n->right, mid + 1, hi, tgt_lo, tgt_hi, d);
    n->value = join_values(n->left->value, n->right->value);
  }

  void clean_up(node_t *n) {
    if (n != NULL) {
      clean_up(n->left);
      clean_up(n->right);
      delete n;
    }
  }

 public:
  segment_tree(const T &v = T()) : root(NULL), init(v) {}

  ~segment_tree() {
    clean_up(root);
  }

  T at(int i) {
    return query(i, i);
  }

  T query(int lo, int hi) {
    return query(root, 0, MAXN, lo, hi);
  }

  void update(int i, const T &d) {
    return update(i, i, d);
  }

  void update(int lo, int hi, const T &d) {
    return update(root, 0, MAXN, lo, hi, d);
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
Values: 5 5 5 1 5

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  segment_tree<int> t(0);
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
  return 0;
}
