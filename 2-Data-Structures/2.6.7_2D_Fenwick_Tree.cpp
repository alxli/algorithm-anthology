/*

Maintain a 2D numerical array while supporting point increments and rectangle-sum queries. This is
the two-dimensional form of the standard Fenwick tree: each internal entry stores a rectangular
block sum, and prefix queries combine O(log R * log C) disjoint blocks.

This implementation uses 1-based indices, so rows are from 1 to `R`, inclusive, and columns are
from 1 to `C`, inclusive.

Choose this for dense grids with additive point updates and rectangle-sum queries; it is simpler and
lighter than a 2D segment tree when sums are the only aggregate needed. For huge sparse grids, use
the sparse 2D Fenwick tree; for non-additive aggregates such as min/max with custom updates, use a
2D segment tree or quadtree.

- `initialize(R, C)` resets the data structure.
- `vals[r][c]` stores the value at index `(r, c)`.
- `add(r, c, x)` adds `x` to the value at index `(r, c)`.
- `set(r, c, x)` assigns `x` to the value at index `(r, c)`.
- `sum(r, c)` returns the sum of the rectangle with upper-left corner (1, 1) and lower-right corner
  `(r, c)`.
- `sum(r1, c1, r2, c2)` returns the sum of the rectangle with upper-left corner `(r1, c1)` and
  lower-right corner `(r2, c2)`.

Time Complexity:
- O(R*C) per call to `initialize(R, C)`.
- O(log(R)*log(C)) per call to all other operations.

Space Complexity:
- O(R*C) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <vector>

std::vector<std::vector<int>> vals, tree;

void initialize(int R, int C) {
  vals.assign(R + 2, std::vector<int>(C + 2, 0));
  tree.assign(R + 2, std::vector<int>(C + 2, 0));
}

void add(int r, int c, int x) {
  vals[r][c] += x;
  for (int i = r; i < static_cast<int>(tree.size()); i += i & -i) {
    for (int j = c; j < static_cast<int>(tree[0].size()); j += j & -j) {
      tree[i][j] += x;
    }
  }
}

void set(int r, int c, int x) {
  add(r, c, x - vals[r][c]);
}

int sum(int r, int c) {
  int res = 0;
  for (int i = r; i > 0; i -= i & -i) {
    for (int j = c; j > 0; j -= j & -j) {
      res += tree[i][j];
    }
  }
  return res;
}

int sum(int r1, int c1, int r2, int c2) {
  return sum(r2, c2) + sum(r1 - 1, c1 - 1) - sum(r1 - 1, c2) - sum(r2, c1 - 1);
}

/*** Example Usage and Output:

Values:
5 6 0
3 0 0
0 0 9

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  initialize(3, 3);
  set(1, 1, 5);
  set(1, 2, 6);
  set(2, 1, 7);
  add(3, 3, 9);
  add(2, 1, -4);
  cout << "Values:" << endl;
  for (int i = 1; i <= 3; i++) {
    for (int j = 1; j <= 3; j++) {
      cout << vals[i][j] << " ";
    }
    cout << endl;
  }
  assert(sum(1, 1, 1, 2) == 11);
  assert(sum(1, 1, 2, 1) == 8);
  assert(sum(1, 1, 3, 3) == 23);
  return 0;
}
