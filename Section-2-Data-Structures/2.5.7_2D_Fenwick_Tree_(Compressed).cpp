/*

Maintain a 2D array of numerical type, allowing for rectangular sub-matrices to
be simultaneously incremented by arbitrary values (range update) and queries for
the sum of rectangular sub-matrices (range query). This implementation uses
std::map for coordinate compression, allowing for large indices to be accessed
with efficient space complexity. That is, rows have valid indices from 0 to
MAXR, inclusive, and columns have valid indices from 0 to MAXC, inclusive.

- add(r, c, x) adds x to the value at index (r, c).
- add(r1, c1, r2, c2, x) adds x to all indices in the rectangle with upper-left
  corner (r1, c1) and lower-right corner (r2, c2).
- set(r, c, x) assigns x to the value at index (r, c).
- sum(r, c) returns the sum of the rectangle with upper-left corner (0, 0) and
  lower-right corner (r, c).
- sum(r1, c1, r2, c2) returns the sum of the rectangle with upper-left corner
  (r1, c1) and lower-right corner (r2, c2).
- at(r, c) returns the value at index (r, c).

Time Complexity:
- O(log^2(MAXR)*log^2(MAXC)) per call to all member functions. If std::map is
  replaced with std::unordered_map, then the amortized running time will become
  O(log(MAXR)*log(MAXC)).

Space Complexity:
- O(n*log(MAXR)*log(MAXC)) for storage of the array elements, where n is the
  number of distinct indices that have been accessed across all of the
  operations so far.
- O(1) auxiliary for all operations.

*/

#include <map>
#include <utility>

template<class T>
class fenwick_tree_2d {
  static const int MAXR = 1000000001;
  static const int MAXC = 1000000001;
  std::map<std::pair<int, int>, T> t1, t2, t3, t4;

  template<class Map>
  void add(Map &tree, int r, int c, const T &x) {
    for (int i = r + 1; i <= MAXR; i += i & -i) {
      for (int j = c + 1; j <= MAXC; j += j & -j) {
        tree[std::make_pair(i, j)] += x;
      }
    }
  }

  void add_helper(int r, int c, const T &x) {
    add(t1, 0, 0, x);
    add(t1, 0, c, -x);
    add(t2, 0, c, x*c);
    add(t1, r, 0, -x);
    add(t3, r, 0, x*r);
    add(t1, r, c, x);
    add(t2, r, c, -x*c);
    add(t3, r, c, -x*r);
    add(t4, r, c, x*r*c);
  }

 public:
  void add(int r1, int c1, int r2, int c2, const T &x) {
    add_helper(r2 + 1, c2 + 1, x);
    add_helper(r1, c2 + 1, -x);
    add_helper(r2 + 1, c1, -x);
    add_helper(r1, c1, x);
  }

  void add(int r, int c, const T &x) {
    add(r, c, r, c, x);
  }

  void set(int r, int c, const T &x) {
    add(r, c, x - at(r, c));
  }

  T sum(int r, int c) {
    r++;
    c++;
    T s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    for (int i = r; i > 0; i -= i & -i) {
      for (int j = c; j > 0; j -= j & -j) {
        const std::pair<int, int> ij(i, j);
        s1 += t1[ij];
        s2 += t2[ij];
        s3 += t3[ij];
        s4 += t4[ij];
      }
    }
    return s1*r*c + s2*r + s3*c + s4;
  }

  T sum(int r1, int c1, int r2, int c2) {
    return sum(r2, c2) + sum(r1 - 1, c1 - 1) -
           sum(r1 - 1, c2) - sum(r2, c1 - 1);
  }

  T at(int r, int c) {
    return sum(r, c, r, c);
  }
};

/*** Example Usage and Output:

Values:
5 6 0
3 5 5
0 5 14

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  fenwick_tree_2d<int> t;
  t.set(0, 0, 5);
  t.set(0, 1, 6);
  t.set(1, 0, 7);
  t.add(2, 2, 9);
  t.add(1, 0, -4);
  t.add(1, 1, 2, 2, 5);
  cout << "Values:" << endl;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      cout << t.at(i, j) << " ";
    }
    cout << endl;
  }
  assert(t.sum(0, 0, 0, 1) == 11);
  assert(t.sum(0, 0, 1, 0) == 8);
  assert(t.sum(1, 1, 2, 2) == 29);
  t.set(500000000, 500000000, 100);
  assert(t.sum(0, 0, 1000000000, 1000000000) == 143);
  return 0;
}
