/*

Maintain a 2D array of numerical type, allowing for updates of individual cells
in the matrix (point update) and queries for the sum of rectangular sub-matrices
(range query). This implementation assumes that array dimensions are 1-based
(i.e. rows have valid indices from 1 to MAXR, inclusive, and columns have valid
indices from 1 to MAXC, inclusive).

- initialize() resets the data structure.
- a[r][c] stores the value at index (r, c).
- add(r, c, x) adds x to the value at index (r, c).
- set(r, c, x) assigns x to the value at index (r, c).
- sum(r, c) returns the sum of the rectangle with upper-left corner (1, 1) and
  lower-right corner (r, c).
- sum(r1, c1, r2, c2) returns the sum of the rectangle with upper-left corner
  (r1, c1) and lower-right corner (r2, c2).

Time Complexity:
- O(n*m) per call to initialize(), where n is the number of rows and m is the
  number of columns.
- O(log(n)*log(m)) per call to all other operations.

Space Complexity:
- O(n*m) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

const int MAXR = 100, MAXC = 100;
int a[MAXR + 1][MAXC + 1];
int bits[MAXR + 1][MAXC + 1];

void initialize() {
  for (int i = 0; i <= MAXR; i++) {
    for (int j = 0; j <= MAXC; j++) {
      a[i][j] = bits[i][j] = 0;
    }
  }
}

void add(int r, int c, int x) {
  a[r][c] += x;
  for (int i = r; i <= MAXR; i += i & -i) {
    for (int j = c; j <= MAXC; j += j & -j) {
      bits[i][j] += x;
    }
  }
}

void set(int r, int c, int x) {
  add(r, c, x - a[r][c]);
}

int sum(int r, int c) {
  int res = 0;
  for (int i = r; i > 0; i -= i & -i) {
    for (int j = c; j > 0; j -= j & -j) {
      res += bits[i][j];
    }
  }
  return res;
}

int sum(int r1, int c1, int r2, int c2) {
  return sum(r2, c2) + sum(r1 - 1, c1 - 1) -
         sum(r1 - 1, c2) - sum(r2, c1 - 1);
}

/*** Example Usage and Output:

Values:
5 6 0
3 0 0
0 0 9

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  initialize();
  set(1, 1, 5);
  set(1, 2, 6);
  set(2, 1, 7);
  add(3, 3, 9);
  add(2, 1, -4);
  cout << "Values:" << endl;
  for (int i = 1; i <= 3; i++) {
    for (int j = 1; j <= 3; j++) {
      cout << a[i][j] << " ";
    }
    cout << endl;
  }
  assert(sum(1, 1, 1, 2) == 11);
  assert(sum(1, 1, 2, 1) == 8);
  assert(sum(1, 1, 3, 3) == 23);
  return 0;
}
