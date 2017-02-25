/*

Maintain an array, allowing for updates and queries of contiguous subarrays
using the lazy propagation technique. This implementation assumes that the array
is 0-based (i.e. has valid indices from 0 to size() - 1, inclusive).

The query operation is defined by an associative join_values() function which:
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the array. The default definition below assumes a
numerical array type, supporting queries for the "max" of the target range.
Another possible query operation is "sum", in which the join_values() function
should defined to return "a + b".

The update operation is defined by the join_value_with_delta() and join_deltas()
functions, which determines the change made to array values. These must satisfy:
- join_deltas(d1, join_deltas(d2, d3)) = join_deltas(join_deltas(d1, d2), d3).
- join_value_with_delta(v, join_deltas(d, ..(m times).., d), 1) should equal
  join_value_with_delta(v, d, m), which is a faster implementation thereof.
- if a sequence d_1, ..., d_m of deltas is used to update a value v, then
  join_value_with_delta(v, join_deltas(d_1, ..., d_m), 1) should be equivalent
  to m sequential calls to join_value_with_delta(v, d_i, 1) for i = 1..m.
The default definition below supports updates that "set" the chosen array index
to a new value. Another possible update operation is "increment", in which
join_value_with_delta(v, d, len) should be defined to return "v + d*len" and
join_deltas(d1, d2) should be defined to return "d1 + d2".

- segment_tree(n, v) constructs an array with n indices, all initialized to v.
- segment_tree(lo, hi) constructs an array from two RandomAccessIterators as a
  range [lo, hi), initialized to the elements of the range, in the same order.
- size() returns the size of the array.
- at(i) returns the value at index i, where i is between 0 and size() - 1.
- query(lo, hi) returns the result of join_values() applied to all indices from
  lo to hi, inclusive (i.e. join_values(a[lo], a[lo + 1], ..., a[hi])).
- update(i, d) modifies the value at index i by joining the current value with d
  using join_value_with_delta() (i.e. a[i] = join_value_with_delta(a[i], d, 1)).
- update(lo, hi, d) modifies all array indices from lo to hi, inclusive, by
  joining the values at each array index with d using join_value_with_delta()
  (i.e. for (int i=lo; i<=hi; i++) a[i] = join_value_with_delta(a[i], d, 1);).

Time Complexity:
- O(n) per call to both constructors, where n is the size of the array.
- O(1) per call to size().
- O(log n) per call to at(), update(), and query().

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary per call to size().
- O(log n) auxiliary stack space per call to update() and query().

*/

#include <algorithm>  // std::max(), std::min()
#include <vector>

template<class T> class segment_tree {
  static T join_values(const T &a, const T &b) {
    return std::max(a, b);
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
    build(i*2 + 1, lo, (lo + hi)/2, v);
    build(i*2 + 2, (lo + hi)/2 + 1, hi, v);
    value[i] = join_values(value[i*2 + 1], value[i*2 + 2]);
  }

  template<class It> void build(int i, int lo, int hi, It arr) {
    if (lo == hi) {
      value[i] = *(arr + lo);
      return;
    }
    build(i*2 + 1, lo, (lo + hi)/2, arr);
    build(i*2 + 2, (lo + hi)/2 + 1, hi, arr);
    value[i] = join_values(value[i*2 + 1], value[i*2 + 2]);
  }

  void update_delta(int i, const T &d, int lo, int hi) {
    value[i] = join_value_with_delta(value[i], d, hi - lo + 1);
    if (lo != hi) {
      int l = 2*i + 1, r = 2*i + 2;
      delta[l] = pending[l] ? join_deltas(delta[l], d) : d;
      delta[r] = pending[r] ? join_deltas(delta[r], d) : d;
      pending[l] = pending[r] = true;
    }
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) {
    if (lo == tgt_lo && hi == tgt_hi) {
      return value[i];
    }
    if (pending[i]) {
      update_delta(i, delta[i], lo, hi);
      pending[i] = false;
    }
    int mid = (lo + hi)/2;
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
    if (pending[i]) {
      update_delta(i, delta[i], lo, hi);
      pending[i] = false;
    }
    if (hi < tgt_lo || lo > tgt_hi) {
      return;
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      update_delta(i, d, lo, hi);
      return;
    }
    update(2*i + 1, lo, (lo + hi)/2, tgt_lo, tgt_hi, d);
    update(2*i + 2, (lo + hi)/2 + 1, hi, tgt_lo, tgt_hi, d);
    value[i] = join_values(value[2*i + 1], value[2*i + 2]);
  }

 public:
  segment_tree(int n, const T &v = T()) {
    len = n;
    value.resize(4*len);
    delta.resize(4*len);
    pending.resize(4*len, false);
    build(0, 0, len - 1, v);
  }

  template<class It> segment_tree(It lo, It hi) {
    len = hi - lo;
    value.resize(4*len);
    delta.resize(4*len);
    pending.resize(4*len, false);
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
The maximum value in the range [0, 3] is 8.
Values: -5 -5 -5 1 -5
The maximum value in the range [0, 3] is 1.

***/

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
  cout << endl << "The maximum value in the range [0, 3] is "
       << t.query(0, 3) << "." << endl;
  t.update(0, 4, -5);
  t.update(3, 2);
  t.update(3, 1);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl << "The maximum value in the range [0, 3] is "
       << t.query(0, 3) << "." << endl;
  return 0;
}
