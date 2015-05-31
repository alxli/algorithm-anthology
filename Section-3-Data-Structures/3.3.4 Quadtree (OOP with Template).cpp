/*

3.3.4 - Quadtree (OOP version with templates)

Description: A quadtree can be used to dynamically query values
of rectangles in a 2D array. In a quadtree, every node has exactly
4 children. The following uses a statically allocated array to
store the nodes. This is less efficient than a 2D segment tree.

Time Complexity: For update(), query() and at(): O(log(N*M)) on
average and O(sqrt(N*M)) in the worst case. Where N is the number
of rows and M is the number of columns in the 2D array.

Space Complexity: O(N*M log(N*M))

Note: This implementation is 0-based, meaning that all
indices from 0 to N - 1, inclusive, are accessible.

*/

#include <cmath> /* ceil(), log2() */
#include <limits> /* std::numeric_limits<T>::min() */

template<class T> class quadtree {
  int xlen, ylen, x, y, XLO, XHI, YLO, YHI;
  T *tree, val;
    
  //define the following yourself. merge(x, nullv) must return x for all valid x
  static inline const T nullv() { return std::numeric_limits<T>::min(); }
  static inline const T merge(const T & a, const T & b) { return a > b ? a : b; }

  void internal_update(int n, int xlo, int xhi, int ylo, int yhi) {
    if (x < xlo || x > xhi || y < ylo || y > yhi) return;
    if (xlo == xhi && ylo == yhi) {
      tree[n] = val;
      return;
    }
    internal_update(n*4 + 1, xlo, (xlo + xhi)/2, ylo, (ylo + yhi)/2);
    internal_update(n*4 + 2, xlo, (xlo + xhi)/2, (ylo + yhi)/2 + 1, yhi);
    internal_update(n*4 + 3, (xlo + xhi)/2 + 1, xhi, ylo, (ylo + yhi)/2);
    internal_update(n*4 + 4, (xlo + xhi)/2 + 1, xhi, (ylo + yhi)/2 + 1, yhi);
    tree[n] = merge(merge(tree[n*4 + 1], tree[n*4 + 2]),
                    merge(tree[n*4 + 3], tree[n*4 + 4]));

  }

  void internal_query(int n, int xlo, int xhi, int ylo, int yhi) {
    if (xlo > XHI || xhi < XLO || yhi < YLO || ylo > YHI || merge(tree[n], val) == val) return;
    if (xlo >= XLO && xhi <= XHI && ylo >= YLO && yhi <= YHI) {
      val = merge(tree[n], val);
      return;
    }
    internal_query(n*4 + 1, xlo, (xlo + xhi)/2, ylo, (ylo + yhi)/2);
    internal_query(n*4 + 2, xlo, (xlo + xhi)/2, (ylo + yhi)/2 + 1, yhi);
    internal_query(n*4 + 3, (xlo + xhi)/2 + 1, xhi, ylo, (ylo + yhi)/2);
    internal_query(n*4 + 4, (xlo + xhi)/2 + 1, xhi, (ylo + yhi)/2 + 1, yhi);
  }

 public:
  quadtree(int N, int M): xlen(N), ylen(M) {
    tree = new T[4*N*M*(int)ceil(log2(4*N*M))];
  }
  
  ~quadtree() { delete[] tree; }
  int xmax() { return xlen; }
  int ymax() { return ylen; }
  T at(int x, int y) { return query(x, x, y, y); }
  
  void update(int x, int y, const T & v) {
    this->x = x; this->y = y;
    val = v;
    internal_update(0, 0, xlen - 1, 0, ylen - 1);
  }
  
  T query(int xlo, int ylo, int xhi, int yhi) {
    XLO = xlo; XHI = xhi; YLO = ylo; YHI = yhi;
    val = nullv();
    internal_query(0, 0, xlen - 1, 0, ylen - 1);
    return val;
  }
};

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int arr[5][5] = {{1, 2, 3, 4, 5},
                   {5, 4, 3, 2, 1},
                   {6, 7, 8, 0, 0},
                   {0, 1, 2, 3, 4},
                   {5, 9, 9, 1, 2}};
  quadtree<int> T(5, 5);
  for (int r = 0; r < T.xmax(); r++)
    for (int c = 0; c < T.ymax(); c++)
      T.update(r, c, arr[r][c]);
  cout << "The maximum value in the rectangle with ";
  cout << "upper left (0,2) and lower right (3,4) is ";
  cout << T.query(0, 2, 3, 4) << ".\n"; //8
  return 0;
}
