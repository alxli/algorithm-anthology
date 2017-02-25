/*

Maintain an array, allowing for updates of individual indices (point update) and
queries on contiguous sub-arrays (range query). This implementation assumes that
the array is 0-based (i.e. has valid indices from 0 to size() - 1, inclusive).

The query operation is defined by an associative join_values() function which:
satisfies join_values(x, join_values(y, z)) = join_values(join_values(x, y), z)
for all values x, y, and z in the array. The default definition below assumes a
numerical array type, supporting queries for the "max" of the target range.
Another possible query operation is "sum", in which the join_values() function
should defined to return "a + b".

The update operation is defined by the join_value_with_delta() function, which
determines the change made to array values. The default definition below
supports updates that "set" the chosen array index to a new value. Another
possible update operation is "increment", in which join_value_with_delta(v, d)
should be defined to return "v + d".

- segment_tree(n, v) constructs an array with n indices, all initialized to v.
- segment_tree(lo, hi) constructs an array from two RandomAccessIterators as a
  range [lo, hi), initialized to the elements of the range, in the same order.
- size() returns the size of the array.
- at(i) returns the value at index i, where i is between 0 and size() - 1.
- query(lo, hi) returns the result of join_values() applied to all indices from
  lo to hi, inclusive (i.e. join_values(a[lo], a[lo + 1], ..., a[hi])).
- update(i, d) modifies the value at index i by joining the current value with d
  using join_value_with_delta() (i.e. a[i] = join_value_with_delta(a[i], d)).

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

  static T join_value_with_delta(const T &v, const T &d) {
    return d;
  }

  int len;
  std::vector<T> value;

  void build(int i, int lo, int hi, const T &val) {
    if (lo == hi) {
      value[i] = val;
      return;
    }
    build(i*2 + 1, lo, (lo + hi)/2, val);
    build(i*2 + 2, (lo + hi)/2 + 1, hi, val);
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

  void update(int i, int lo, int hi, int target, const T &delta) {
    if (target < lo || target > hi) {
      return;
    }
    if (lo == hi) {
      value[i] = join_value_with_delta(value[i], delta);
      return;
    }
    update(i*2 + 1, lo, (lo + hi)/2, target, delta);
    update(i*2 + 2, (lo + hi)/2 + 1, hi, target, delta);
    value[i] = join_values(value[i*2 + 1], value[i*2 + 2]);
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) {
    if (lo == tgt_lo && hi == tgt_hi) {
      return value[i];
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

 public:
  segment_tree(int n, const T &v = T()) {
    len = n;
    value.resize(4*len);
    build(0, 0, len - 1, false, 0, v);
  }

  template<class It> segment_tree(It lo, It hi) {
    len = hi - lo;
    value.resize(4*len);
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
    update(0, 0, len - 1, i, d);
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
The maximum value in the range [0, 3] is 8.

***/

#include <iostream>
#include <limits>
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
  return 0;
}
