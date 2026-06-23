/*

Given an array of numbers, determine the maximum possible sum of any contiguous subarray. Kadane's
algorithm scans the array while maintaining the maximum nonnegative sum of a subarray ending at the
current position. A negative running sum is discarded because it can only reduce the sum of every
subarray that extends it. Thus, by including `a[i]`, the running sum becomes `max(0, sum + a[i])`,
and the largest running sum seen is the answer. This can be adapted to compute the maximal submatrix
sum as well.

- `max_subarray_sum(lo, hi)` returns `(sum, begin, end)` for the maximal subarray in [`lo`, `hi`),
  where `lo` and `hi` are random-access iterators to numeric types. The endpoints are inclusive.
  This implementation requires operators `+` and `<` on the iterators' value type. By convention,
  the empty subarray is allowed, so an input range containing only negative values returns sum $0$
  and endpoints $[0, -1]$.
- `max_submatrix_sum(a)` returns `(sum, r1, c1, r2, c2)` for the largest rectangular submatrix of a
  matrix `a` with $m$ rows and $n$ columns. The corners are inclusive. This implementation requires
  operators `+` and `<` on the matrix value type. By convention, the empty submatrix is allowed, so
  a matrix containing only negative values returns sum $0$ with empty row and column intervals.

Time Complexity:
- O(n) per call to `max_subarray_sum()`, where $n$ is the distance between `lo` and `hi`.
- O(m*n^2) per call to `max_submatrix_sum()`, where $m$ is the number of rows and $n$ is the number
  of columns in the matrix.

Space Complexity:
- O(1) auxiliary for `max_subarray_sum()`.
- O(m) auxiliary heap space for `max_submatrix_sum()`, where $m$ is the number of rows in the
  matrix.

*/

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <vector>

template<typename It>
auto max_subarray_sum(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  if (lo == hi) {
    return std::make_tuple(T(), 0, -1);
  }
  int curr_begin = 0, begin = 0, end = -1;
  T sum = 0, max_sum = 0;
  for (It it = lo; it != hi; ++it) {
    sum += *it;
    if (sum < 0) {
      sum = 0;
      curr_begin = (it - lo) + 1;
    } else if (max_sum < sum) {
      max_sum = sum;
      begin = curr_begin;
      end = it - lo;
    }
  }
  return std::make_tuple(max_sum, begin, end);
}

template<typename T>
std::tuple<T, int, int, int, int> max_submatrix_sum(const std::vector<std::vector<T>> &a) {
  if (a.empty() || a[0].empty()) {
    return {T(), 0, 0, -1, -1};
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  int r1 = 0, c1 = 0, r2 = -1, c2 = -1;
  T max_sum = 0;
  for (int clo = 0; clo < cols; clo++) {
    std::vector<T> sums(rows, 0);
    for (int chi = clo; chi < cols; chi++) {
      for (int i = 0; i < rows; i++) {
        sums[i] += a[i][chi];
      }
      auto [sum, rlo, rhi] = max_subarray_sum(sums.begin(), sums.end());
      if (max_sum < sum) {
        max_sum = sum;
        r1 = rlo;
        c1 = clo;
        r2 = rhi;
        c2 = chi;
      }
    }
  }
  return {max_sum, r1, c1, r2, c2};
}

/*** Example Usage and Output:

Maximal sum subarray:
4 -1 2 1

Maximal sum submatrix:
9 2
-4 1
-1 8

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  {
    vector<int> a{-2, -1, -3, 4, -1, 2, 1, -5, 4};
    auto [negative_sum, negative_lo, negative_hi] = max_subarray_sum(a.begin(), a.begin() + 3);
    assert(negative_sum == 0 && negative_lo == 0 && negative_hi == -1);
    auto [sum, lo, hi] = max_subarray_sum(a.begin(), a.end());
    assert(sum == 6);
    cout << "Maximal sum subarray:" << endl;
    for (int i = lo; i <= hi; i++) {
      cout << a[i] << " ";
    }
    cout << endl;
  }
  {
    // clang-format off
    vector<vector<int>> a{{0, -2, -7, 0, 5},
                          {9, 2, -6, 2, -4},
                          {-4, 1, -4, 1, 0},
                          {-1, 8, 0, -2, 3}};
    // clang-format on
    auto [sum, r1, c1, r2, c2] = max_submatrix_sum(a);
    assert(sum == 15);
    cout << "\nMaximal sum submatrix:" << endl;
    for (int i = r1; i <= r2; i++) {
      for (int j = c1; j <= c2; j++) {
        cout << a[i][j] << " ";
      }
      cout << endl;
    }
  }
  return 0;
}
