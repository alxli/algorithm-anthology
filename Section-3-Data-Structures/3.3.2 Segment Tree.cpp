/*

Description: A segment tree is a data structure used for
solving the dynamic range query problem, which asks to
determine the minimum (or maximum) value in any given
range in an array that is constantly being updated.

Time Complexity: Assuming merge() is O(1), query(),
update(), and at() are O(log N). size() is O(1). If
merge() is not O(1), then all logarithmic running times
are multiplied by a factor of the complexity of merge().

Space Complexity: O(MAXN). Note that a segment tree with
N leaves requires 2^(log2(N) - 1) = 4*N total nodes.

Note: This implementation is 0-based, meaning that all
indices from 0 to N - 1, inclusive, are accessible.

*/

#include <limits> /* std::numeric_limits<T>::min() */
#include <vector>

#include <cstdio>

template<class T> class segment_tree {

  static T identity() {
    return std::numeric_limits<T>::min();
  }

  static T join_values(const T &a, const T &b) {
    return a > b ? a : b;
  }

  int len;
  std::vector<T> t;

  // Temporary helper variables.
  int x, y;
  T val;

  template<class It>
  void build(int n, int lo, int hi, bool init_with_arr, It arr) {
    if (lo == hi) {
      t[n] = init_with_arr ? *(arr + lo) : val;
      return;
    }
    build(n*2 + 1, lo, (lo + hi)/2, init_with_arr, arr);
    build(n*2 + 2, (lo + hi)/2 + 1, hi, init_with_arr, arr);
    t[n] = join_values(t[n*2 + 1], t[n*2 + 2]);
  }

  void update(int n, int lo, int hi) {
    if (x < lo || x > hi) {
      return;
    }
    if (lo == hi) {
      t[n] = val;
      return;
    }
    update(n*2 + 1, lo, (lo + hi)/2);
    update(n*2 + 2, (lo + hi)/2 + 1, hi);
    t[n] = join_values(t[n*2 + 1], t[n*2 + 2]);
  }

  T query(int n, int lo, int hi) {
    if (hi < x || lo > y) {
      return identity();
    }
    if (lo >= x && hi <= y) {
      return t[n];
    }
    return join_values(query(n*2 + 1, lo, (lo + hi)/2),
                       query(n*2 + 2, (lo + hi)/2 + 1, hi));
  }

 public:
  segment_tree(int n, const T &v = T()) {
    len = n;
    t.resize(4*len, identity());
    val = v;
    build<T*>(0, 0, len - 1, false, 0);
  }

  template<class It> segment_tree(It lo, It hi) {
    len = hi - lo;
    t.resize(4*len, identity());
    build<It>(0, 0, len - 1, true, lo);
  }

  int size() const {
    return len;
  }

  T at(int i) {
    return query(i, i);
  }

  void update(int i, const T &v) {
    x = i;
    val = v;
    update(0, 0, len - 1);
  }

  T query(int lo, int hi) {
    x = lo;
    y = hi;
    return query(0, 0, len - 1);
  }
};

/*** Example Usage and Output:

Values: 6 4 1 8 10
The max value in the range [0, 3] is 8.

***/

#include <iostream>
using namespace std;

int main() {
  int arr[5] = {6, -2, 1, 8, 10};
  segment_tree<int> t(arr, arr + 5);
  t.update(1, 4);
  cout << "Values:";
  for (int i = 0; i < t.size(); i++) {
    cout << " " << t.at(i);
  }
  cout << endl << "The max value in the range [0, 3] is "
       << t.query(0, 3) << "." << endl;
  return 0;
}
