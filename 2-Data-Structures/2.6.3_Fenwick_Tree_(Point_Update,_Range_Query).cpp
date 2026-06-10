/*

Maintain a numerical array while supporting point increments and range-sum queries. This is the
standard Fenwick tree pattern: each internal entry stores a block sum, `add(i, x)` updates all
blocks containing `i`, and `sum(hi)` combines disjoint blocks to obtain a prefix sum.

- `FenwickPURQ(n)` constructs an array with 0-based indices from 0 to `n - 1`, inclusive, with
  values set to 0.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `add(i, x)` adds `x` to the value at index `i`.
- `set(i, x)` assigns the value at index `i` to `x`.
- `sum(hi)` returns the sum of all values at indices from 0 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()` and `at()`.
- O(log n) per call to `add()`, `set()`, and both `sum()` functions.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <vector>

template<class T>
class FenwickPURQ {
  int len;
  std::vector<T> vals, tree;

 public:
  explicit FenwickPURQ(int n) : len(n), vals(n + 1), tree(n + 1) {}

  int size() const { return len; }
  T at(int i) const { return vals[i + 1]; }

  void add(int i, const T &x) {
    vals[++i] += x;
    for (; i <= len; i += i & -i) {
      tree[i] += x;
    }
  }

  void set(int i, const T &x) {
    T inc = x - vals[i + 1];
    add(i, inc);
  }

  T sum(int hi) {
    T res = 0;
    for (hi++; hi > 0; hi -= hi & -hi) {
      res += tree[hi];
    }
    return res;
  }

  T sum(int lo, int hi) { return sum(hi) - sum(lo - 1); }
};

/*** Example Usage and Output:

Values: 5 1 2 3 4

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<int> a{10, 1, 2, 3, 4};
  FenwickPURQ<int> t(5);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    t.set(i, a[i]);
  }
  t.add(0, -5);
  cout << "Values: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t.at(i) << " ";
  }
  cout << endl;
  assert(t.sum(1, 3) == 6);
  return 0;
}
