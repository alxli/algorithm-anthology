/*

Precomputes prefix sums so range-sum queries can be answered in constant time. This is one of the
most common array preprocessing tools, and also appears as a building block for subarray sums,
difference arrays, and two-dimensional grids. Each table entry stores the sum of all elements before
it, so any range sum is the difference of two entries; in two dimensions, rectangle sums combine
four entries by inclusion-exclusion.

Prefix sums can also be combined with a frequency table to count subarrays having a target sum. When
the current prefix sum is $s$, every earlier prefix sum equal to $s - t$ identifies a subarray with
sum $t$. Recording frequencies rather than only whether a prefix exists counts duplicate sums
correctly.

- `prefix_sums(a)` returns the prefix sum array `pref` of length $n + 1$, with `pref[0] = 0` and
  `pref[i + 1]` equal to the sum of `a[0]` through `a[i]`.
- `range_sum(pref, lo, hi)` returns the sum of range $[`lo`, `hi`]$.
- `prefix_sums_2d(a)` returns a two-dimensional prefix sum table for matrix `a`.
- `rectangle_sum(pref, r1, c1, r2, c2)` returns the sum of the rectangle with rows $[`r1`, `r2`]$
  and columns $[`c1`, `c2`]$.
- `count_subarrays_with_sum(a, target)` returns the number of contiguous subarrays whose sum is
  `target`.

Overflow warning: All prefix sums, target differences, inclusion-exclusion results, and returned
counts must fit in `int64_t`.

Time Complexity:
- O(n) per call to `prefix_sums(a)`, where $n$ is the array size.
- O(R*C) per call to `prefix_sums_2d(a)`, where $R$ and $C$ are the number of rows and columns of
  `a`, respectively.
- O(1) per range or rectangle query.
- O(n) expected per call to `count_subarrays_with_sum(a, target)`.

Space Complexity:
- O(n) for the array returned by `prefix_sums(a)`.
- O(R*C) for the table returned by `prefix_sums_2d(a)`.
- O(1) auxiliary for `range_sum()` and `rectangle_sum()`.
- O(n) auxiliary for `count_subarrays_with_sum(a, target)`.

*/

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <vector>

std::vector<int64_t> prefix_sums(const std::vector<int> &a) {
  std::vector<int64_t> pref(a.size() + 1);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    pref[i + 1] = pref[i] + a[i];
  }
  return pref;
}

int64_t range_sum(const std::vector<int64_t> &pref, int lo, int hi) {
  assert(0 <= lo && lo <= hi && hi < static_cast<int>(pref.size()) - 1);
  return pref[hi + 1] - pref[lo];
}

std::vector<std::vector<int64_t>> prefix_sums_2d(const std::vector<std::vector<int>> &a) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  std::vector<std::vector<int64_t>> pref(rows + 1, std::vector<int64_t>(cols + 1));
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      pref[i + 1][j + 1] = a[i][j] + pref[i][j + 1] + pref[i + 1][j] - pref[i][j];
    }
  }
  return pref;
}

int64_t rectangle_sum(
    const std::vector<std::vector<int64_t>> &pref, int r1, int c1, int r2, int c2
) {
  assert(
      !pref.empty() && !pref[0].empty() && 0 <= r1 && r1 <= r2 &&
      r2 < static_cast<int>(pref.size()) - 1 && 0 <= c1 && c1 <= c2 &&
      c2 < static_cast<int>(pref[0].size()) - 1
  );
  return pref[r2 + 1][c2 + 1] - pref[r1][c2 + 1] - pref[r2 + 1][c1] + pref[r1][c1];
}

int64_t count_subarrays_with_sum(const std::vector<int> &a, int64_t target) {
  std::unordered_map<int64_t, int64_t> count{{0, 1}};
  int64_t sum = 0, result = 0;
  for (int x : a) {
    sum += x;
    int64_t needed = sum - target;
    if (auto it = count.find(needed); it != count.end()) {
      result += it->second;
    }
    count[sum]++;
  }
  return result;
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> a{3, -1, 4, 1, 5};
  auto pref = prefix_sums(a);
  assert(range_sum(pref, 0, 4) == 12);  // Whole array.
  assert(range_sum(pref, 1, 3) == 4);   // -1 + 4 + 1.
  assert(range_sum(pref, 2, 2) == 4);   // Single element.
  assert(count_subarrays_with_sum(a, 4) == 2);
  assert(count_subarrays_with_sum(vector<int>{0, 0, 0}, 0) == 6);

  // clang-format off
  vector<vector<int>> grid{
    {1, 2, 3},
    {4, 5, 6}
  };
  // clang-format on
  auto pre2 = prefix_sums_2d(grid);
  assert(rectangle_sum(pre2, 0, 1, 1, 2) == 16);  // Rows 0-1 and columns 1-2.
  assert(rectangle_sum(pre2, 0, 0, 1, 2) == 21);  // Whole grid.
  assert(rectangle_sum(pre2, 1, 1, 1, 1) == 5);   // Single cell.
  return 0;
}
