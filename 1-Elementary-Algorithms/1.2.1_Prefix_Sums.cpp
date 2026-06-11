/*

Precomputes prefix sums so range-sum queries can be answered in constant time. This is one of the
most common array preprocessing tools, and also appears as a building block for subarray sums,
difference arrays, and two-dimensional grids.

- `prefix_sums(a)` returns array `pref` with `pref[0] = 0` and `pref[i + 1] = a[0] + ... + a[i]`.
- `range_sum(pref, lo, hi)` returns the sum of the inclusive range `[lo, hi]`.
- `prefix_sums_2d(a)` returns a two-dimensional prefix sum table for matrix `a`.
- `rectangle_sum(pref, r1, c1, r2, c2)` returns the sum of the rectangle with top left at `(r1, c1)`
  and bottom right at `(r2, c2)`.

Time Complexity:
- O(n) per call to `prefix_sums(a)`, where $n$ is the array size.
- O(m*n) per call to `prefix_sums_2d(a)`, where $m$ and $n$ are the number of rows and columns of
  $a$, respectively.
- O(1) per range or rectangle query.

Space Complexity:
- O(n) for `prefix_sums(a)`.
- O(m*n) for `prefix_sums_2d(a)`.
- O(1) auxiliary for `range_sum()` and `rectangle_sum()`.

*/

#include <vector>

std::vector<long long> prefix_sums(const std::vector<int> &a) {
  std::vector<long long> pref(a.size() + 1, 0);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    pref[i + 1] = pref[i] + a[i];
  }
  return pref;
}

long long range_sum(const std::vector<long long> &pref, int lo, int hi) {
  return pref[hi + 1] - pref[lo];
}

std::vector<std::vector<long long>> prefix_sums_2d(const std::vector<std::vector<int>> &a) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  std::vector<std::vector<long long>> pref(rows + 1, std::vector<long long>(cols + 1, 0));
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      pref[i + 1][j + 1] = a[i][j] + pref[i][j + 1] + pref[i + 1][j] - pref[i][j];
    }
  }
  return pref;
}

long long rectangle_sum(
    const std::vector<std::vector<long long>> &pref, int r1, int c1, int r2, int c2
) {
  return pref[r2 + 1][c2 + 1] - pref[r1][c2 + 1] - pref[r2 + 1][c1] + pref[r1][c1];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{3, -1, 4, 1, 5};
  auto pref = prefix_sums(a);
  assert(range_sum(pref, 0, 4) == 12);  // Whole array a[0..4]
  assert(range_sum(pref, 1, 3) == 4);   // -1 + 4 + 1

  // clang-format off
  vector<vector<int>> grid{
    {1, 2, 3},
    {4, 5, 6}
  };
  // clang-format on
  auto pre2 = prefix_sums_2d(grid);
  assert(rectangle_sum(pre2, 0, 1, 1, 2) == 16);  // rows 0..1, cols 1..2
  return 0;
}
