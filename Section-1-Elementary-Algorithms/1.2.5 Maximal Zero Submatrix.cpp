/*

1.2.5 - Maximal Zero Submatrix

Given a rectangular matrix of 0's and 1's, determine the
area of the largest submatrix which contains only 0's.

Time Complexity: O(n * m), where n is the number of rows
and m is the number of columns in the matrix.

Space Complexity: O(m) auxiliary on the number of columns.

*/

#include <algorithm> /* std::max() */
#include <vector>

typedef std::vector<std::vector<bool> > matrix;

int max_zero_submatrix(const matrix & a) {
  int n = a.size(), m = a[0].size(), res = 0;
  std::vector<int> d(m, -1), d1(m), d2(m), st(m);
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < m; c++)
      if (a[r][c]) d[c] = r;
    int sz = 0;
    for (int c = 0; c < m; c++) {
      while (sz > 0 && d[st[sz - 1]] <= d[c]) sz--;
      d1[c] = sz == 0 ? -1 : st[sz - 1];
      st[sz++] = c;
    }
    sz = 0;
    for (int c = m - 1; c >= 0; c--) {
      while (sz > 0 && d[st[sz - 1]] <= d[c]) sz--;
      d2[c] = sz == 0 ? m : st[sz - 1];
      st[sz++] = c;
    }
    for (int j = 0; j < m; j++)
      res = std::max(res, (r - d[j]) * (d2[j] - d1[j] - 1));
  }
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  const int n = 5, m = 6;
  bool a[n][m] = {{1, 0, 1, 1, 0, 0},
                  {1, 0, 0, 1, 0, 0},
                  {0, 0, 0, 0, 0, 1},
                  {1, 0, 0, 1, 0, 0},
                  {1, 0, 1, 0, 0, 1}};
  matrix v(n);
  for (int i = 0; i < 5; i++)
    v[i] = vector<bool>(a[i], a[i] + m);
  cout << max_zero_submatrix(v) << "\n";
  return 0;
}
