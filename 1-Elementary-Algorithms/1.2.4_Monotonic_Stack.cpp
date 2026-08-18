/*

A monotonic stack maintains its elements in sorted order as values are pushed, popping away any
entries that would violate the ordering. Scanning an array once while keeping such a stack of
indices answers, for every position, where the nearest smaller or larger neighbor lies. These
"nearest smaller/greater" relationships underlie many array problems, the most famous being the
largest rectangle in a histogram. Each index is pushed and popped at most once, so a full scan runs
in linear time. A largest all-zero submatrix can then be found by sweeping rows, turning each row
into a histogram of consecutive zeros ending at that row.

For any histogram bar, the nearest strictly lower bars to its left and right are the first positions
that prevent a rectangle at the current bar's height from extending farther. The widest such
rectangle therefore spans the positions strictly between those boundaries. For a binary matrix, the
height at each column counts consecutive zeros ending in the current row; every all-zero rectangle
has some bottom row, so solving one histogram per row considers every possible rectangle.

All functions below take a vector `a` of $n$ comparable values and return a vector of $n$ indices. A
"less" query uses a strictly smaller neighbor and a "greater" query uses a strictly larger one;
changing the comparison from strict to non-strict (e.g. `>=` to `>`) toggles how ties are handled.

- `prev_less(a)` returns, for each $i$, the largest index $j < i$ with $a[j] < a[i]$, or $-1$ if
  there's no such index.
- `next_less(a)` returns, for each $i$, the smallest index $j > i$ with $a[j] < a[i]$, or $n$ if
  there's no such index.
- `prev_greater(a)` returns, for each $i$, the largest index $j < i$ with $a[j] > a[i]$, or $-1$ if
  there's no such index.
- `next_greater(a)` returns, for each $i$, the smallest index $j > i$ with $a[j] > a[i]$, or $n$ if
  there's no such index.
- `largest_histogram_rectangle(heights)` returns the maximum area and its inclusive left and right
  endpoints in a histogram where bars have width $1$ and nonnegative heights given by `heights`.
- `largest_zero_submatrix(a)` returns the maximum area and its inclusive top, left, bottom, and
  right boundaries among all-zero rectangular submatrices of the 0/1 matrix `a`.

Overflow warning: Histogram areas must fit in the height value type.

Time Complexity:
- O(n) per call to all one-dimensional functions, where $n$ is the size of the input.
- O(R*C) per call to `largest_zero_submatrix()`, where $R$ and $C$ are the matrix dimensions.

Space Complexity:
- O(n) auxiliary and O(n) for the returned indices from each neighbor query.
- O(n) auxiliary for `largest_histogram_rectangle()`.
- O(C) auxiliary for `largest_zero_submatrix()`.

*/

#include <cassert>
#include <stack>
#include <tuple>
#include <vector>

template<typename T>
std::vector<int> prev_less(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = 0; i < n; i++) {
    while (!s.empty() && a[s.top()] >= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? -1 : s.top();
    s.push(i);
  }
  return res;
}

template<typename T>
std::vector<int> next_less(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = n - 1; i >= 0; i--) {
    while (!s.empty() && a[s.top()] >= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? n : s.top();
    s.push(i);
  }
  return res;
}

template<typename T>
std::vector<int> prev_greater(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = 0; i < n; i++) {
    while (!s.empty() && a[s.top()] <= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? -1 : s.top();
    s.push(i);
  }
  return res;
}

template<typename T>
std::vector<int> next_greater(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> res(n);
  std::stack<int> s;
  for (int i = n - 1; i >= 0; i--) {
    while (!s.empty() && a[s.top()] <= a[i]) {
      s.pop();
    }
    res[i] = s.empty() ? n : s.top();
    s.push(i);
  }
  return res;
}

template<typename T>
std::tuple<T, int, int> largest_histogram_rectangle(const std::vector<T> &heights) {
  int n = static_cast<int>(heights.size());
  std::vector<int> left = prev_less(heights), right = next_less(heights);
  T best{};
  int lo = 0, hi = -1;
  for (int i = 0; i < n; i++) {
    T area = heights[i] * (right[i] - left[i] - 1);  // Overflow warning.
    if (area > best) {
      best = area;
      lo = left[i] + 1;
      hi = right[i] - 1;
    }
  }
  return {best, lo, hi};
}

std::tuple<int, int, int, int, int> largest_zero_submatrix(
    const std::vector<std::vector<char>> &a
) {
  if (a.empty()) {
    return {0, 0, 0, -1, -1};
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  int best = 0, rlo = 0, clo = 0, rhi = -1, chi = -1;
  std::vector<int> height(cols);
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      height[j] = a[i][j] ? 0 : height[j] + 1;
    }
    auto [area, lo, hi] = largest_histogram_rectangle(height);
    if (area > best) {
      best = area;
      rlo = i - area / (hi - lo + 1) + 1;
      clo = lo;
      rhi = i;
      chi = hi;
    }
  }
  return {best, rlo, clo, rhi, chi};
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{2, 1, 4, 3};
  assert((prev_less(a) == vector<int>{-1, -1, 1, 1}));
  assert((next_less(a) == vector<int>{1, 4, 3, 4}));
  assert((prev_greater(a) == vector<int>{-1, 0, -1, 2}));
  assert((next_greater(a) == vector<int>{2, 2, 4, 4}));

  vector<int> hist{2, 1, 5, 6, 2, 3};
  // The best histogram rectangle uses bars 2 and 3: min height 5 times width 2.
  assert((largest_histogram_rectangle(hist) == make_tuple(10, 2, 3)));
  assert((largest_histogram_rectangle(vector<int>{}) == make_tuple(0, 0, -1)));

  vector<vector<char>> grid{
      {1, 0, 1, 1, 0, 0},
      {1, 0, 0, 1, 0, 0},
      {0, 0, 0, 0, 0, 1},
      {1, 0, 0, 1, 0, 0},
      {1, 0, 1, 0, 0, 1}
  };
  // The largest zero rectangle has 3 rows and 2 columns.
  assert((largest_zero_submatrix(grid) == make_tuple(6, 1, 1, 3, 2)));
  assert((largest_zero_submatrix({}) == make_tuple(0, 0, 0, -1, -1)));
  return 0;
}
