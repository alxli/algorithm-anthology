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

- `max_subarray_sum(lo, hi)` returns the sum and inclusive endpoints (`sum`, `begin`, `end`) for the
  maximal-sum subarray of $[`lo`, `hi`)$, where `lo` and `hi` are random-access iterators to numeric
  types. This implementation requires operators `+` and `<` on the iterators' value type. By
  convention, the empty subarray is allowed, so an input range containing only negative values
  returns sum $0$ and endpoints $[0, -1]$.
- `max_circular_subarray_sum(lo, hi)` returns a tuple (`sum`, `begin`, `end`) containing the maximal
  circular-subarray sum and its inclusive endpoints. If `begin` $\leq$ `end`, the result is an
  ordinary range; if `begin` > `end`, it wraps from `begin` through the end of the input and
  continues through `end`. The empty subarray is allowed under the same convention as
  `max_subarray_sum()`.
- `max_product_subarray(lo, hi)` returns a tuple (`product`, `begin`, `end`) containing the product
  and inclusive endpoints of a nonempty maximal-product subarray, or product $0$ and endpoints
  $[0, -1]$ for an empty input range.
- `max_submatrix_sum(a)` returns the sum and inclusive boundaries (`sum`, `r1`, `c1`, `r2`, `c2`)
  for the largest rectangular submatrix of a matrix `a`. This implementation requires operators `+`
  and `<` on the matrix value type. By convention, the empty submatrix is allowed, so a matrix
  containing only negative values returns sum $0$ with empty row and column intervals.

Overflow warning: All resulting sums and products must fit in the value type.

Time Complexity:
- O(n) per call to `max_subarray_sum()`, `max_circular_subarray_sum()`, and
  `max_product_subarray()`, where $n$ is the distance between `lo` and `hi`.
- O(m*n^2) per call to `max_submatrix_sum()`, where $m$ is the number of rows and $n$ is the number
  of columns in the matrix.

Space Complexity:
- O(1) auxiliary for each subarray function.
- O(m) auxiliary for `max_submatrix_sum()`, where $m$ is the number of rows in the matrix.

*/

#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

template<typename It>
auto max_subarray_sum(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  if (lo == hi) {
    return std::tuple{T{}, 0, -1};
  }
  int curr_begin = 0, begin = 0, end = -1;
  T sum = 0, max_sum = 0;
  for (It it = lo; it != hi; ++it) {
    sum += *it;  // Overflow warning.
    if (sum < 0) {
      sum = 0;
      curr_begin = (it - lo) + 1;
    } else if (max_sum < sum) {
      max_sum = sum;
      begin = curr_begin;
      end = it - lo;
    }
  }
  return std::tuple{max_sum, begin, end};
}

template<typename It>
auto max_circular_subarray_sum(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  auto [max_sum, begin, end] = max_subarray_sum(lo, hi);
  if (lo == hi) {
    return std::tuple{max_sum, begin, end};
  }
  int curr_begin = 0, min_begin = 0, min_end = -1;
  T total = 0, min_sum = 0, curr_sum = 0;
  for (It it = lo; it != hi; ++it) {
    total += *it;     // Overflow warning.
    curr_sum += *it;  // Overflow warning.
    if (0 < curr_sum) {
      curr_sum = 0;
      curr_begin = (it - lo) + 1;
    } else if (curr_sum < min_sum) {
      min_sum = curr_sum;
      min_begin = curr_begin;
      min_end = it - lo;
    }
  }
  T wrapped_sum = total - min_sum;
  if (max_sum < wrapped_sum) {
    max_sum = wrapped_sum;
    int n = static_cast<int>(hi - lo);
    begin = (min_end + 1) % n;
    end = (min_begin + n - 1) % n;
  }
  return std::tuple{max_sum, begin, end};
}

template<typename It>
auto max_product_subarray(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  if (lo == hi) {
    return std::tuple{T{}, 0, -1};
  }
  T max_ending = *lo, min_ending = *lo, best = *lo;
  int max_begin = 0, min_begin = 0, begin = 0, end = 0;
  for (It it = lo + 1; it != hi; ++it) {
    int i = it - lo;
    if (*it < 0) {
      std::swap(max_ending, min_ending);
      std::swap(max_begin, min_begin);
    }
    T next_max = max_ending * *it;  // Overflow warning.
    T next_min = min_ending * *it;  // Overflow warning.
    if (next_max < *it) {
      max_ending = *it;
      max_begin = i;
    } else {
      max_ending = next_max;
    }
    if (*it < next_min) {
      min_ending = *it;
      min_begin = i;
    } else {
      min_ending = next_min;
    }
    if (best < max_ending) {
      best = max_ending;
      begin = max_begin;
      end = i;
    }
  }
  return std::tuple{best, begin, end};
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
    auto [neg_sum, neg_begin, neg_end] = max_subarray_sum(a.begin(), a.begin() + 3);
    assert(neg_sum == 0 && neg_begin == 0 && neg_end == -1);
    auto [sum, begin, end] = max_subarray_sum(a.begin(), a.end());
    assert(sum == 6);
    cout << "Maximal sum subarray:" << endl;
    for (int i = begin; i <= end; i++) {
      cout << a[i] << " ";
    }
    cout << endl;
  }
  {
    vector<int> a{5, -3, 5};
    auto [sum, begin, end] = max_circular_subarray_sum(a.begin(), a.end());
    assert(sum == 10 && begin == 2 && end == 0);
  }
  {
    vector<int> a{2, 3, -2, 4};
    auto [product, begin, end] = max_product_subarray(a.begin(), a.end());
    assert(product == 6 && begin == 0 && end == 1);
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
