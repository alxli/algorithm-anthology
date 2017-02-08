/*

Given an n by m rectangular matrix of 0's and 1's as a two dimensional vector of
bool, returns the area of the largest rectangular submatrix consisting of only
0's. This can be reduced to the problem of finding the maximum rectangular area
under a histogram, which is efficiently solved using a stack.

Explanation: http://stackoverflow.com/a/13657337

Time Complexity: O(n * m) for a matrix n rows by m columns.
Space Complexity: O(m) auxiliary.

*/

#include <algorithm>  // std::max()
#include <stack>
#include <vector>

int max_zero_submatrix(const std::vector< std::vector<bool> > &matrix) {
  int n = matrix.size(), m = matrix[0].size(), res = 0;
  std::vector<int> d(m, -1), d1(m), d2(m);
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < m; c++) {
      if (matrix[r][c])
        d[c] = r;
    }
    std::stack<int> s;
    for (int c = 0; c < m; c++) {
      while (!s.empty() && d[s.top()] <= d[c])
        s.pop();
      d1[c] = s.empty() ? -1 : s.top();
      s.push(c);
    }
    while (!s.empty())
      s.pop();
    for (int c = m - 1; c >= 0; c--) {
      while (!s.empty() && d[s.top()] <= d[c])
        s.pop();
      d2[c] = s.empty() ? m : s.top();
      s.push(c);
    }
    for (int j = 0; j < m; j++)
      res = std::max(res, (r - d[j])*(d2[j] - d1[j] - 1));
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  const int n = 5, m = 6;
  bool a[n][m] = {{1, 0, 1, 1, 0, 0},
                  {1, 0, 0, 1, 0, 0},
                  {0, 0, 0, 0, 0, 1},
                  {1, 0, 0, 1, 0, 0},
                  {1, 0, 1, 0, 0, 1}};
  vector<vector<bool> > mat(n);
  for (int i = 0; i < n; i++) {
    mat[i] = vector<bool>(a[i], a[i] + m);
  }
  assert(max_zero_submatrix(mat) == 6);
  return 0;
}
