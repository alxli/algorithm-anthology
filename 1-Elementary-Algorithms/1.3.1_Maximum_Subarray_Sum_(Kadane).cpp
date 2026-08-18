/*

Given an array of numbers, determine the maximum possible sum of any contiguous subarray. Kadane's
algorithm scans the array while maintaining a nonnegative running sum. At each position, it adds the
current value, records the largest sum seen, and resets the running sum to zero if it becomes
negative. Discarding a negative sum is safe because it could only reduce every subarray extending
it. This can be adapted to compute the maximal submatrix sum as well.

Two related scans maintain slightly different states. For a circular array, the best wrapped
subarray is the whole-array sum minus the minimum-sum subarray. For a maximum-product subarray, a
negative value swaps the roles of the minimum and maximum products ending at the previous position,
so both products must be tracked.

- `max_subarray_sum(a)` returns the sum and inclusive endpoints (`sum`, `begin`, `end`) for the
  maximal-sum subarray of vector `a`. This implementation requires operators `+` and `<` on the
  value type. By convention, the empty subarray is allowed, so an input containing only negative
  values returns sum $0$ and endpoints $[0, -1]$.
- `max_circular_subarray_sum(a)` returns a tuple (`sum`, `begin`, `end`) containing the maximal
  circular-subarray sum and its inclusive endpoints. If `begin` $\leq$ `end`, the result is an
  ordinary range; if `begin` > `end`, it wraps from `begin` through the end of `a` and continues
  through `end`. The empty subarray is allowed under the same convention as `max_subarray_sum()`.
- `max_product_subarray(a)` returns a tuple (`product`, `begin`, `end`) containing the product and
  inclusive endpoints of a nonempty maximal-product subarray, or product $0$ and endpoints $[0, -1]$
  for an empty input.
- `max_submatrix_sum(a)` returns the sum and inclusive boundaries (`sum`, `r1`, `c1`, `r2`, `c2`)
  for the largest rectangular submatrix of a matrix `a`. This implementation requires operators `+`
  and `<` on the matrix value type. By convention, the empty submatrix is allowed, so a matrix
  containing only negative values returns sum $0$ with empty row and column intervals.

Overflow warning: All resulting sums and products must fit in the value type.

Time Complexity:
- O(n) per call to `max_subarray_sum()`, `max_circular_subarray_sum()`, and
  `max_product_subarray()`, where $n$ is the size of `a`.
- O(R*C^2) per call to `max_submatrix_sum()`, where $R$ is the number of rows and $C$ is the number
  of columns in the matrix.

Space Complexity:
- O(1) auxiliary for each subarray function.
- O(R) auxiliary for `max_submatrix_sum()`.

*/

#include <tuple>
#include <utility>
#include <vector>

template<typename T>
std::tuple<T, int, int> max_subarray_sum(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return {T{}, 0, -1};
  }
  int curr_begin = 0, begin = 0, end = -1;
  T sum = 0, max_sum = 0;
  for (int i = 0; i < n; i++) {
    sum += a[i];  // Overflow warning.
    if (sum < 0) {
      sum = 0;
      curr_begin = i + 1;
    } else if (max_sum < sum) {
      max_sum = sum;
      begin = curr_begin;
      end = i;
    }
  }
  return {max_sum, begin, end};
}

template<typename T>
std::tuple<T, int, int> max_circular_subarray_sum(const std::vector<T> &a) {
  auto [max_sum, begin, end] = max_subarray_sum(a);
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return {max_sum, begin, end};
  }
  int curr_begin = 0, min_begin = 0, min_end = -1;
  T total = 0, min_sum = 0, curr_sum = 0;
  for (int i = 0; i < n; i++) {
    total += a[i];     // Overflow warning.
    curr_sum += a[i];  // Overflow warning.
    if (0 < curr_sum) {
      curr_sum = 0;
      curr_begin = i + 1;
    } else if (curr_sum < min_sum) {
      min_sum = curr_sum;
      min_begin = curr_begin;
      min_end = i;
    }
  }
  T wrapped_sum = total - min_sum;
  if (max_sum < wrapped_sum) {
    max_sum = wrapped_sum;
    begin = (min_end + 1) % n;
    end = (min_begin + n - 1) % n;
  }
  return {max_sum, begin, end};
}

template<typename T>
std::tuple<T, int, int> max_product_subarray(const std::vector<T> &a) {
  int n = static_cast<int>(a.size());
  if (n == 0) {
    return {T{}, 0, -1};
  }
  T max_end = a[0], min_end = a[0], best = a[0];
  int max_begin = 0, min_begin = 0, begin = 0, end = 0;
  for (int i = 1; i < n; i++) {
    if (a[i] < 0) {
      std::swap(max_end, min_end);
      std::swap(max_begin, min_begin);
    }
    T next_max = max_end * a[i];  // Overflow warning.
    T next_min = min_end * a[i];  // Overflow warning.
    if (next_max < a[i]) {
      max_end = a[i];
      max_begin = i;
    } else {
      max_end = next_max;
    }
    if (a[i] < next_min) {
      min_end = a[i];
      min_begin = i;
    } else {
      min_end = next_min;
    }
    if (best < max_end) {
      best = max_end;
      begin = max_begin;
      end = i;
    }
  }
  return {best, begin, end};
}

template<typename T>
std::tuple<T, int, int, int, int> max_submatrix_sum(const std::vector<std::vector<T>> &a) {
  if (a.empty() || a[0].empty()) {
    return {T{}, 0, 0, -1, -1};
  }
  int rows = static_cast<int>(a.size()), cols = static_cast<int>(a[0].size());
  int r1 = 0, c1 = 0, r2 = -1, c2 = -1;
  T max_sum = 0;
  for (int clo = 0; clo < cols; clo++) {
    std::vector<T> sums(rows);
    for (int chi = clo; chi < cols; chi++) {
      for (int i = 0; i < rows; i++) {
        sums[i] += a[i][chi];  // Overflow warning.
      }
      auto [sum, rlo, rhi] = max_subarray_sum(sums);
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
    // All negative values, so the empty subarray is maximal.
    auto [sum, begin, end] = max_subarray_sum(vector<int>{-2, -1, -3});
    assert(sum == 0 && begin == 0 && end == -1);
  }
  {
    vector<int> a{-2, -1, -3, 4, -1, 2, 1, -5, 4};
    auto [sum, begin, end] = max_subarray_sum(a);
    assert(sum == 6);
    cout << "Maximal sum subarray:" << endl;
    for (int i = begin; i <= end; i++) {
      cout << a[i] << " ";
    }
    cout << endl;
  }
  {
    vector<int> a{5, -3, 5};
    auto [sum, begin, end] = max_circular_subarray_sum(a);
    assert(sum == 10 && begin == 2 && end == 0);
  }
  {
    vector<int> a{2, 3, -2, 4};
    auto [product, begin, end] = max_product_subarray(a);
    assert(product == 6 && begin == 0 && end == 1);
  }
  {
    vector<vector<int>> a{
        {0, -2, -7, 0, 5},
        {9, 2, -6, 2, -4},
        {-4, 1, -4, 1, 0},
        {-1, 8, 0, -2, 3},
    };
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
