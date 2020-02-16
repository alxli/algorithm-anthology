/*

Given an array of numbers (at least one of which must be positive), determine
the maximum possible sum of any contiguous subarray. Kadane's algorithm scans
through the array, at each index computing the maximum positive sum subarray
ending there. Either this subarray is empty (in which case its sum is zero) or
it consists of one more element than the maximum sequence ending at the previous
position. This can be adapted to compute the maximal submatrix sum as well.

- max_subarray_sum(lo, hi, &res_lo, &res_hi) returns the maximal subarray sum
  for the range [lo, hi), where lo and hi are random-access iterators to
  numeric types. This implementation requires operators + and < to be defined on
  the iterators' value type. Optionally, two int pointers may be passed to store
  the inclusive boundary indices [res_lo, res_hi] of the resulting subarray. By
  convention, an input range consisting of only negative values will yield a
  size 1 subarray consisting of the maximum value.
- max_submatrix_sum(matrix, &r1, &c1, &r2, &c2) returns the largest sum of any
  rectangular submatrix for a matrix of n rows by m columns. The matrix should
  be given as a 2-dimensional vector, where the outer vector must contain n
  vectors each of size m. This implementation requires operators + and < to be
  defined on the iterators' value type. Optionally, four int pointers may be
  passed to store the boundary indices of the resulting subarray, with (r1, c1)
  specifiying the top-left index and (r2, c2) specifying the bottom-right index.
  By convention, an input matrix consisting of only negative values will yield a
  size 1 submatrix consisting of the maximum value.

Time Complexity:
- O(n) per call to max_subarray_sum(), where n is the distance between lo and
  hi.
- O(n*m^2) per call to max_submatrix_sum(), where n is the number of rows and m
  is the number of columns in the matrix.

Space Complexity:
- O(1) auxiliary for max_subarray_sum().
- O(n) auxiliary heap space for max_submatrix_sum(), where n is the number of
  rows in the matrix.

*/

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <limits>
#include <vector>

template<class It>
typename std::iterator_traits<It>::value_type
max_subarray_sum(It lo, It hi, int *res_lo = NULL, int *res_hi = NULL) {
  typedef typename std::iterator_traits<It>::value_type T;
  int curr_begin = 0, begin = 0, end = -1;
  T sum = 0, max_sum = std::numeric_limits<T>::min();
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
  if (end == -1) {  // All negative. By convention, return the maximum value.
    for (It it = lo; it != hi; ++it) {
      if (max_sum < *it) {
        max_sum = *it;
        begin = it - lo;
        end = begin;
      }
    }
  }
  if (res_lo != NULL && res_hi != NULL) {
    *res_lo = begin;
    *res_hi = end;
  }
  return max_sum;
}

template<class T>
T max_submatrix_sum(const std::vector<std::vector<T> > &matrix,
    int *r1 = NULL, int *c1 = NULL, int *r2 = NULL, int *c2 = NULL) {
  int n = matrix.size(), m = matrix[0].size();
  std::vector<T> sums(n);
  T sum, max_sum = std::numeric_limits<T>::min();
  for (int clo = 0; clo < m; clo++) {
    std::fill(sums.begin(), sums.end(), 0);
    for (int chi = clo; chi < m; chi++) {
      for (int i = 0; i < n; i++) {
        sums[i] += matrix[i][chi];
      }
      int rlo, rhi;
      sum = max_subarray_sum(sums.begin(), sums.end(), &rlo, &rhi);
      if (max_sum < sum) {
        max_sum = sum;
        if (r1 != NULL && c1 != NULL && r2 != NULL && c2 != NULL) {
          *r1 = rlo;
          *c1 = clo;
          *r2 = rhi;
          *c2 = chi;
        }
      }
    }
  }
  return max_sum;
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
    int a[] = {-2, -1, -3, 4, -1, 2, 1, -5, 4};
    int lo = 0, hi = 0;
    assert(max_subarray_sum(a, a + 3) == -1);
    assert(max_subarray_sum(a, a + 9, &lo, &hi) == 6);
    cout << "Maximal sum subarray:" << endl;
    for (int i = lo; i <= hi; i++) {
      cout << a[i] << " ";
    }
    cout << endl;
  }
  {
    const int n = 4, m = 5;
    int a[n][m] = {{0, -2, -7, 0, 5},
                   {9, 2, -6, 2, -4},
                   {-4, 1, -4, 1, 0},
                   {-1, 8, 0, -2, 3}};
    vector<vector<int> > matrix(n);
    for (int i = 0; i < n; i++) {
      matrix[i] = vector<int>(a[i], a[i] + m);
    }
    int r1 = 0, c1 = 0, r2 = 0, c2 = 0;
    assert(max_submatrix_sum(matrix, &r1, &c1, &r2, &c2) == 15);
    cout << "\nMaximal sum submatrix:" << endl;
    for (int i = r1; i <= r2; i++) {
      for (int j = c1; j <= c2; j++) {
        cout << matrix[i][j] << " ";
      }
      cout << endl;
    }
  }
  return 0;
}
