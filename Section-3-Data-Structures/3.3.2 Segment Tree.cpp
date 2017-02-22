/*

Maintain an array, allowing for updates of individual indices (point update) and
queries on contiguous sub-arrays (range query) of any associative function with
an identity value. The join_values() and identity() functions may be arbitrarily
defined as long as associativity and the identity property are preserved. That
is, for all x, y, and z of the array's type, the definitions must satisfy:

- join_values(x, join_values(y, z)) = join_values(join_values(x, y), z).
- join_values(x, identity()) = join_values(identity(), x) = x.

The default definition below assumes a numerical array type, making queries
return the max of the target range (identity() is defined as negative infinity).
Another common usage is sum(), where join_values() returns a + b and identity()
is defined as 0. This implementation assumes that the array is 0-based (i.e. has
valid indices from 0 to size() - 1, inclusive).

- segment_tree(n, v) constructs an array with n indices, all initialized to v.
- segment_tree(lo, hi) constructs an array from two RandomAccessIterators as a
  range [lo, hi), initialized to the elements of the range, in the same order.
- size() returns the size of the array.
- at(i) returns the value at index i, where i is between
- update(i, v) assigns v to the value at index i (i.e. a[i] = v).
- query(lo, hi) returns the result of join_values() applied to all indices from
  lo to hi, inclusive (i.e. join_values(a[lo], a[lo + 1], ..., a[hi])).

Time Complexity:
- O(n) per call to both constructors, where n is the size of the array.
- O(1) per call to size().
- O(log n) per call to at(), update(), and query().

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary per call to size().
- O(log n) auxiliary stack space per call to update() and query().

*/

#include <limits>  // std::numeric_limits<T>::min()
#include <vector>

template<class T> class segment_tree {

  static T join_values(const T &a, const T &b) {
    return a > b ? a : b;
  }

  static T identity() {
    return std::numeric_limits<T>::min();
  }

  int len;
  std::vector<T> t;

  void build(int i, int lo, int hi, const T &val) {
    if (lo == hi) {
      t[i] = val;
      return;
    }
    build(i*2 + 1, lo, (lo + hi)/2, val);
    build(i*2 + 2, (lo + hi)/2 + 1, hi, val);
    t[i] = join_values(t[i*2 + 1], t[i*2 + 2]);
  }

  template<class It> void build(int i, int lo, int hi, It arr) {
    if (lo == hi) {
      t[i] = *(arr + lo);
      return;
    }
    build(i*2 + 1, lo, (lo + hi)/2, arr);
    build(i*2 + 2, (lo + hi)/2 + 1, hi, arr);
    t[i] = join_values(t[i*2 + 1], t[i*2 + 2]);
  }

  void update(int i, int lo, int hi, int target, const T &val) {
    if (target < lo || target > hi) {
      return;
    }
    if (lo == hi) {
      t[i] = val;
      return;
    }
    update(i*2 + 1, lo, (lo + hi)/2, target, val);
    update(i*2 + 2, (lo + hi)/2 + 1, hi, target, val);
    t[i] = join_values(t[i*2 + 1], t[i*2 + 2]);
  }

  T query(int i, int lo, int hi, int tgt_lo, int tgt_hi) {
    if (hi < tgt_lo || lo > tgt_hi) {
      return identity();
    }
    if (tgt_lo <= lo && hi <= tgt_hi) {
      return t[i];
    }
    return join_values(query(i*2 + 1, lo, (lo + hi)/2, tgt_lo, tgt_hi),
                       query(i*2 + 2, (lo + hi)/2 + 1, hi, tgt_lo, tgt_hi));
  }

 public:
  segment_tree(int n, const T &v = T()) {
    len = n;
    t.resize(4*len, identity());
    build(0, 0, len - 1, false, 0, v);
  }

  template<class It> segment_tree(It lo, It hi) {
    len = hi - lo;
    t.resize(4*len, identity());
    build(0, 0, len - 1, lo);
  }

  int size() const {
    return len;
  }

  T at(int i) {
    return query(i, i);
  }

  void update(int i, const T &v) {
    update(0, 0, len - 1, i, v);
  }

  T query(int lo, int hi) {
    return query(0, 0, len - 1, lo, hi);
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10
The maximum value in the range [0, 3] is 8.

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
  return 0;
}
