/*

Precomputes prefix sums so range-sum queries can be answered in constant time. This is one of the
most common array preprocessing tools, and also appears as a building block for subarray sums,
difference arrays, and two-dimensional grids.

- `prefix_sums(a)` returns array `pref` with `pref[0] = 0` and `pref[i + 1] = a[0] + ... + a[i]`.
- `range_sum(pref, lo, hi)` returns the sum of the half-open range `[l, r)`.
- `prefix_sums_2d(a)` returns a two-dimensional prefix sum table for matrix `a`.
- `rectangle_sum(pref, r1, c1, r2, c2)` returns the sum of rows `[r1, r2)` and columns `[c1, c2)`.

Time Complexity:
- O(n) per call to `prefix_sums(a)`, where $n$ is the array size.
- O(RC) per call to `prefix_sums_2d(a)`, where $R$ and $C$ are matrix dimensions.
- O(1) per range or rectangle query.

Space Complexity:
- O(n) for one-dimensional prefix sums.
- O(RC) for two-dimensional prefix sums.

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
  return pref[hi] - pref[lo];
}

std::vector<std::vector<long long>> prefix_sums_2d(const std::vector<std::vector<int>> &a) {
  int rows = a.size(), cols = rows == 0 ? 0 : a[0].size();
  std::vector<std::vector<long long>> pref(rows + 1, std::vector<long long>(cols + 1, 0));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      pref[r + 1][c + 1] = a[r][c] + pref[r][c + 1] + pref[r + 1][c] - pref[r][c];
    }
  }
  return pref;
}

long long rectangle_sum(
    const std::vector<std::vector<long long>> &pref, int r1, int c1, int r2, int c2
) {
  return pref[r2][c2] - pref[r1][c2] - pref[r2][c1] + pref[r1][c1];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int raw[] = {3, -1, 4, 1, 5};
  vector<int> a(raw, raw + 5);
  vector<long long> pref = prefix_sums(a);
  assert(range_sum(pref, 0, 5) == 12);
  assert(range_sum(pref, 1, 4) == 4);

  vector<vector<int>> grid(2, vector<int>(3));
  grid[0][0] = 1;
  grid[0][1] = 2;
  grid[0][2] = 3;
  grid[1][0] = 4;
  grid[1][1] = 5;
  grid[1][2] = 6;
  vector<vector<long long>> ps = prefix_sums_2d(grid);
  assert(rectangle_sum(ps, 0, 1, 2, 3) == 16);
  return 0;
}
