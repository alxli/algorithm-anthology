/*

1.2.5 - Maximal Zero Submatrix

Given an n by m rectangular matrix of 0's and 1's, determine the area
of the largest rectangular submatrix which contains only 0's. This can
be reduced the problem of finding the maximum rectangular area under a
histogram, which can be efficiently solved using a stack. The following
implementation accepts a 2-dimensional vector of bools and returns the
area of the maximum zero submatrix.

Explanation: http://stackoverflow.com/a/13657337

Time Complexity: O(n * m) for a matrix n rows by m columns.
Space Complexity: O(m) auxiliary.

*/

#include <algorithm> /* std::max() */
#include <vector>

int max_zero_submatrix(const std::vector< std::vector<bool> > & mat) {
  int n = mat.size(), m = mat[0].size(), res = 0;
  std::vector<int> d(m, -1), d1(m), d2(m), stack;
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < m; c++) {
      if (mat[r][c])
        d[c] = r;
    }
    stack.clear();
    for (int c = 0; c < m; c++) {
      while (!stack.empty() && d[stack.back()] <= d[c])
        stack.pop_back();
      d1[c] = stack.empty() ? -1 : stack.back();
      stack.push_back(c);
    }
    stack.clear();
    for (int c = m - 1; c >= 0; c--) {
      while (!stack.empty() && d[stack.back()] <= d[c])
        stack.pop_back();
      d2[c] = stack.empty() ? m : stack.back();
      stack.push_back(c);
    }
    for (int j = 0; j < m; j++)
      res = std::max(res, (r - d[j]) * (d2[j] - d1[j] - 1));
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  const int n = 5, m = 6;
  bool a[n][m] = {{1, 0, 1, 1, 0, 0},
                  {1, 0, 0, 1, 0, 0},
                  {0, 0, 0, 0, 0, 1},
                  {1, 0, 0, 1, 0, 0},
                  {1, 0, 1, 0, 0, 1}};
  std::vector< std::vector<bool> > mat(n);
  for (int i = 0; i < n; i++)
    mat[i] = vector<bool>(a[i], a[i] + m);
  assert(max_zero_submatrix(mat) == 6);
  return 0;
}
