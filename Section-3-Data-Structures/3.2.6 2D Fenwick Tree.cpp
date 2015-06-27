/*

3.2.6 - 2D Fenwick Tree (Point Update, Range Query)

Description: A 2D Fenwick tree is abstractly a 2D array which also
supports efficient queries for the sum of values in the rectangle
with top-left (1, 1) and bottom-right (r, c). The implementation
below has indices accessible in the range [1...xmax][1...ymax].

Time Complexity: All functions are O(log(xmax)*log(ymax)).
Space Complexity: O(xmax*ymax) storage and auxiliary.

*/

const int xmax = 100, ymax = 100;

int a[xmax+1][ymax+1], bit[xmax+1][ymax+1];

//a[x][y] += v
void add(int x, int y, int v) {
  a[x][y] += v;
  for (int i = x; i <= xmax; i += i & -i)
    for (int j = y; j <= ymax; j += j & -j)
      bit[i][j] += v;
}

 //a[x][y] = v
void set(int x, int y, int v) {
  int inc = v - a[x][y];
  add(x, y, inc);
}

//returns sum(data[1..x][1..y], all inclusive)
int sum(int x, int y) {
  int ret = 0;
  for (int i = x; i > 0; i -= i & -i)
    for (int j = y; j > 0; j -= j & -j)
      ret += bit[i][j];
  return ret;
}

//returns sum(data[x1..x2][y1..y2], all inclusive)
int sum(int x1, int y1, int x2, int y2) {
  return sum(x2, y2) + sum(x1 - 1, y1 - 1) -
         sum(x1 - 1, y2) - sum(x2, y1 - 1);
}

/*** Example Usage ***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  set(1, 1, 5);
  set(1, 2, 6);
  set(2, 1, 7);
  add(3, 3, 9);
  add(2, 1, -4);
/*
  5 6 0
  3 0 0
  0 0 9
*/
  cout << "2D BIT values:\n";
  for (int i = 1; i <= 3; i++) {
    for (int j = 1; j <= 3; j++)
      cout << a[i][j] << " ";
    cout << "\n";
  }
  assert(sum(1, 1, 1, 2) == 11);
  assert(sum(1, 1, 2, 1) == 8);
  assert(sum(1, 1, 3, 3) == 23);
  return 0;
}
