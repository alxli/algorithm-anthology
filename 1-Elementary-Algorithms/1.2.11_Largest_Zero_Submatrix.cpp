/*

Given a rectangular matrix with $n$ rows and $m$ columns consisting of only 0's and 1's as a
two-dimensional vector of bool, return the area of the largest rectangular submatrix consisting of
only 0's. This solution uses a reduction to the problem of finding the maximum rectangular area
under a histogram, which is efficiently solved using a stack algorithm.

Time Complexity:
- O(n*m) per call to `max_zero_submatrix()`, where $n$ is the number of rows and $m$ is the number
  of columns in the matrix.

Space Complexity:
- O(m) auxiliary heap space, where $m$ is the number of columns in the matrix.

*/

#include <algorithm>
#include <stack>
#include <vector>

int max_zero_submatrix(const std::vector<std::vector<bool>> &matrix) {
  if (matrix.empty()) {
    return 0;
  }
  int n = static_cast<int>(matrix.size()), m = static_cast<int>(matrix[0].size());
  int res = 0;
  // last1[c] = row index of the most recent 1 in column c (-1 if none yet).
  // left_wall[c] = nearest column left of c whose last1 value >= last1[c] (exclusive left bound).
  // right_wall[c] = nearest column right of c whose last1 value >= last1[c] (exclusive right bound).
  std::vector<int> last1(m, -1), left_wall(m), right_wall(m);
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < m; c++) {
      if (matrix[r][c]) {
        last1[c] = r;
      }
    }
    std::stack<int> s;
    for (int c = 0; c < m; c++) {
      while (!s.empty() && last1[s.top()] <= last1[c]) {
        s.pop();
      }
      left_wall[c] = s.empty() ? -1 : s.top();
      s.push(c);
    }
    while (!s.empty()) {
      s.pop();
    }
    for (int c = m - 1; c >= 0; c--) {
      while (!s.empty() && last1[s.top()] <= last1[c]) {
        s.pop();
      }
      right_wall[c] = s.empty() ? m : s.top();
      s.push(c);
    }
    for (int j = 0; j < m; j++) {
      res = std::max(res, (r - last1[j]) * (right_wall[j] - left_wall[j] - 1));
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<bool>> matrix{
      {1, 0, 1, 1, 0, 0},
      {1, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0, 1},
      {1, 0, 0, 1, 0, 0},
      {1, 0, 1, 0, 0, 1}
  };
  assert(max_zero_submatrix(matrix) == 6);
  return 0;
}
