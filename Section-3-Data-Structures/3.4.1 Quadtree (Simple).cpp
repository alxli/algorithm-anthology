/*

3.4.1 - Quadtree (Simple)

Description: A quadtree can be used to dynamically query values
of rectangles in a 2D array. In a quadtree, every node has exactly
4 children. The following uses a statically allocated array to
store the nodes. This is less efficient than a 2D segment tree.

Time Complexity: For update(), query() and at(): O(log(N*M)) on
average and O(sqrt(N*M)) in the worst case, where N is the number
of rows and M is the number of columns in the 2D array.

Space Complexity: O(N*M)

Note: This implementation is 0-based. Valid indices for
all operations are [0..XMAX][0..YMAX]

*/

#include <climits> /* INT_MIN */

const int XMAX = 100, YMAX = 100;
int tree[4*XMAX*YMAX];
int X, Y, XLO, XHI, YLO, YHI, V; //temp vals for speed

//define the following yourself. merge(x, nullv) must return x for all valid x
inline int nullv() {
  return INT_MIN;
}

inline int merge(int a, int b) {
  return a > b ? a : b;
}

void update(int n, int xl, int xh, int yl, int yh) {
  if (X < xl || X > xh || Y < yl || Y > yh) return;
  if (xl == xh && yl == yh) {
    tree[n] = V;
    return;
  }
  update(n*4 + 1, xl, (xl + xh)/2, yl, (yl + yh)/2);
  update(n*4 + 2, xl, (xl + xh)/2, (yl + yh)/2 + 1, yh);
  update(n*4 + 3, (xl + xh)/2 + 1, xh, yl, (yl + yh)/2);
  update(n*4 + 4, (xl + xh)/2 + 1, xh, (yl + yh)/2 + 1, yh);
  tree[n] = merge(merge(tree[n*4 + 1], tree[n*4 + 2]),
                  merge(tree[n*4 + 3], tree[n*4 + 4]));
}

void query(int n, int xl, int xh, int yl, int yh) {
  if (xl > XHI || xh < XLO || yh < YLO || yl > YHI || merge(tree[n], V) == V)
    return;
  if (xl >= XLO && xh <= XHI && yl >= YLO && yh <= YHI) {
    V = merge(tree[n], V);
    return;
  }
  query(n*4 + 1, xl, (xl + xh)/2, yl, (yl + yh)/2);
  query(n*4 + 2, xl, (xl + xh)/2, (yl + yh)/2 + 1, yh);
  query(n*4 + 3, (xl + xh)/2 + 1, xh, yl, (yl + yh)/2);
  query(n*4 + 4, (xl + xh)/2 + 1, xh, (yl + yh)/2 + 1, yh);
}

void update(int x, int y, int v) {
  X = x; Y = y; V = v;
  update(0, 0, XMAX - 1, 0, YMAX - 1);
}

int query(int xl, int yl, int xh, int yh) {
  XLO = xl; XHI = xh;
  YLO = yl; YHI = yh;
  V = nullv();
  query(0, 0, XMAX - 1, 0, YMAX - 1);
  return V;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  int arr[5][5] = {{1, 2, 3, 4, 5},
                   {5, 4, 3, 2, 1},
                   {6, 7, 8, 0, 0},
                   {0, 1, 2, 3, 4},
                   {5, 9, 9, 1, 2}};
  for (int r = 0; r < 5; r++)
    for (int c = 0; c < 5; c++)
      update(r, c, arr[r][c]);
  cout << "The maximum value in the rectangle with ";
  cout << "upper left (0,2) and lower right (3,4) is ";
  cout << query(0, 2, 3, 4) << ".\n"; //8
  return 0;
}
