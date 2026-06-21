/*

Maintain a two-dimensional numerical array while supporting point increments and rectangle-sum
queries. This is the two-dimensional form of the standard Fenwick tree: each internal entry stores a
rectangular block sum, and a prefix query combines O(log R * log C) disjoint blocks.

Choose among the three 2D Fenwick trees by coordinate range and update style. Use this dense version
when the rows and columns are small enough to allocate the full grid: it has the simplest code and
the best constants, supporting point updates and rectangle sums. Use the sparse 2D Fenwick tree
(next section) when the coordinates are too large to store densely and the updates arrive online; it
relies on hashing and is the only one of the three that also supports rectangle updates. Use the
offline 2D Fenwick tree when the coordinates are large but every updated cell is known before the
queries begin, which lets it coordinate-compress instead of hashing for markedly better constants
than the sparse version. For non-additive aggregates such as min/max with custom updates, use a 2D
segment tree or quadtree instead, since Fenwick-tree algebra relies on addition and subtraction.

- `Fenwick2D<T>(rows, cols)` constructs a `rows` by `cols` array with 0-based indices, with all
  values initialized to 0.
- `size_rows()` and `size_cols()` return the number of rows and columns.
- `add(r, c, x)` adds `x` to the value at index (`r`, `c`).
- `set(r, c, x)` assigns `x` to the value at index (`r`, `c`).
- `at(r, c)` returns the value at index (`r`, `c`).
- `sum(r, c)` returns the sum of the rectangle with corners (`0`, `0`) and (`r`, `c`), inclusive.
- `sum(r1, c1, r2, c2)` returns the sum of the rectangle with upper-left corner (`r1`, `c1`) and
  lower-right corner (`r2`, `c2`), inclusive.

Time Complexity:
- O(R*C) per call to the constructor, where $R$ and $C$ are the number of rows and columns.
- O(1) per call to `size_rows()` and `size_cols()`.
- O(log(R)*log(C)) per call to all other operations.

Space Complexity:
- O(R*C) for storage of the Fenwick tree.
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <vector>

template<typename T>
class Fenwick2D {
  int rows, cols;
  std::vector<std::vector<T>> tree;

 public:
  Fenwick2D(int rows, int cols)
      : rows(rows), cols(cols), tree(rows + 1, std::vector<T>(cols + 1)) {}

  int size_rows() const { return rows; }
  int size_cols() const { return cols; }

  void add(int r, int c, const T &x) {
    assert(0 <= r && r < rows && 0 <= c && c < cols);
    for (int i = r + 1; i <= rows; i += i & -i) {
      for (int j = c + 1; j <= cols; j += j & -j) {
        tree[i][j] += x;
      }
    }
  }

  T sum(int r, int c) const {
    assert(-1 <= r && r < rows && -1 <= c && c < cols);
    T res = 0;
    for (int i = r + 1; i > 0; i -= i & -i) {
      for (int j = c + 1; j > 0; j -= j & -j) {
        res += tree[i][j];
      }
    }
    return res;
  }

  T sum(int r1, int c1, int r2, int c2) const {
    return sum(r2, c2) - sum(r1 - 1, c2) - sum(r2, c1 - 1) + sum(r1 - 1, c1 - 1);
  }

  T at(int r, int c) const { return sum(r, c, r, c); }
  void set(int r, int c, const T &x) { add(r, c, x - at(r, c)); }
};

/*** Example Usage ***/

using namespace std;

int main() {
  Fenwick2D<int> t(3, 3);
  t.set(0, 0, 5);
  t.set(0, 1, 6);
  t.set(1, 0, 7);
  t.add(2, 2, 9);
  t.add(1, 0, -4);  // Value at (1, 0) is now 3.
  assert(t.at(1, 0) == 3);
  assert(t.sum(0, 0, 0, 1) == 11);  // 5 + 6
  assert(t.sum(0, 0, 1, 0) == 8);   // 5 + 3
  assert(t.sum(0, 0, 2, 2) == 23);  // 5 + 6 + 3 + 9
  return 0;
}
