/*

3.2.3 - 2D Binary Indexed Tree

Description: A 2D BIT is abstractly a 2D array which also
supports efficient queries for the sum of values in the
rectangle with top-left (1,1) and bottom-right (r,c). The
2D BIT implemented below has indices accessible in the
range [1...MAXR-1][1...MAXC].

Time Complexity: All functions are O(log(MAXR)*log(MAXC)).
Space Complexity: O(MAXR*MAXC) storage and auxiliary.

=~=~=~=~= Sample Output =~=~=~=~=
2D BIT values:
5 6 0
3 0 0
0 0 9

*/

const int MAXR = 100, MAXC = 100;

int a[MAXR+1][MAXC+1], bit[MAXR+1][MAXC+1];

//a[r][c] += v
void add(int r, int c, int v) {
  a[r][c] += v;
  for (int i = r; i < MAXR; i += i & -i)
    for (int j = c; j < MAXC; j += j & -j)
      bit[i][j] += v;
}

 //a[r][c] = v
void set(int r, int c, int v) {
  int inc = v - a[r][c];
  add(r, c, inc);
}

//returns sum(data[1..r][1..c], all inclusive)
int sum(int r, int c) {
  int ret = 0;
  for (int i = r; i > 0; i -= i & -i)
    for (int j = c; j > 0; j -= j & -j)
      ret += bit[i][j];
  return ret;
}

//returns sum(data[r1..r2][c1..c2], all inclusive)
int sum(int r1, int c1, int r2, int c2) {
  return sum(r2, c2) + sum(r1 - 1, c1 - 1) -
         sum(r1 - 1, c2) - sum(r2, c1 - 1);
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
