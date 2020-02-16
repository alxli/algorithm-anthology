/*

Maintain an array of numerical type, allowing for updates of individual indices
(point update) and queries for the sum of contiguous sub-arrays (range queries).
This implementation assumes that the array is 0-based (i.e. has valid indices
from 0 to size() - 1, inclusive).

- size() returns the size of the array.
- at(i) returns the value at index i.
- add(i, x) adds x to the value at index i.
- set(i, x) assigns the value at index i to x.
- sum(hi) returns the sum of all values at indices from 0 to hi, inclusive.
- sum(lo, hi) returns the sum of all values at indices from lo to hi, inclusive.

Time Complexity:
- O(n) per call to the constructor, where n is the size of the array.
- O(1) per call to size() and at().
- O(log n) per call to add(), set(), and both sum() functions.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <vector>

template<class T>
class fenwick_tree {
  int len;
  std::vector<int> a, t;

 public:
  fenwick_tree(int n) : len(n), a(n + 1), t(n + 1) {}

  int size() const {
    return len;
  }

  T at(int i) const {
    return a[i + 1];
  }

  void add(int i, const T &x) {
    a[++i] += x;
    for (; i <= len; i += i & -i) {
      t[i] += x;
    }
  }

  void set(int i, const T &x) {
    T inc = x - a[i + 1];
    add(i, inc);
  }

  T sum(int hi) {
    T res = 0;
    for (hi++; hi > 0; hi -= hi & -hi) {
      res += t[hi];
    }
    return res;
  }

  T sum(int lo, int hi) {
    return sum(hi) - sum(lo - 1);
  }
};

/*** Example Usage and Output:

Values: 5 1 2 3 4

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  fenwick_tree<int> t(5);
  for (int i = 0; i < 5; i++) {
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
