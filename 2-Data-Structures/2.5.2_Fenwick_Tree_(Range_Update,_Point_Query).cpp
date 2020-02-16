/*

Maintain an array of numerical type, allowing for contiguous sub-arrays to be
simultaneously incremented by arbitrary values (range update) and values at
individual indices to be queried (point query). This implementation assumes that
the array is 0-based (i.e. has valid indices from 0 to size() - 1, inclusive).

- size() returns the size of the array.
- at(i) returns the value at index i.
- add(i, x) adds x to the value at index i.
- add(lo, hi, x) adds x to the values at all indices from lo to hi, inclusive.

Time Complexity:
- O(n) per call to the constructor, where n is the size of the array.
- O(1) per call to size().
- O(log n) per call to at() and both add() functions.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <vector>

template<class T>
class fenwick_tree {
  int len;
  std::vector<int> t;

 public:
  fenwick_tree(int n) : len(n), t(n + 2) {}

  int size() const {
    return len;
  }

  T at(int i) const {
    T res = 0;
    for (i++; i > 0; i -= i & -i) {
      res += t[i];
    }
    return res;
  }

  void add(int i, const T &x) {
    for (i++; i <= len + 1; i += i & -i) {
      t[i] += x;
    }
  }

  void add(int lo, int hi, const T &x) {
    add(lo, x);
    add(hi + 1, -x);
  }
};

/*** Example Usage and Output:

Values: 5 10 15 10 10

***/

#include <iostream>
using namespace std;

int main() {
  fenwick_tree<int> t(5);
  t.add(0, 1, 5);
  t.add(1, 2, 5);
  t.add(2, 4, 10);
  cout << "Values: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t.at(i) << " ";
  }
  cout << endl;
  return 0;
}
