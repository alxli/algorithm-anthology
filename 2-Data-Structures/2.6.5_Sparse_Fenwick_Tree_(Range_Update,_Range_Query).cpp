/*

Maintain a numerical array over a huge index range while supporting range increments and range-sum
queries. This is the sparse version of the two-tree Fenwick range-update/range-query trick: only
Fenwick nodes reached by previous operations are stored, using `std::unordered_map` instead of dense
vectors.

This implementation uses 0-based array indices, so all indices from 0 to `N`, inclusive, are
accessible.

- `SparseFenwickRURQ()` constructs an initially zero array over indices 0 to `N`.
- `at(i)` returns the value at index `i`.
- `add(i, x)` adds `x` to the value at index `i`.
- `add(lo, hi, x)` adds `x` to the values at all indices from `lo` to `hi`, inclusive.
- `set(i, x)` assigns the value at index `i` to `x`.
- `sum(hi)` returns the sum of all values at indices from 0 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.

Time Complexity:
- O(log n) per call to all member functions, where $n$ is the number of distinct indices that have
  been accessed.

Space Complexity:
- O(n log n) for storage of the array elements, where $n$ is the number of distinct indices that
  have been accessed across all of the operations so far.
- O(1) auxiliary for all operations.

*/

#include <unordered_map>

template<class T>
class SparseFenwickRURQ {
  static const int N = 1000000001;
  std::unordered_map<int, T> tmul, tadd;

  void add_helper(int at, int mul, T add) {
    for (int i = at; i <= N; i |= i + 1) {
      tmul[i] += mul;
      tadd[i] += add;
    }
  }

 public:
  void add(int lo, int hi, const T &x) {
    add_helper(lo, x, -x * (lo - 1));
    add_helper(hi, -x, x * hi);
  }

  void add(int i, const T &x) { add(i, i, x); }
  void set(int i, const T &x) { add(i, x - at(i)); }

  T sum(int hi) {
    T mul = 0, add = 0;
    for (int i = hi; i >= 0; i = (i & (i + 1)) - 1) {
      if (auto it = tmul.find(i); it != tmul.end()) {
        mul += it->second;
      }
      if (auto it = tadd.find(i); it != tadd.end()) {
        add += it->second;
      }
    }
    return mul * hi + add;
  }

  T sum(int lo, int hi) { return sum(hi) - sum(lo - 1); }
  T at(int i) { return sum(i, i); }
};

/*** Example Usage and Output:

Values: 15 6 7 -5 4

***/

#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  vector<int> a{10, 1, 2, 3, 4};
  SparseFenwickRURQ<int> t;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    t.set(i, a[i]);
  }
  t.add(0, 2, 5);
  t.set(3, -5);
  cout << "Values: ";
  for (int i = 0; i < 5; i++) {
    cout << t.at(i) << " ";
  }
  cout << endl;
  assert(t.sum(0, 4) == 27);
  t.add(500000001, 500000010, 3);
  t.add(500000011, 500000015, 5);
  t.set(500000000, 10);
  assert(t.sum(0, 1000000000) == 92);
  return 0;
}
