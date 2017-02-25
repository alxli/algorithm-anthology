/*

Maintain a fixed-size array (from 0 to size() - 1) while supporting both dynamic
queries and updates of contiguous subarrays via the lazy propagation technique.
This implementation is optimized to avoid recursion.

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
The default definition below assumes that updates "set" the chosen array index
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
- O(1) auxiliary per call to all operations.

*/

#include <vector>

template<class T> class segment_tree {
  static T join_values(const T &a, const T &b) {
    return a > b ? a : b;
  }

  static T join_value_with_delta(const T &v, const T &d, int len) {
    return d;
  }

  static T join_deltas(const T &d1, const T &d2) {
    return d2;  // For "set" updates, the more recent delta prevails.
  }

  int len;
  std::vector<T> value, delta;
  std::vector<int> seglen;
  std::vector<bool> pending;

  void init() {
    delta.resize(2*len);
    seglen.resize(2*len, 0);
    pending.resize(2*len, false);
    std::fill(seglen.begin() + len/2, seglen.end(), 1);
    for (int i = value.size() - 1; i > 1; i -= 2) {
      value[i >> 1] = join_values(value[i], value[i ^ 1]);
      seglen[i >> 1] = seglen[i] + seglen[i ^ 1];
    }
  }

  T join_value_with_delta(int i) {
    return pending[i] ? join_value_with_delta(value[i], delta[i], seglen[i])
                      : value[i];
  }

  void push_delta(int parent, int child) {
    delta[child] = pending[child] ? join_deltas(delta[child], delta[parent])
                                  : delta[parent];
    pending[child] = true;
  }

  void push_delta(int i) {
    int d;
    for (d = 0; (i >> d) > 0; d++) {}
    for (d -= 2; d >= 0; d--) {
      int x = i >> d;
      if (pending[x >> 1]) {
        value[x >> 1] = join_value_with_delta(x >> 1);
        push_delta(x >> 1, x);
        push_delta(x >> 1, x ^ 1);
        pending[x >> 1] = false;
      }
    }
  }

 public:
  segment_tree(int n, const T &v = T()) {
    len = n;
    value.resize(2*len);
    for (int i = 0; i < len; i++) {
      value[len + i] = v;
    }
    init();
  }

  template<class It> segment_tree(It lo, It hi) {
    len = hi - lo;
    value.resize(2*len);
    for (int i = 0; i < len; i++) {
      value[len + i] = *(lo + i);
    }
    init();
  }

  int size() const {
    return len;
  }

  T at(int i) {
    return query(i, i);
  }

  T query(int lo, int hi) {
    push_delta(lo += len);
    push_delta(hi += len);
    T res;
    bool found = false;
    for (; lo <= hi; lo = (lo + 1) >> 1, hi = (hi - 1) >> 1) {
      if ((lo & 1) != 0) {
        res = found ? join_values(res, join_value_with_delta(lo))
                    : join_value_with_delta(lo);
        found = true;
      }
      if ((hi & 1) == 0) {
        res = found ? join_values(res, join_value_with_delta(hi))
                    : join_value_with_delta(hi);
        found = true;
      }
    }
    return res;
  }

  void update(int i, const T &d) {
    update(i, i, d);
  }

  void update(int lo, int hi, const T &d) {
    push_delta(lo += len);
    push_delta(hi += len);
    int ta = -1, tb = -1;
    for (; lo <= hi; lo = (lo + 1) >> 1, hi = (hi - 1) >> 1) {
      if ((lo & 1) != 0) {
        delta[lo] = pending[lo] ? join_deltas(delta[lo], d) : d;
        pending[lo] = true;
        ta = (ta == -1) ? lo : ta;
      }
      if ((hi & 1) == 0) {
        delta[hi] = pending[hi] ? join_deltas(delta[hi], d) : d;
        pending[hi] = true;
        tb = (tb == -1) ? hi : tb;
      }
    }
    for (int i = ta; i > 1; i >>= 1) {
      value[i >> 1] = join_values(join_value_with_delta(i),
                                  join_value_with_delta(i ^ 1));
    }
    for (int i = tb; i > 1; i >>= 1) {
      value[i >> 1] = join_values(join_value_with_delta(i),
                                  join_value_with_delta(i ^ 1));
    }
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
