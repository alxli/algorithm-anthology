/*

Maintain a fixed-size array while supporting point updates and contiguous range aggregate queries.
Square root decomposition partitions the array into contiguous blocks of about $\sqrt{n}$ elements,
each caching the aggregate of its block. A range query combines the cached aggregates of the whole
blocks contained in the range with the individual elements of the partial blocks at either end,
while a point update refreshes only the single block containing the changed index.

The query operation is defined by an associative aggregate function `combine(a, b)`. The default
definition below assumes a numerical array type, supporting queries for the "min" of the target
range. Another possible query operation is "sum", in which case `combine(a, b)` should return
`a + b`.

The point update operation is defined by `apply_delta(v, d)`, which returns the new value at a
single updated index. The default definition below supports updates that "set" the chosen array
index to a new value. Another possible update operation is "increment", in which case
`apply_delta(v, d)` should return `v + d`.

- `SqrtDecomposition(n, v)` constructs an array of size `n` with indices from 0 to `n - 1`,
  inclusive, and all values initialized to `v`.
- `SqrtDecomposition(lo, hi)` constructs an array from two random-access iterators as a range
  `[lo, hi)`, initialized to the elements of the range in the same order.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `query(lo, hi)` returns the result of `combine()` applied to all indices from `lo` to `hi`,
  inclusive.
- `update(i, d)` assigns the value `v` at index `i` to `apply_delta(v, d)`.

The supported operations are identical to those of the point-update segment tree in this section.

Time Complexity:
- O(n) per call to both constructors, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(sqrt n) per call to `at()`, `update()`, and `query()`.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <algorithm>
#include <cmath>
#include <vector>

template<class T>
class SqrtDecomposition {
  static T combine(const T &a, const T &b) { return std::min(a, b); }
  static T apply_delta(const T &v, const T &d) { return d; }

  int len, blocklen;
  std::vector<T> value, block;

  void init() {
    blocklen = std::max(1, static_cast<int>(sqrt(len)));
    int nblocks = (len + blocklen - 1) / blocklen;
    for (int i = 0; i < nblocks; i++) {
      T blockval = value[i * blocklen];
      int blockhi = std::min(len, (i + 1) * blocklen);
      for (int j = i * blocklen + 1; j < blockhi; j++) {
        blockval = combine(blockval, value[j]);
      }
      block.push_back(blockval);
    }
  }

 public:
  explicit SqrtDecomposition(int n, const T &v = T()) : len(n), value(n, v) { init(); }

  template<class It>
  SqrtDecomposition(It lo, It hi) : len(hi - lo), value(lo, hi) {
    init();
  }

  int size() const { return len; }
  T at(int i) const { return query(i, i); }

  T query(int lo, int hi) const {
    T res;
    int blocklo = ceil(static_cast<double>(lo) / blocklen), blockhi = (hi + 1) / blocklen - 1;
    if (blocklo > blockhi) {
      res = value[lo];
      for (int i = lo + 1; i <= hi; i++) {
        res = combine(res, value[i]);
      }
    } else {
      res = block[blocklo];
      for (int i = blocklo + 1; i <= blockhi; i++) {
        res = combine(res, block[i]);
      }
      for (int i = lo; i < blocklo * blocklen; i++) {
        res = combine(res, value[i]);
      }
      for (int i = (blockhi + 1) * blocklen; i <= hi; i++) {
        res = combine(res, value[i]);
      }
    }
    return res;
  }

  void update(int i, const T &d) {
    value[i] = apply_delta(value[i], d);
    int b = i / blocklen;
    int blockhi = std::min(len, (b + 1) * blocklen);
    block[b] = value[b * blocklen];
    for (int i = b * blocklen + 1; i < blockhi; i++) {
      block[b] = combine(block[b], value[i]);
    }
  }
};

/*** Example Usage and Output:

Values: 6 -2 4 8 10

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<int> arr{6, -2, 1, 8, 10};
  SqrtDecomposition<int> sd(arr.begin(), arr.end());
  sd.update(2, 4);
  cout << "Values:";
  for (int i = 0; i < sd.size(); i++) {
    cout << " " << sd.at(i);
  }
  cout << endl;
  assert(sd.query(0, 3) == -2);
  return 0;
}
