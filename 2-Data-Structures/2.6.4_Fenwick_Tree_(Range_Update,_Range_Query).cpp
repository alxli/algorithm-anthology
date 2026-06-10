/*

Maintain a numerical array while supporting range increments and range-sum queries. This uses two
Fenwick trees to recover prefix sums after difference-array updates: if the difference array stores
range additions, then the prefix sum through `hi` can be written as `hi*sum(t1, hi) - sum(t2, hi)`.

- `FenwickRURQ(n)` constructs an array with 0-based indices from 0 to `n - 1`, inclusive, with
  values set to 0.
- `size()` returns the size of the array.
- `at(i)` returns the value at index `i`.
- `add(i, x)` increments the value at index `i` by `x`.
- `add(lo, hi, x)` adds `x` to the values at all indices from `lo` to `hi`, inclusive.
- `set(i, x)` assigns the value at index `i` to `x`.
- `sum(hi)` returns the sum of all values at indices from 0 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.

Time Complexity:
- O(n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()`.
- O(log n) per call to all other operations.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <vector>

template<class T>
class FenwickRURQ {
  int len;
  std::vector<T> t1, t2;

  T sum(const std::vector<T> &tree, int i) {
    T res = 0;
    for (; i != 0; i -= i & -i) {
      res += tree[i];
    }
    return res;
  }

  void add(std::vector<T> &tree, int i, const T &x) {
    for (; i <= len + 1; i += i & -i) {
      tree[i] += x;
    }
  }

 public:
  explicit FenwickRURQ(int n) : len(n), t1(n + 2), t2(n + 2) {}

  int size() const { return len; }

  void add(int lo, int hi, const T &x) {
    lo++;
    hi++;
    add(t1, lo, x);
    add(t1, hi + 1, -x);
    add(t2, lo, x * (lo - 1));
    add(t2, hi + 1, -x * hi);
  }

  void add(int i, const T &x) { return add(i, i, x); }
  void set(int i, const T &x) { add(i, x - at(i)); }

  T sum(int hi) {
    hi++;
    return hi * sum(t1, hi) - sum(t2, hi);
  }

  T sum(int lo, int hi) { return sum(hi) - sum(lo - 1); }
  T at(int i) { return sum(i, i); }
};

/*** Example Usage and Output:

Values: 15 6 7 -5 4

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<int> a{10, 1, 2, 3, 4};
  FenwickRURQ<int> t(5);
  for (int i = 0; i < t.size(); i++) {
    t.set(i, a[i]);
  }
  t.add(0, 2, 5);
  t.set(3, -5);
  cout << "Values: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t.at(i) << " ";
  }
  cout << endl;
  assert(t.sum(0, 4) == 27);
  return 0;
}
