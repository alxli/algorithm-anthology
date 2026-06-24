/*

Applies many range-add updates to an array in linear total time using a difference array. Instead of
immediately adding `delta` to every element of an inclusive range $[`lo`, `hi`]$, we can add `delta`
at `diff[lo]` and subtract it at `diff[hi + 1]`; a final prefix sum reconstructs the updated values.
This idea extends to two dimensions: a rectangle update touches just the four corner cells of a
difference grid by inclusion-exclusion, and a final two-dimensional prefix sum reconstructs the
grid.

- `DifferenceArray(n)` constructs an initially zero array of size `n`.
- `add(lo, hi, delta)` adds `delta` to every position in the inclusive range $[`lo`, `hi`]$.
- `build()` returns the final array after all range updates.
- `DifferenceArray2D(rows, cols)` constructs an initially zero grid with `rows` rows and `cols`
  columns.
- `add(r1, c1, r2, c2, delta)` adds `delta` to every cell of the rectangle with top left at
  $(`r1`, `c1`)$ and bottom right at $(`r2`, `c2`)$, inclusive.
- `build()` returns the final grid after all rectangle updates.

Time Complexity:
- O(1) per call to `add()` of either version.
- O(n) per call to `build()` of `DifferenceArray`, where $n$ is the array size.
- O(m*n) per call to `build()` of `DifferenceArray2D`, where $m$ and $n$ are the number of rows and
  columns, respectively.

Space Complexity:
- O(n) for the one-dimensional difference array.
- O(m*n) for the two-dimensional difference grid.

*/

#include <cstdint>
#include <vector>

class DifferenceArray {
  std::vector<int64_t> diff;

 public:
  explicit DifferenceArray(int n) : diff(n + 1, 0) {}

  void add(int lo, int hi, int64_t delta) {
    diff[lo] += delta;
    diff[hi + 1] -= delta;
  }

  std::vector<int64_t> build() const {
    std::vector<int64_t> res(static_cast<int>(diff.size()) - 1);
    int64_t cur = 0;
    for (int i = 0; i < static_cast<int>(res.size()); i++) {
      cur += diff[i];
      res[i] = cur;
    }
    return res;
  }
};

class DifferenceArray2D {
  std::vector<std::vector<int64_t>> diff;

 public:
  DifferenceArray2D(int rows, int cols) : diff(rows + 1, std::vector<int64_t>(cols + 1, 0)) {}

  void add(int r1, int c1, int r2, int c2, int64_t delta) {
    diff[r1][c1] += delta;
    diff[r1][c2 + 1] -= delta;
    diff[r2 + 1][c1] -= delta;
    diff[r2 + 1][c2 + 1] += delta;
  }

  std::vector<std::vector<int64_t>> build() const {
    int rows = static_cast<int>(diff.size()) - 1;
    int cols = static_cast<int>(diff[0].size()) - 1;
    std::vector<std::vector<int64_t>> res(rows, std::vector<int64_t>(cols));
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        int64_t up = i > 0 ? res[i - 1][j] : 0;
        int64_t left = j > 0 ? res[i][j - 1] : 0;
        int64_t diag = i > 0 && j > 0 ? res[i - 1][j - 1] : 0;
        res[i][j] = diff[i][j] + up + left - diag;
      }
    }
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  DifferenceArray d(5);
  d.add(0, 2, 2);   // a[0..2] += 2
  d.add(1, 4, 4);   // a[1..4] += 4
  d.add(2, 3, -1);  // a[2..3] -= 1
  vector<int64_t> a = d.build();
  vector<int64_t> expected{2, 6, 5, 3, 4};
  for (int i = 0; i < static_cast<int>(expected.size()); i++) {
    assert(a[i] == expected[i]);
  }

  DifferenceArray2D d2(3, 4);
  d2.add(0, 0, 1, 2, 1);  // Rows 0..1, cols 0..2 += 1
  d2.add(1, 1, 2, 3, 2);  // Rows 1..2, cols 1..3 += 2
  vector<vector<int64_t>> g = d2.build();
  // clang-format off
  vector<vector<int64_t>> expected2{
    {1, 1, 1, 0},
    {1, 3, 3, 2},
    {0, 2, 2, 2}
  };
  // clang-format on
  assert(g == expected2);
  return 0;
}
