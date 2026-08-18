/*

Applies many range-add updates to an array in linear total time using a difference array. Instead of
immediately adding `delta` to every element of range $[`lo`, `hi`]$, we can add `delta` at
`diff[lo]` and subtract it at `diff[hi + 1]`; a final prefix sum reconstructs the updated values.
This idea extends to two dimensions: a rectangle update touches just the four corner cells of a
difference grid by inclusion-exclusion, and a final two-dimensional prefix sum reconstructs the
grid.

- `DifferenceArray(n)` constructs an initially zero array of size `n`.
- `add(lo, hi, delta)` adds `delta` to every position in range $[`lo`, `hi`]$.
- `build()` returns the final array after all range updates.
- `DifferenceArray2D(rows, cols)` constructs an initially zero grid with `rows` rows and `cols`
  columns.
- `add(r1, c1, r2, c2, delta)` adds `delta` to every cell of the rectangle with rows $[`r1`, `r2`]$
  and columns $[`c1`, `c2`]$.
- `build()` returns the final grid after all rectangle updates.

Overflow warning: All stored differences and reconstructed values must fit in `int64_t`.

Time Complexity:
- O(1) per call to `add()` of either version.
- O(n) per call to `build()` of `DifferenceArray`, where $n$ is the array size.
- O(R*C) per call to `build()` of `DifferenceArray2D`, where $R$ and $C$ are the number of rows and
  columns, respectively.

Space Complexity:
- O(n) for storage and O(n) for the array returned by `DifferenceArray::build()`.
- O(R*C) for storage and O(R*C) for the grid returned by `DifferenceArray2D::build()`.

*/

#include <cassert>
#include <cstdint>
#include <vector>

class DifferenceArray {
  std::vector<int64_t> diff;

 public:
  explicit DifferenceArray(int n) {
    assert(n >= 0);
    diff.assign(n + 1, 0);
  }

  void add(int lo, int hi, int64_t delta) {
    assert(0 <= lo && lo <= hi && hi + 1 < static_cast<int>(diff.size()));
    diff[lo] += delta;  // Overflow warning.
    diff[hi + 1] -= delta;
  }

  std::vector<int64_t> build() const {
    std::vector<int64_t> res(static_cast<int>(diff.size()) - 1);
    int64_t cur = 0;
    for (int i = 0; i < static_cast<int>(res.size()); i++) {
      cur += diff[i];  // Overflow warning.
      res[i] = cur;
    }
    return res;
  }
};

class DifferenceArray2D {
  std::vector<std::vector<int64_t>> diff;

 public:
  DifferenceArray2D(int rows, int cols) {
    assert(rows >= 0 && cols >= 0);
    diff.assign(rows + 1, std::vector<int64_t>(cols + 1));
  }

  void add(int r1, int c1, int r2, int c2, int64_t delta) {
    int rows = static_cast<int>(diff.size()) - 1;
    int cols = static_cast<int>(diff[0].size()) - 1;
    assert(0 <= r1 && r1 <= r2 && r2 < rows);
    assert(0 <= c1 && c1 <= c2 && c2 < cols);
    diff[r1][c1] += delta;  // Overflow warning.
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
        res[i][j] = diff[i][j] + up + left - diag;  // Overflow warning.
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
  d.add(0, 2, 2);   // Add 2 to indices 0-2.
  d.add(1, 4, 4);   // Add 4 to indices 1-4.
  d.add(2, 3, -1);  // Subtract 1 from indices 2-3.
  d.add(4, 4, 3);   // Single index update.
  assert((d.build() == vector<int64_t>{2, 6, 5, 3, 7}));

  DifferenceArray2D d2(3, 4);
  d2.add(0, 0, 1, 2, 1);  // Add 1 to rows 0-1 and columns 0-2.
  d2.add(1, 1, 2, 3, 2);  // Add 2 to rows 1-2 and columns 1-3.
  d2.add(2, 0, 2, 0, 5);  // Single cell update.
  vector<vector<int64_t>> expected2{
      {1, 1, 1, 0},
      {1, 3, 3, 2},
      {5, 2, 2, 2},
  };
  assert(d2.build() == expected2);
  return 0;
}
