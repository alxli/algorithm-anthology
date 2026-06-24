/*

Maintain a 2D numerical array over an arbitrary, possibly huge index range while supporting point
increments and rectangle-sum queries, when every coordinate that will ever be updated is known in
advance. This offline restriction is what makes the structure cheap: both coordinates are
coordinate-compressed against the update points, so no hashing is required and the storage is
exactly the set of cells each Fenwick node can ever touch.

The outer dimension is an ordinary Fenwick tree over the sorted distinct update rows. Each outer
node owns an inner Fenwick tree over only those columns that reach it along an update path, again
stored in a sorted vector. A prefix query maps its row argument to the number of update rows at or
below it, walks the outer tree, and within each visited node maps its column argument to a rank by
binary search. Compared with the hashed sparse 2D Fenwick tree, this is deterministic and has
markedly better constants, at the cost of requiring all update locations up front. Prefer the dense
2D Fenwick tree when the coordinates are small enough to allocate directly, and the sparse 2D
Fenwick tree when update locations are not known ahead of time or rectangle updates are needed.

Usage is two-phase. First declare every cell that will be updated with `reserve()`; then call
`build()` once; then issue `add()` updates and `sum()` / `at()` queries freely. Updates may only
target reserved cells, but queries may use any coordinates.

- `OfflineFenwick2D<T>()` constructs an empty structure in the declaration phase.
- `reserve(r, c)` declares that the cell $(`r`, `c`)$ may later be updated. Call before `build()`.
- `build()` finalizes the coordinate compression. Call exactly once, after all `reserve()` calls.
- `add(r, c, x)` adds `x` to the value at index $(`r`, `c`)$, which must have been reserved.
- `set(r, c, x)` assigns `x` to the value at index $(`r`, `c`)$, which must have been reserved.
- `sum(r, c)` returns the sum of the rectangle with corners $(0, 0)$ and $(`r`, `c`)$, inclusive.
- `sum(r1, c1, r2, c2)` returns the sum of the rectangle with upper-left corner $(`r1`, `c1`)$ and
  lower-right corner $(`r2`, `c2`)$, inclusive.
- `at(r, c)` returns the value at index $(`r`, `c`)$.

Time Complexity:
- O(n log n) per call to `build()`, where $n$ is the number of reserved cells.
- O(log^2 n) per call to `add()`, `set()`, `sum()`, and `at()`.

Space Complexity:
- O(n log n) for storage, where $n$ is the number of reserved cells.
- O(1) auxiliary for all operations after `build()`.

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

template<typename T>
class OfflineFenwick2D {
  std::vector<int> xs;                       // Sorted distinct update rows.
  std::vector<std::vector<int>> ys;          // ys[p]: sorted distinct columns at outer node p.
  std::vector<std::vector<T>> tree;          // tree[p]: 1-indexed inner Fenwick over ys[p].
  std::vector<std::pair<int, int>> pending;  // Reserved (row, column) cells.
  bool built = false;

  // Number of update rows at most r; the outer Fenwick prefix bound for a query at row r.
  int row_rank(int r) const {
    return static_cast<int>(std::upper_bound(xs.begin(), xs.end(), r) - xs.begin());
  }

  // 1-indexed outer position of an update row r, which is guaranteed to be present in xs.
  int row_pos(int r) const {
    return static_cast<int>(std::lower_bound(xs.begin(), xs.end(), r) - xs.begin()) + 1;
  }

  // Number of columns at most c registered at outer node p; the inner prefix bound.
  int col_rank(int p, int c) const {
    return static_cast<int>(std::upper_bound(ys[p].begin(), ys[p].end(), c) - ys[p].begin());
  }

  bool is_reserved(int r, int c) const {
    int p = row_pos(r);
    return p < static_cast<int>(tree.size()) && p > 0 && xs[p - 1] == r &&
           std::binary_search(ys[p].begin(), ys[p].end(), c);
  }

 public:
  OfflineFenwick2D() = default;

  void reserve(int r, int c) {
    assert(!built);
    pending.emplace_back(r, c);
  }

  void build() {
    assert(!built);
    built = true;
    for (const auto &u : pending) {
      xs.push_back(u.first);
    }
    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());
    int m = static_cast<int>(xs.size());
    ys.assign(m + 1, {});
    // Register each update's column at every outer node on its update path.
    for (const auto &u : pending) {
      for (int x = row_pos(u.first); x <= m; x += x & -x) {
        ys[x].push_back(u.second);
      }
    }
    tree.assign(m + 1, {});
    for (int x = 1; x <= m; x++) {
      std::sort(ys[x].begin(), ys[x].end());
      ys[x].erase(std::unique(ys[x].begin(), ys[x].end()), ys[x].end());
      tree[x].assign(ys[x].size() + 1, T());
    }
    pending.clear();
  }

  void add(int r, int c, const T &x) {
    assert(built);
    assert(is_reserved(r, c));
    for (int i = row_pos(r); i < static_cast<int>(tree.size()); i += i & -i) {
      for (int j = col_rank(i, c); j <= static_cast<int>(ys[i].size()); j += j & -j) {
        tree[i][j] += x;
      }
    }
  }

  T sum(int r, int c) const {
    T res = T();
    for (int i = row_rank(r); i > 0; i -= i & -i) {
      for (int j = col_rank(i, c); j > 0; j -= j & -j) {
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

#include <cassert>
using namespace std;

int main() {
  OfflineFenwick2D<int> t;
  // Declare every cell that will be updated, then finalize.
  t.reserve(0, 0);
  t.reserve(0, 1000000);
  t.reserve(1000000, 0);
  t.reserve(1000000, 1000000);
  t.reserve(500000, 500000);
  t.build();

  t.set(0, 0, 5);
  t.set(0, 1000000, 6);
  t.set(1000000, 0, 7);
  t.add(1000000, 1000000, 9);
  t.add(500000, 500000, 4);

  assert(t.at(1000000, 1000000) == 9);
  assert(t.sum(0, 0, 0, 1000000) == 11);               // 5 + 6.
  assert(t.sum(0, 0, 1000000, 0) == 12);               // 5 + 7.
  assert(t.sum(0, 0, 1000000, 1000000) == 31);         // 5 + 6 + 7 + 9 + 4.
  assert(t.sum(400000, 400000, 600000, 600000) == 4);  // Only the center cell.
  return 0;
}
