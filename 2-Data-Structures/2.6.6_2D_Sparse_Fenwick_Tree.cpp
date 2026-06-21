/*

Maintain a 2D numerical array over a huge index range while supporting rectangle increments and
rectangle-sum queries. This is the sparse two-dimensional analogue of range-update/range-query
Fenwick trees: four sparse trees store the inclusion-exclusion coefficients needed to recover any
prefix rectangle sum.

Choose this for huge sparse grids when the operation is additive: point or rectangle increments and
rectangle sums. It avoids allocating the dense `R*C` table of the dense 2D Fenwick tree, and unlike
the dense and offline variants it supports rectangle updates, not just point updates. Prefer it over
the offline 2D Fenwick tree only when updates arrive online; if every updated cell is known in
advance, that version compresses coordinates instead of hashing and runs with better constants. It
is less general than the sparse 2D segment tree or quadtrees because Fenwick-tree algebra relies on
addition and subtraction.

- `SparseFenwick2D<T, R, C>()` constructs a 2D array over rows 0 to $R - 1$ (inclusive) and columns
  0 to $C - 1$ (inclusive). All values are implicitly initialized to 0, as nodes are allocated
  lazily as indices are touched.
- `add(r, c, x)` adds `x` to the value at index (`r`, `c`).
- `add(r1, c1, r2, c2, x)` adds `x` to all indices in the rectangle with upper-left corner
  (`r1`, `c1`) and lower-right corner (`r2`, `c2`).
- `set(r, c, x)` assigns `x` to the value at index (`r`, `c`).
- `sum(r, c)` returns the sum of the rectangle with upper-left corner (0, 0) and lower-right corner
  (`r`, `c`).
- `sum(r1, c1, r2, c2)` returns the sum of the rectangle with upper-left corner (`r1`, `c1`) and
  lower-right corner (`r2`, `c2`).
- `at(r, c)` returns the value at index (`r`, `c`).

Time Complexity:
- O(log(R)*log(C)) per call to all member functions.

Space Complexity:
- O(n*log(R)*log(C)) for storage of the array elements, where $n$ is the number of distinct indices
  that have been accessed across all of the operations so far.
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <cstdint>
#include <unordered_map>
#include <utility>

template<typename T, int R = 1000000001, int C = 1000000001>
class SparseFenwick2D {
  std::unordered_map<int64_t, T> t1, t2, t3, t4;

  template<typename Map>
  static T get(const Map &tree, int r, int c) {
    auto it = tree.find(static_cast<int64_t>(r) * (C + 1) + c);
    return it == tree.end() ? T() : it->second;
  }

  template<typename Map>
  static void add(Map &tree, int r, int c, const T &x) {
    assert(0 <= r && r <= R && 0 <= c && c <= C);
    for (int i = r + 1; i <= R; i += i & -i) {
      for (int j = c + 1; j <= C; j += j & -j) {
        tree[static_cast<int64_t>(i) * (C + 1) + j] += x;
      }
    }
  }

  void add_helper(int r, int c, const T &x) {
    add(t1, 0, 0, x);
    add(t1, 0, c, -x);
    add(t2, 0, c, x * c);
    add(t1, r, 0, -x);
    add(t3, r, 0, x * r);
    add(t1, r, c, x);
    add(t2, r, c, -x * c);
    add(t3, r, c, -x * r);
    add(t4, r, c, x * r * c);
  }

 public:
  void add(int r1, int c1, int r2, int c2, const T &x) {
    add_helper(r2 + 1, c2 + 1, x);
    add_helper(r1, c2 + 1, -x);
    add_helper(r2 + 1, c1, -x);
    add_helper(r1, c1, x);
  }

  void add(int r, int c, const T &x) { add(r, c, r, c, x); }
  void set(int r, int c, const T &x) { add(r, c, x - at(r, c)); }

  T sum(int r, int c) {
    assert(-1 <= r && r < R && -1 <= c && c < C);
    r++;
    c++;
    T s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    for (int i = r; i > 0; i -= i & -i) {
      for (int j = c; j > 0; j -= j & -j) {
        s1 += get(t1, i, j);
        s2 += get(t2, i, j);
        s3 += get(t3, i, j);
        s4 += get(t4, i, j);
      }
    }
    return s1 * r * c + s2 * r + s3 * c + s4;
  }

  T sum(int r1, int c1, int r2, int c2) {
    return sum(r2, c2) + sum(r1 - 1, c1 - 1) - sum(r1 - 1, c2) - sum(r2, c1 - 1);
  }

  T at(int r, int c) { return sum(r, c, r, c); }
};

/*** Example Usage and Output:

Values:
5 6 0
3 5 5
0 5 14

***/

#include <iostream>
using namespace std;

int main() {
  SparseFenwick2D<int> t;
  t.set(0, 0, 5);
  t.set(0, 1, 6);
  t.set(1, 0, 7);
  t.add(2, 2, 9);
  t.add(1, 0, -4);
  t.add(1, 1, 2, 2, 5);
  cout << "Values:" << endl;
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      cout << t.at(i, j) << " ";
    }
    cout << endl;
  }
  assert(t.sum(0, 0, 0, 1) == 11);
  assert(t.sum(0, 0, 1, 0) == 8);
  assert(t.sum(1, 1, 2, 2) == 29);
  t.set(500000000, 500000000, 100);
  assert(t.sum(0, 0, 999999999, 999999999) == 143);
  return 0;
}
