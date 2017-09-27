/*

Maintain a fixed-size array while supporting both dynamic queries and updates of
contiguous subarrays via the lazy propagation technique.

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
#include <vector>

template<class T>
class segment_tree {
  static T join_values(const T &a, const T &b) {
    return std::min(a, b);
  }

  static T join_value_with_delta(const T &v, const T &d, int len) {
    return d;
  }

  static T join_deltas(const T &d1, const T &d2) {
    return d2;  // For "set" updates, the more recent delta prevails.
  }

  int len;
  std::vector<T> value, delta;
  std::vector<bool> pending;

  void build(int i, int lo, int hi, const T &v) {
    if (lo == hi) {
      value[i] = v;
      return;
    }
    int mid = lo + (hi - lo)/2;
    build(i*2 + 1, lo, mid, v);
    build(i*2 + 2, mid + 1, hi, v);
    value[i] = join_values(value[i*2 + 1], value[i*2 + 2]);
  }

  template<class It>
  void build(int i, int lo, int hi, It arr) {
    if (lo == hi) {
      value[i] = *(arr + lo);
      return;
    }
    int mid = lo + (hi - lo)/2;
    build(i*2 + 1, lo, mid, arr);
    build(i*2 + 2, mid + 1, hi, arr);
    value[i] = join_values(value[i*2 + 1], value[i*2 + 2]);
  }

  void push_delta(int i, int lo, int hi) {
    if (pending[i]) {
      value[i] = join_value_with_delta(value[i], delta[i], hi - lo + 1);
      if (lo != hi) {
        int l = 2*i + 1, r = 2*i + 2;
        delta[l] = pending[l] ? join_deltas(delta[l], delta[i]) : delta[i];
        delta[r] = pending[r] ? join_deltas(delta[r], delta[i]) : delta[i];
        pending[l] = pending[r] = true;
      }
      pending[i] = false;
    }
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) {
    push_delta(i, lo, hi);
    if (lo == tgt_lo && hi == tgt_hi) {
      return value[i];
    }
    int mid = lo + (hi - lo)/2;
    if (tgt_lo <= mid && mid < tgt_hi) {
      return join_values(
                query(i*2 + 1, lo, mid, tgt_lo, std::min(tgt_hi, mid)),
                query(i*2 + 2, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi));
    }
    if (tgt_lo <= mid) {
      return query(i*2 + 1, lo, mid, tgt_lo, std::min(tgt_hi, mid));
    }
    return query(i*2 + 2, mid + 1, hi, std::max(tgt_lo, mid + 1), tgt_hi);
  }

  void update(int i, int lo, int hi, int tgt_lo, int tgt_hi, const T &d) {
    push_delta(i, lo, hi);
    if (hi < tgt_lo || lo > tgt_hi) {
      return;
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      delta[i] = d;
      pending[i] = true;
      push_delta(i, lo, hi);
      return;
    }
    update(2*i + 1, lo, (lo + hi)/2, tgt_lo, tgt_hi, d);
    update(2*i + 2, (lo + hi)/2 + 1, hi, tgt_lo, tgt_hi, d);
    value[i] = join_values(value[2*i + 1], value[2*i + 2]);
  }

 public:
  segment_tree(int n, const T &v = T())
      : len(n), value(4*len), delta(4*len), pending(4*len, false) {
    build(0, 0, len - 1, v);
  }

  template<class It>
  segment_tree(It lo, It hi)
      : len(hi - lo), value(4*len), delta(4*len), pending(4*len, false) {
    build(0, 0, len - 1, lo);
  }

  int size() const {
    return len;
  }

  T at(int i) {
    return query(i, i);
  }

  T query(int lo, int hi) {
    return query(0, 0, len - 1, lo, hi);
  }

  void update(int i, const T &d) {
    update(0, 0, len - 1, i, i, d);
  }

  void update(int lo, int hi, const T &d) {
    update(0, 0, len - 1, lo, hi, d);
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
  int arr[5] = {6, -2, 1, 8, 10};
  segment_tree<int> t(arr, arr + 5);
  t.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == -2);
  t.update(0, 4, 5);
  t.update(3, 2);
  t.update(3, 1);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl;
  assert(t.query(0, 3) == 1);
  return 0;
}
