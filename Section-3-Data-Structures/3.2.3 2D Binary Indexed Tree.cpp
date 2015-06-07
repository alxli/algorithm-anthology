/*

3.2.3 - 2D Binary Indexed Tree

Description: A 2D BIT is abstractly a 2D array which also
supports efficient queries for the sum of values in the
rectangle with bottom-left (0,0) and top-right (x,y). The
2D BIT implemented below has indices accessible in the
range [1...MAXR-1][1...MAXC].

Time Complexity: query() and update() are both
O(log(rows)*log(cols)). All other functions are O(1).

Space Complexity: O(rows*cols) storage and auxiliary.

*/

const int MAXR = 100, MAXC = 100;

int data[MAXR+1][MAXC+1];
int tree[MAXR+1][MAXC+1];

void update_pre(int R, int C, int v) { //data[R][C] += v
  for (int r = R; r < MAXR; r += r & -r)
    for (int c = C; c < MAXC; c += c & -c)
      tree[r][c] += v;
}

void update(int r, int c, int v) { //data[R][C] = v
  int inc = v - data[r][c];
  data[r][c] = v;
  update_pre(r, c, inc);
}

int query(int R, int C) { //sum(data[1..R][1..C])
  int ret = 0;
  for (int r = R; r > 0; r -= r & -r)
    for (int c = C; c > 0; c -= c & -c)
      ret += tree[r][c];
  return ret;
}

int query(int r1, int c1, int r2, int c2) {
  return query(r2, c2) + query(r1 - 1, c1 - 1) -
         query(r1 - 1, c2) - query(r2, c1 - 1);
}

/*** Example Usage: ***/

#include <iostream>
using namespace std;

int main() {
  update(1, 1, 5);
  update(1, 2, 6);
  update(2, 1, 7);
  cout << query(1, 1, 1, 2) << endl; //11
  cout << query(1, 1, 2, 1) << endl; //12
  cout << query(1, 1, 2, 2) << endl; //18
  return 0;
}
