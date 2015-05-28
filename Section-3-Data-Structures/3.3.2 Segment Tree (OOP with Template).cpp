/*

3.3.2 - 1D Segment Tree (OOP version with templates)

Description: A segment tree is a data structure used for
solving the dynamic range query problem, which asks to
determine the minimum (or maximum) value in any given
range in an array that is constantly being updated.

Time Complexity: Assuming merge() is O(1), build is O(N)
while query(), update(), at() are O(log(N)). If merge() is
not constant time, then all running times are multiplied
by whatever complexity the merge function runs in.

Space Complexity: O(N) on the size of the array. A segment
tree for an array of size N needs 2^(log2(N)-1) = 4N nodes.

Note: This implementation is 0-based, meaning that all
indices from 0 to N - 1, inclusive, are accessible.

*/

#include <limits> /* std::numeric_limits<T>::min() */

template<class T> class segment_tree {
  int SIZE, x, y;
  T *data, val;
    
  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline const T nullv() { return std::numeric_limits<T>::min(); }
  static inline const T merge(const T &a, const T &b) { return a > b ? a : b; }

  void internal_update(int node, int lo, int hi) {
    if (x < lo || x > hi) return;
    if (lo == hi) {
      data[node] = val;
      return;
    }
    internal_update(node*2 + 1, lo, (lo + hi)/2);
    internal_update(node*2 + 2, (lo + hi)/2 + 1, hi);
    data[node] = merge(data[node*2 + 1], data[node*2 + 2]);
  }

  T internal_query(int node, int lo, int hi) {
    if (hi < x || lo > y) return nullv();
    if (lo >= x && hi <= y) return data[node];
    return merge(internal_query(node*2 + 1, lo, (lo + hi)/2),
                 internal_query(node*2 + 2, (lo + hi)/2 + 1, hi)); 
  }

 public:
  segment_tree(int N): SIZE(N) { data = new T[4*N]; }
  ~segment_tree() { delete[] data; }
  int size() { return SIZE; }
  
  void update(int idx, const T &v) {
    x = idx;
    val = v;
    internal_update(0, 0, SIZE - 1);
  }
  
  T query(int lo, int hi) {
    x = lo;
    y = hi;
    return internal_query(0, 0, SIZE - 1);
  }
  
  T at(int idx) { return query(idx, idx); }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int arr[5] = {6, 4, 1, 8, 10}; 
  segment_tree<int> T(5);
  for (int i = 0; i < 5; i++) T.update(i, arr[i]);
  cout << "Array contains:";
  for (int i = 0; i < T.size(); i++)
    cout << " " << T.at(i);
  cout << "\nThe max value in the range [0, 3] is ";
  cout << T.query(0, 3) << ".\n"; //8
  return 0;
} 
