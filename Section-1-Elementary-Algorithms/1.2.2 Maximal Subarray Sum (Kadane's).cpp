/*

1.2.2 - Maximal Subarray Sum (Kadane's Algorithm)

Given a sequence of numbers (with at least one positive number), find
the maximum possible sum of any contiguous subarray. Kadane's algorithm
scans through the array, computing at each index the maximum (positive
sum) subarray ending at that position. This subarray is either empty
(in which case its sum is zero) or consists of one more element than
the maximum subarray ending at the previous position.

*/

#include <algorithm> /* std::fill() */
#include <iterator>  /* std::iterator_traits */
#include <limits>    /* std::numeric_limits */
#include <vector>

/*

The following implementation takes two random access iterators as the
range of values to be considered. Optionally, two pointers to integers
may be passed to have the positions of the begin and end indices of
the maximal sum subarray stored. begin_idx will be inclusive while
end_idx will be exclusive (i.e. (lo + begin_idx) will reference the
first element of the max sum subarray and (lo + end_idx) will reference
the index just past the last element of the subarray. Note that the
following version does not allow empty subarrays to be returned, so the
the max element will simply be returned if the array is all negative.

Time Complexity: O(n) on the distance between lo and hi.
Space Complexty: O(1) auxiliary.

*/

template<class It> typename std::iterator_traits<It>::value_type
max_subarray_sum(It lo, It hi, int *begin_idx = 0, int *end_idx = 0) {
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
      end = (it - lo) + 1;
    }
  }
  if (end == -1) { //all negative, just return the max value
    for (It it = lo; it != hi; ++it) {
      if (max_sum < *it) {
        max_sum = *it;
        begin = it - lo;
        end = begin + 1;
      }
    }
  }
  if (begin_idx != 0 && end_idx != 0) {
    *begin_idx = begin;
    *end_idx = end;
  }
  return max_sum;
}

/*

Maximal Submatrix Sum

In the 2-dimensional version of the problem, the largest sum of any
rectangular submatrix must be found for a matrix n rows by m columns.
Kadane's algorithm is applied to each interval [lcol, hcol] of columns
in the matrix, for an overall cubic time solution. The input must be a
two dimensional vector, where the outer vector must contain n vectors
each with m elements. Optionally, four int pointers begin_row, end_row,
begin_col, and end_col may be passed. If so, then their dereferenced
values will be set to the boundary indices of the max sum submatrix.
Note that begin_row and begin_col are inclusive indices, while end_row
and end_col are exclusive (referring to the index just past the end).

Time Complexity: O(m^2 * n) for a matrix with m columns and n rows.
Space Complexity: O(n) auxiliary.

*/

template<class T>
T max_submatrix_sum(const std::vector< std::vector<T> > & mat,
                    int *begin_row = 0, int *end_row = 0,
                    int *begin_col = 0, int *end_col = 0) {
  int n = mat.size(), m = mat[0].size();
  std::vector<T> sums(n);
  T sum, max_sum = std::numeric_limits<T>::min();
  for (int lcol = 0; lcol < m; lcol++) {
    std::fill(sums.begin(), sums.end(), 0);
    for (int hcol = lcol; hcol < m; hcol++) {
      for (int i = 0; i < n; i++)
        sums[i] += mat[i][hcol];
      int begin, end;
      sum = max_subarray_sum(sums.begin(), sums.end(), &begin, &end);
      if (sum > max_sum) {
        max_sum = sum;
        if (begin_row != 0) {
          *begin_row = begin;
          *end_row = end;
          *begin_col = lcol;
          *end_col = hcol + 1;
        }
      }
    }
  }
  return max_sum;
}

/*** Example Usage

Sample Output:
1D example - the max sum subarray is
4 -1 2 1
2D example - the max sum submatrix is
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
    int begin, end;
    assert(max_subarray_sum(a, a + 3) == -1);
    assert(max_subarray_sum(a, a + 9, &begin, &end) == 6);
    cout << "1D example - the max sum subarray is" << endl;
    for (int i = begin; i < end; i++)
      cout << a[i] << " ";
    cout << endl;
  }
  {
    const int n = 4, m = 5;
    int a[n][m] = {{ 0, -2, -7,  0,  5},
                   { 9,  2, -6,  2, -4},
                   {-4,  1, -4,  1,  0},
                   {-1,  8,  0, -2,  3}};
    vector< vector<int> > mat(n);
    for (int i = 0; i < n; i++)
      mat[i] = vector<int>(a[i], a[i] + m);
    int lrow, hrow, lcol, hcol;
    assert(max_submatrix_sum(mat, &lrow, &hrow, &lcol, &hcol) == 15);
    cout << "2D example - The max sum submatrix is" << endl;
    for (int i = lrow; i < hrow; i++) {
      for (int j = lcol; j < hcol; j++)
        cout << mat[i][j] << " ";
      cout << endl;
    }
  }
  return 0;
}
