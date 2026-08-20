/*

Maintain a numerical array while supporting range increments and point queries. This is done by
storing the difference array in a Fenwick tree: adding `x` to $[`lo`, `hi`]$ adds `+x` at `lo` and
`-x` after `hi`, and the value at index `i` is the prefix sum of those differences through `i`.

- `FenwickRUPQ<T>(n)` constructs an array with 0-based indices $[0, `n`)$, with all values
  initialized to $0$.
- `size()` returns the size of the array.
- `add(i, x)` adds `x` to the value at index `i`.
- `add(lo, hi, x)` adds `x` to the values at all indices in $[`lo`, `hi`]$.
- `at(i)` returns the value at index `i`.

The value type `T` must represent $0$ and support addition and subtraction.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to `at()` and both `add()` functions.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <vector>

template<typename T>
class FenwickRUPQ {
  int len;
  std::vector<T> tree;

  // Adds x to the value at every index from i onward.
  void add_suffix(int i, const T &x) {
    for (i++; i <= len + 1; i += i & -i) {
      tree[i] += x;
    }
  }

 public:
  explicit FenwickRUPQ(int n) : len(n), tree(n + 2) {}

  int size() const { return len; }
  void add(int i, const T &x) { add(i, i, x); }

  void add(int lo, int hi, const T &x) {
    assert(0 <= lo && lo <= hi && hi < len);
    add_suffix(lo, x);
    add_suffix(hi + 1, -x);
  }

  T at(int i) const {
    assert(0 <= i && i < len);
    T res = 0;
    for (i++; i > 0; i -= i & -i) {
      res += tree[i];
    }
    return res;
  }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  FenwickRUPQ<int> t(5);
  t.add(0, 1, 5);
  t.add(1, 2, 5);
  t.add(2, 4, 10);
  assert(t.size() == 5);
  assert(t.at(0) == 5);
  assert(t.at(1) == 10);
  assert(t.at(2) == 15);
  assert(t.at(4) == 10);
  t.add(3, -4);
  assert(t.at(3) == 6);
  assert(t.at(4) == 10);
  return 0;
}
