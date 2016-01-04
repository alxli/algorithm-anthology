/*

Description: A quadtree can be used to dynamically query values
of rectangles in a 2D array. In a quadtree, every node has exactly
4 children. The following uses a statically allocated array to
store the nodes. This is less efficient than a 2D segment tree.

Time Complexity: For update(), query() and at(): O(log(N*M)) on
average and O(sqrt(N*M)) in the worst case, where N is the number
of rows and M is the number of columns in the 2D array.

Space Complexity: O(N*M)

Note: This implementation is 0-based. Valid indices for
all operations are [0..xmax][0..ymax]

*/

#include <climits> /* INT_MIN */

const int xmax = 100, ymax = 100;
int tree[4 * xmax * ymax];
int X, Y, X1, X2, Y1, Y2, V; //temporary value to speed up recursion

//define the following yourself. merge(x, nullv) must return x for all valid x
inline int nullv() { return INT_MIN; }
inline int merge(int a, int b) { return a > b ? a : b; }

void update(int n, int x1, int x2, int y1, int y2) {
  if (X < x1 || X > x2 || Y < y1 || Y > y2) return;
  if (x1 == x2 && y1 == y2) {
    tree[n] = V;
    return;
  }
  update(n * 4 + 1, x1, (x1 + x2) / 2, y1, (y1 + y2) / 2);
  update(n * 4 + 2, x1, (x1 + x2) / 2, (y1 + y2) / 2 + 1, y2);
  update(n * 4 + 3, (x1 + x2) / 2 + 1, x2, y1, (y1 + y2) / 2);
  update(n * 4 + 4, (x1 + x2) / 2 + 1, x2, (y1 + y2) / 2 + 1, y2);
  tree[n] = merge(merge(tree[n * 4 + 1], tree[n * 4 + 2]),
                  merge(tree[n * 4 + 3], tree[n * 4 + 4]));
}

void query(int n, int x1, int x2, int y1, int y2) {
  if (x1 > X2 || x2 < X1 || y2 < Y1 || y1 > Y2 || merge(tree[n], V) == V)
    return;
  if (x1 >= X1 && x2 <= X2 && y1 >= Y1 && y2 <= Y2) {
    V = merge(tree[n], V);
    return;
  }
  query(n * 4 + 1, x1, (x1 + x2) / 2, y1, (y1 + y2) / 2);
  query(n * 4 + 2, x1, (x1 + x2) / 2, (y1 + y2) / 2 + 1, y2);
  query(n * 4 + 3, (x1 + x2) / 2 + 1, x2, y1, (y1 + y2) / 2);
  query(n * 4 + 4, (x1 + x2) / 2 + 1, x2, (y1 + y2) / 2 + 1, y2);
}

void update(int x, int y, int v) {
  X = x;
  Y = y;
  V = v;
  update(0, 0, xmax - 1, 0, ymax - 1);
}

int query(int x1, int y1, int x2, int y2) {
  X1 = x1;
  X2 = x2;
  Y1 = y1;
  Y2 = y2;
  V = nullv();
  query(0, 0, xmax - 1, 0, ymax - 1);
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
