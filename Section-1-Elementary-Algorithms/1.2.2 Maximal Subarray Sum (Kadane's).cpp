/*

Given an array of numbers (at least one of which must be positive), determine
the maximum possible sum of any subarray (contiguous subsequence). Kadane's
algorithm scans through the array, at each index computing the maximum (positive
sum) subarray ending there. Either this subarray is empty (in which case its sum
is zero) or it consists of one more element than the maximum sequence ending at
the previous position. This can be adapted to compute the maximal submatrix sum
in overall cubic time.

*/

#include <algorithm>
#include <iterator>
#include <limits>
#include <vector>

/*

Returns the maximal subarray sum for the range [lo, hi) where lo and hi are
RandomAccessIterators. Optionally, two int pointers begin_idx and end_idx may be
passed to store the boundary indices of the computed maximal sum subarray. Note
that begin_idx is inclusive while end_idx is exclusive. In particular,
(lo + begin_idx) references the first element of the maximal sum subarray and
(lo + end_idx) references the index just past the last element.

If the input range consists of only negative values, then a maximum value of the
range (rather than an empty subarray) will be returned as the maximal sum.

Time Complexity: O(n) on the distance between lo and hi.
Space Complexity: O(1) auxiliary.

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

Returns the largest sum of any rectangular submatrix for a matrix of n rows by
m columns. The matrix should be given as a 2-dimensional vector, where the outer
vector must contain n vectors each of size m. Optionally, four int pointers
begin_row, end_row, begin_col, and end_col may be passed to store the boundary
indices of the maximal sum submatrix. Note that begin_row and begin_col are
inclusive indices, while end_row and end_col are exclusive (referencing the
indices just past the last elements for their respective dimensions).

If the input matrix consists of only negative values, then a maximum value of
the matrix (rather than an empty submatrix) will be returned as the maximal sum.

Time Complexity: O(m^2 * n) for a matrix with m columns and n rows.
Space Complexity: O(n) auxiliary.

*/

template<class T>
T max_submatrix_sum(const std::vector< std::vector<T> > &matrix,
                    int *begin_row = 0, int *end_row = 0,
                    int *begin_col = 0, int *end_col = 0) {
  int n = matrix.size(), m = matrix[0].size();
  std::vector<T> sums(n);
  T sum, max_sum = std::numeric_limits<T>::min();
  for (int lcol = 0; lcol < m; lcol++) {
    std::fill(sums.begin(), sums.end(), 0);
    for (int hcol = lcol; hcol < m; hcol++) {
      for (int i = 0; i < n; i++) {
        sums[i] += matrix[i][hcol];
      }
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

/*** Example Usage and Output:

1D example - the max sum subarray is:
4 -1 2 1
2D example - the max sum submatrix is:
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
    cout << "1D example - the max sum subarray is:" << endl;
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
    cout << "2D example - The max sum submatrix is:" << endl;
    for (int i = lrow; i < hrow; i++) {
      for (int j = lcol; j < hcol; j++)
        cout << mat[i][j] << " ";
      cout << endl;
    }
  }
  return 0;
}
