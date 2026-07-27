/*

Maintain a dense two-dimensional array while supporting point assignments and rectangle queries. A
2D segment tree applies the ordinary segment-tree decomposition independently to rows and columns:
each row-tree node contains a complete segment tree over its columns, so a rectangle decomposes into
O(log(R)*log(C)) canonical blocks.

The query operation is defined by a commutative associative `combine(a, b)`. The default below
computes the minimum value in a rectangle. For rectangle maximum queries, use `std::max`; for
rectangle sums, use addition.

Point updates are defined by `apply_delta(v, d)`, which applies delta `d` to the current value `v`.
The default below assigns `d`; for point increments, return `v + d` instead.

Use this when the grid is small enough for O(R*C) storage and queries need an aggregate other than
addition. For additive rectangle sums, a 2D Fenwick tree is simpler and has smaller constants. Use
the sparse 2D segment tree in the next section when the coordinate range is too large to allocate
dense storage.

- `SegTree2D<T>(rows, cols, value = T())` constructs a `rows` by `cols` array with all entries
  initialized to `value`.
- `SegTree2D<T>(a)` constructs the tree from the matrix `a`.
- `num_rows()` and `num_cols()` return the dimensions of the array.
- `update(r, c, d)` applies delta `d` to the entry at index (`r`, `c`).
- `at(r, c)` returns the value at index (`r`, `c`).
- `query(r1, c1, r2, c2)` returns the aggregate over the rectangle with rows in $[`r1`, `r2`]$ and
  columns in $[`c1`, `c2`]$.

Time Complexity:
- O(R*C) per call to either constructor, where $R$ and $C$ are the matrix dimensions.
- O(log(R)*log(C)) per call to `update()` and `query()`.
- O(1) per call to `num_rows()`, `num_cols()`, and `at()`.

Space Complexity:
- O(R*C) for storage of the segment tree.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cassert>
#include <optional>
#include <vector>

template<typename T>
class SegTree2D {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d) { return d; }

  int rows, cols;
  std::vector<std::vector<T>> tree;

  template<typename Gen>
  void build(const Gen &gen) {
    tree.assign(2 * rows, std::vector<T>(2 * cols));
    for (int r = 0; r < rows; r++) {
      for (int c = 0; c < cols; c++) {
        tree[r + rows][c + cols] = gen(r, c);
      }
    }
    for (int r = rows; r < 2 * rows; r++) {
      for (int c = cols - 1; c > 0; c--) {
        tree[r][c] = combine(tree[r][2 * c], tree[r][2 * c + 1]);
      }
    }
    for (int r = rows - 1; r > 0; r--) {
      for (int c = 1; c < 2 * cols; c++) {
        tree[r][c] = combine(tree[2 * r][c], tree[2 * r + 1][c]);
      }
    }
  }

  T query_columns(int r, int c1, int c2) const {
    std::optional<T> res;
    for (c1 += cols, c2 += cols + 1; c1 < c2; c1 /= 2, c2 /= 2) {
      if ((c1 & 1) != 0) {
        res = res ? combine(*res, tree[r][c1++]) : tree[r][c1++];
      }
      if ((c2 & 1) != 0) {
        res = res ? combine(*res, tree[r][--c2]) : tree[r][--c2];
      }
    }
    return *res;
  }

 public:
  SegTree2D(int rows, int cols, const T &value = T()) : rows(rows), cols(cols) {
    assert(rows > 0 && cols > 0);
    build([&](int, int) { return value; });
  }

  explicit SegTree2D(const std::vector<std::vector<T>> &a)
      : rows(static_cast<int>(a.size())), cols(rows == 0 ? 0 : static_cast<int>(a[0].size())) {
    assert(rows > 0 && cols > 0);
    build([&](int r, int c) { return a[r][c]; });
  }

  int num_rows() const { return rows; }
  int num_cols() const { return cols; }

  T at(int r, int c) const {
    assert(0 <= r && r < rows && 0 <= c && c < cols);
    return tree[r + rows][c + cols];
  }

  void update(int r, int c, const T &d) {
    assert(0 <= r && r < rows && 0 <= c && c < cols);
    int row = r + rows, col = c + cols;
    tree[row][col] = apply_delta(tree[row][col], d);
    for (int j = col; j > 1; j /= 2) {
      tree[row][j / 2] = combine(tree[row][j], tree[row][j ^ 1]);
    }
    for (row /= 2; row > 0; row /= 2) {
      tree[row][col] = combine(tree[2 * row][col], tree[2 * row + 1][col]);
      for (int j = col; j > 1; j /= 2) {
        tree[row][j / 2] = combine(tree[row][j], tree[row][j ^ 1]);
      }
    }
  }

  T query(int r1, int c1, int r2, int c2) const {
    assert(0 <= r1 && r1 <= r2 && r2 < rows);
    assert(0 <= c1 && c1 <= c2 && c2 < cols);
    std::optional<T> res;
    for (r1 += rows, r2 += rows + 1; r1 < r2; r1 /= 2, r2 /= 2) {
      if ((r1 & 1) != 0) {
        T value = query_columns(r1++, c1, c2);
        res = res ? combine(*res, value) : value;
      }
      if ((r2 & 1) != 0) {
        T value = query_columns(--r2, c1, c2);
        res = res ? combine(*res, value) : value;
      }
    }
    return *res;
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  SegTree2D<int> tree(3, 4, 100);
  tree.update(0, 1, 8);
  tree.update(1, 2, 3);
  tree.update(2, 0, 5);
  assert(tree.num_rows() == 3 && tree.num_cols() == 4);
  assert(tree.at(0, 1) == 8);
  assert(tree.query(0, 0, 1, 3) == 3);
  assert(tree.query(2, 0, 2, 3) == 5);

  SegTree2D<int> from_matrix(vector<vector<int>>{{7, 2, 6}, {4, 9, 1}});
  assert(from_matrix.query(0, 0, 1, 1) == 2);
  assert(from_matrix.query(0, 1, 1, 2) == 1);
  return 0;
}
