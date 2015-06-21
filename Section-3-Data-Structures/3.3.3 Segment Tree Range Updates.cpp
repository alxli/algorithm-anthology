/*

3.3.3 - 1D Segment Tree with Range Updates

Description: A segment tree is a data structure used for
solving the dynamic range query problem, which asks to
determine the minimum (or maximum) value in any given
range in an array that is constantly being updated.
Lazy propagation is a technique applied to segment trees that
allows range updates to be carried out in O(log N) time. The
range updating mechanism is less versatile than the one
implemented in the next section.

Time Complexity: Assuming merge() is O(1), query(), update(),
at() are O(log(N)). If merge() is not constant time, then all
running times are multiplied by whatever complexity the merge
function runs in.

Space Complexity: O(N) on the size of the array. A segment
tree for an array of size N needs 2^(log2(N)-1) = 4N nodes.

Note: This implementation is 0-based, meaning that all
indices from 0 to size() - 1, inclusive, are accessible.

*/

#include <limits> /* std::numeric_limits<T>::min() */
#include <vector>

template<class T> class segment_tree {
  int len, x, y;
  std::vector<T> tree, lazy;
  T val, *init;

  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline const T nullv() { return std::numeric_limits<T>::min(); }
  static inline const T merge(const T & a, const T & b) { return a > b ? a : b; }

  void build(int n, int lo, int hi) {
    if (lo == hi) {
      tree[n] = init[lo];
      return;
    }
    build(n * 2 + 1, lo, (lo + hi) / 2);
    build(n * 2 + 2, (lo + hi) / 2 + 1, hi);
    tree[n] = merge(tree[n * 2 + 1], tree[n * 2 + 2]);
  }

  T query(int n, int lo, int hi) {
    if (x > hi || y < lo) return nullv();
    if (x <= lo && hi <= y) {
      if (lazy[n] == nullv()) return tree[n];
      return tree[n] = lazy[n];
    }
    int lchild = n * 2 + 1, rchild = n * 2 + 2;
    if (lazy[n] != nullv()) {
      lazy[lchild] = lazy[rchild] = lazy[n];
      lazy[n] = nullv();
    }
    return merge(query(lchild, lo, (lo + hi)/2),
                 query(rchild, (lo + hi)/2 + 1, hi));
  }

  void _update(int n, int lo, int hi) {
    if (x > hi || y < lo) return;
    if (lo == hi) {
      tree[n] = val;
      return;
    }
    if (x <= lo && hi <= y) {
      tree[n] = lazy[n] = merge(lazy[n], val);
      return;
    }
    int lchild = n * 2 + 1, rchild = n * 2 + 2;
    if (lazy[n] != nullv()) {
      lazy[lchild] = lazy[rchild] = lazy[n];
      lazy[n] = nullv();
    }
    _update(lchild, lo, (lo + hi) / 2);
    _update(rchild, (lo + hi) / 2 + 1, hi);
    tree[n] = merge(tree[lchild], tree[rchild]);
  }

 public:
  segment_tree(int n, T * a = 0):
   len(n), tree(4 * n, nullv()), lazy(4 * n, nullv()) {
    if (a != 0) {
      init = a;
      build(0, 0, len - 1);
    }
  }

  void update(int i, const T & v) {
    x = y = i;
    val = v;
    _update(0, 0, len - 1);
  }

  //a[i] = v for i = lo..hi, inclusive
  void update(int lo, int hi, const T & v) {
    x = lo; y = hi;
    val = v;
    _update(0, 0, len - 1);
  }

  //returns merge(a[i] for i = lo..hi, inclusive)
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
  int arr[5] = {6, 4, 1, 8, 10};
  segment_tree<int> T(5, arr);
  cout << "Array contains:"; //6 4 1 8 10
  for (int i = 0; i < T.size(); i++)
    cout << " " << T.at(i);
  cout << "\n";
  T.update(2, 4, 12);
  cout << "Array contains:"; //6 4 12 12 12
  for (int i = 0; i < T.size(); i++)
    cout << " " << T.at(i);
  cout << "\nThe max value in the range [0, 3] is ";
  cout << T.query(0, 3) << ".\n"; //12
  return 0;
}
