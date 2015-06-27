/*

3.2.6 - 2D Fenwick Tree (Range Update, Range Query,
                         with Coordinate Compression)

Description: A 2D Fenwick tree is abstractly a 2D array which also
supports efficient queries for the sum of values in the rectangle
with top-left (1, 1) and bottom-right (r, c). The implementation
below has indices accessible in the range [0..xmax][0...ymax].

Time Complexity: All functions are O(log(xmax)*log(ymax)*log(N))
where N is the number of indices operated on so far. Use an array
or an unordered_map instead of a map to remove the log(N) factor.

Space Complexity: O(xmax*ymax) storage and auxiliary.

*/

#include <map>
#include <utility>

template<class T> class fenwick_tree_2d {
  static const int xmax = 1000000000;
  static const int ymax = 1000000000;

  std::map<std::pair<int, int>, T> t1, t2, t3, t4;

  template<class Tree>
  void add(Tree & t, int x, int y, const T & v) {
    for (int i = x; i <= xmax; i += i & -i)
      for (int j = y; j <= ymax; j += j & -j)
        t[std::make_pair(i, j)] += v;
  }

  //a[i][j] += v for i = [1,x], j = [1,y]
  void add_pre(int x, int y, const T & v) {
    add(t1, 1, 1, v);

    add(t1, 1, y + 1, -v);
    add(t2, 1, y + 1, v * y);

    add(t1, x + 1, 1, -v);
    add(t3, x + 1, 1, v * x);

    add(t1, x + 1, y + 1, v);
    add(t2, x + 1, y + 1, -v * y);
    add(t3, x + 1, y + 1, -v * x);
    add(t4, x + 1, y + 1, v * x * y);
  }

 public:
  //a[i][j] += v for i = [x1,x2], j = [y1,y2]
  void add(int x1, int y1, int x2, int y2, const T & v) {
    x1++; y1++; x2++; y2++;
    add_pre(x2, y2, v);
    add_pre(x1 - 1, y2, -v);
    add_pre(x2, y1 - 1, -v);
    add_pre(x1 - 1, y1 - 1, v);
  }

  //a[x][y] += v
  void add(int x, int y, const T & v) {
    add(x, y, x, y, v);
  }

  //a[x][y] = v
  void set(int x, int y, const T & v) {
    add(x, y, v - at(x, y));
  }

  //returns sum(a[i][j] for i = [1,x], j = [1,y])
  T sum(int x, int y) {
    x++; y++;
    T s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    for (int i = x; i > 0; i -= i & -i)
      for (int j = y; j > 0; j -= j & -j) {
        s1 += t1[std::make_pair(i, j)];
        s2 += t2[std::make_pair(i, j)];
        s3 += t3[std::make_pair(i, j)];
        s4 += t4[std::make_pair(i, j)];
      }
    return s1 * x * y + s2 * x + s3 * y + s4;
  }

  //returns sum(a[i][j] for i = [x1,x2], j = [y1,y2])
  T sum(int x1, int y1, int x2, int y2) {
    return sum(x2, y2) + sum(x1 - 1, y1 - 1) -
           sum(x1 - 1, y2) - sum(x2, y1 - 1);
  }

  T at(int x, int y) { return sum(x, y, x, y); }
};

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  fenwick_tree_2d<long long> t;
  t.set(0, 0, 5);
  t.set(0, 1, 6);
  t.set(1, 0, 7);
  t.add(2, 2, 9);
  t.add(1, 0, -4);
  t.add(1, 1, 2, 2, 5);
/*
  5 6 0
  3 5 5
  0 5 14
*/
  cout << "2D BIT values:\n";
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++)
      cout << t.at(i, j) << " ";
    cout << "\n";
  }
  assert(t.sum(0, 0, 0, 1) == 11);
  assert(t.sum(0, 0, 1, 0) == 8);
  assert(t.sum(1, 1, 2, 2) == 29);
  return 0;
}
