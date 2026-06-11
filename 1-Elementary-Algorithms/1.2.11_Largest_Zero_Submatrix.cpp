/*

Given a rectangular matrix with $m$ rows and $n$ columns consisting of only 0's and 1's as a
two-dimensional vector of bool, return the area of the largest rectangular submatrix consisting of
only 0's. This solution sweeps the rows from top to bottom, maintaining for each column the height
of the run of 0's ending at the current row. Each row then reduces to finding the maximum
rectangular area under a histogram, which is solved in linear time with a monotonic stack.

- `max_zero_submatrix(a)` returns the area of the largest all-zero rectangular submatrix of `a`,
  a two-dimensional vector of bool.

Time Complexity:
- O(m*n) per call to `max_zero_submatrix()`, where $m$ is the number of rows and $n$ is the number
  of columns in the matrix.

Space Complexity:
- O(n) auxiliary heap space, where $n$ is the number of columns in the matrix.

*/

#include <algorithm>
#include <stack>
#include <vector>

int max_zero_submatrix(const std::vector<std::vector<bool>> &a) {
  if (a.empty()) {
    return 0;
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  int res = 0;
  // last1[j] = row index of the most recent 1 in column j (-1 if none yet).
  // left_wall[j] = nearest col left of j whose last1 value >= last1[j] (exclusive left bound).
  // right_wall[j] = nearest col right of j whose last1 value >= last1[j] (exclusive right bound).
  std::vector<int> last1(cols, -1), left_wall(cols), right_wall(cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      if (a[i][j]) {
        last1[j] = i;
      }
    }
    std::stack<int> s;
    for (int j = 0; j < cols; j++) {
      while (!s.empty() && last1[s.top()] <= last1[j]) {
        s.pop();
      }
      left_wall[j] = s.empty() ? -1 : s.top();
      s.push(j);
    }
    while (!s.empty()) {
      s.pop();
    }
    for (int j = cols - 1; j >= 0; j--) {
      while (!s.empty() && last1[s.top()] <= last1[j]) {
        s.pop();
      }
      right_wall[j] = s.empty() ? cols : s.top();
      s.push(j);
    }
    for (int j = 0; j < cols; j++) {
      res = std::max(res, (i - last1[j]) * (right_wall[j] - left_wall[j] - 1));
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<bool>> a{
      {1, 0, 1, 1, 0, 0},
      {1, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0, 1},
      {1, 0, 0, 1, 0, 0},
      {1, 0, 1, 0, 0, 1}
  };
  assert(max_zero_submatrix(a) == 6);
  return 0;
}
