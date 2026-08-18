/*

Transforms a rectangular grid by transposing, rotating, or reflecting its cells. Transposition
exchanges rows and columns. Rotation is clockwise by default, while a negative angle rotates
counter-clockwise; the angle must be a multiple of $90$ degrees. Together, rotations and reflections
generate all eight symmetries of a square grid.

- `transpose(a)` returns the transpose of grid `a`, so each output cell $(c, r)$ equals input cell
  $(r, c)$.
- `transpose_in_place(a)` transposes the square grid `a` in place and returns a reference to it.
- `rotate(a, degrees = 90)` returns `a` rotated clockwise by `degrees`.
- `rotate_in_place(a, degrees = 90)` rotates `a` in place and returns a reference to it. The grid
  must be square for rotations by $90$ or $270$ degrees.
- `reflect_horizontal(a)` returns `a` reflected across its horizontal axis, reversing the row order.
- `reflect_vertical(a)` returns `a` reflected across its vertical axis, reversing every row.

Time Complexity:
- O(R*C) per call, where $R$ and $C$ are the grid dimensions.

Space Complexity:
- O(R*C) for the returned grid from the non-in-place operations.
- O(1) auxiliary for the in-place operations.

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

using Grid = std::vector<std::vector<int>>;

Grid transpose(const Grid &a) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  Grid res(cols, std::vector<int>(rows));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      res[c][r] = a[r][c];
    }
  }
  return res;
}

Grid &transpose_in_place(Grid &a) {
  assert(a.empty() || a.size() == a[0].size());
  for (int r = 0; r < static_cast<int>(a.size()); r++) {
    for (int c = r + 1; c < static_cast<int>(a.size()); c++) {
      std::swap(a[r][c], a[c][r]);
    }
  }
  return a;
}

Grid rotate(const Grid &a, int degrees = 90) {
  assert(degrees % 90 == 0);
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  degrees = (degrees % 360 + 360) % 360;
  Grid res(degrees % 180 == 0 ? rows : cols, std::vector<int>(degrees % 180 == 0 ? cols : rows));
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      if (degrees == 90) {
        res[c][rows - r - 1] = a[r][c];
      } else if (degrees == 180) {
        res[rows - r - 1][cols - c - 1] = a[r][c];
      } else if (degrees == 270) {
        res[cols - c - 1][r] = a[r][c];
      } else {
        res[r][c] = a[r][c];
      }
    }
  }
  return res;
}

Grid &rotate_in_place(Grid &a, int degrees = 90) {
  assert(degrees % 90 == 0);
  degrees = (degrees % 360 + 360) % 360;
  assert(degrees % 180 == 0 || a.empty() || a.size() == a[0].size());
  if (degrees == 90) {
    transpose_in_place(a);
    for (auto &row : a) {
      std::reverse(row.begin(), row.end());
    }
  } else if (degrees == 180) {
    std::reverse(a.begin(), a.end());
    for (auto &row : a) {
      std::reverse(row.begin(), row.end());
    }
  } else if (degrees == 270) {
    transpose_in_place(a);
    std::reverse(a.begin(), a.end());
  }
  return a;
}

Grid reflect_horizontal(Grid a) {
  std::reverse(a.begin(), a.end());
  return a;
}

Grid reflect_vertical(Grid a) {
  for (auto &row : a) {
    std::reverse(row.begin(), row.end());
  }
  return a;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  Grid a{{1, 2, 3}, {4, 5, 6}};
  assert((transpose(a) == Grid{{1, 4}, {2, 5}, {3, 6}}));
  assert((rotate(a) == Grid{{4, 1}, {5, 2}, {6, 3}}));
  assert((rotate(a, -90) == Grid{{3, 6}, {2, 5}, {1, 4}}));
  assert((rotate(a, 180) == Grid{{6, 5, 4}, {3, 2, 1}}));
  assert((reflect_horizontal(a) == Grid{{4, 5, 6}, {1, 2, 3}}));
  assert((reflect_vertical(a) == Grid{{3, 2, 1}, {6, 5, 4}}));

  Grid square{{1, 2, 3}, {4, 5, 6}, {7, 8, 9}};
  for (int degrees = -360; degrees <= 360; degrees += 90) {
    Grid rotated = square;
    assert(rotate_in_place(rotated, degrees) == rotate(square, degrees));
  }
  Grid transposed = square;
  assert(transpose_in_place(transposed) == transpose(square));

  Grid rectangular = a;
  assert(rotate_in_place(rectangular, 180) == rotate(a, 180));
  return 0;
}
