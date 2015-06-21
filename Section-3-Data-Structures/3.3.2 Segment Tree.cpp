/*

3.3.2 - 1D Segment Tree Class

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

template<class T> class segment_tree {
  int len, x, y;
  std::vector<T> t;
  T val, *init;

  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline const T nullv() { return std::numeric_limits<T>::min(); }
  static inline const T merge(const T & a, const T & b) { return a > b ? a : b; }

  void build(int n, int lo, int hi) {
    if (lo == hi) {
      t[n] = init[lo];
      return;
    }
    build(n * 2 + 1, lo, (lo + hi) / 2);
    build(n * 2 + 2, (lo + hi) / 2 + 1, hi);
    t[n] = merge(t[n * 2 + 1], t[n * 2 + 2]);
  }

  void update(int n, int lo, int hi) {
    if (x < lo || x > hi) return;
    if (lo == hi) {
      t[n] = val;
      return;
    }
    update(n * 2 + 1, lo, (lo + hi) / 2);
    update(n * 2 + 2, (lo + hi) / 2 + 1, hi);
    t[n] = merge(t[n * 2 + 1], t[n * 2 + 2]);
  }

  T query(int n, int lo, int hi) {
    if (hi < x || lo > y) return nullv();
    if (lo >= x && hi <= y) return t[n];
    return merge(query(n * 2 + 1, lo, (lo + hi) / 2),
                 query(n * 2 + 2, (lo + hi) / 2 + 1, hi));
  }

 public:
  segment_tree(int n, T * a = 0): len(n), t(4 * n, nullv()) {
    if (a != 0) {
      init = a;
      build(0, 0, len - 1);
    }
  }

  //a[i] = v
  void update(int i, const T & v) {
    x = i;
    val = v;
    update(0, 0, len - 1);
  }

  //merge(a[i] for i = lo..hi, inclusive)
  T query(int lo, int hi) {
    x = lo;
    y = hi;
    return query(0, 0, len - 1);
  }

  inline int size() { return len; }
  inline T at(int i) { return query(i, i); }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int arr[5] = {6, -2, 1, 8, 10};
  segment_tree<int> T(5, arr);
  T.update(1, 4);
  cout << "Array contains:";
  for (int i = 0; i < T.size(); i++)
    cout << " " << T.at(i);
  cout << "\nThe max value in the range [0, 3] is ";
  cout << T.query(0, 3) << ".\n"; //8
  return 0;
}
