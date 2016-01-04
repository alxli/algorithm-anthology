/*

Description: A Fenwick tree (a.k.a. binary indexed tree) is a
data structure that allows for the sum of an arbitrary range
of values in an array to be dynamically queried in logarithmic
time. All methods below work on 0-based indices (i.e. indices
in the range from 0 to size() - 1, inclusive, are valid).

Time Complexity: add(), set(), and sum() are all O(log N) on
the length of the array. size() and at() are O(1).

Space Complexity: O(N) storage and O(N) auxiliary on size().

*/

#include <vector>

template<class T> class fenwick_tree {
  int len;
  std::vector<int> a, bit;

 public:
  fenwick_tree(int n): len(n),
   a(n + 1), bit(n + 1) {}

  //a[i] += v
  void add(int i, const T & v) {
    a[++i] += v;
    for (; i <= len; i += i & -i)
      bit[i] += v;
  }

  //a[i] = v
  void set(int i, const T & v) {
    T inc = v - a[i + 1];
    add(i, inc);
  }

  //returns sum(a[i] for i = 1..hi inclusive)
  T sum(int hi) {
    T res = 0;
    for (hi++; hi > 0; hi -= hi & -hi)
      res += bit[hi];
    return res;
  }

  //returns sum(a[i] for i = lo..hi inclusive)
  T sum(int lo, int hi) {
    return sum(hi) - sum(lo - 1);
  }

  inline int size() { return len; }
  inline T at(int i) { return a[i + 1]; }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  fenwick_tree<int> t(5);
  for (int i = 0; i < 5; i++) t.set(i, a[i]);
  t.add(0, -5);
  cout << "BIT values: ";
  for (int i = 0; i < t.size(); i++)
    cout << t.at(i) << " "; //5 1 2 3 4
  cout << "\nSum of range [1, 3] is ";
  cout << t.sum(1, 3) << ".\n"; //6
  return 0;
}
