/*

Maintain an array of numerical type, allowing for contiguous sub-arrays to be
simultaneously incremented by arbitrary values (range update) and queries for
the sum of contiguous sub-arrays (range query). This implementation assumes that
the array is 0-based (i.e. has valid indices from 0 to size() - 1, inclusive).

- size() returns the size of the array.
- add(i, x) adds x to the value at index i (i.e. a[i] += x).
- add(lo, hi, x) adds x to all indices from lo to hi, inclusive.
- set(i, x) assigns x to the value at index i (i.e. a[i] = x).
- sum(hi) returns the sum of all values at indices from 0 to hi, inclusive.
- sum(lo, hi) returns the sum of all values at indices from lo to hi, inclusive.
- at(i) returns the value at index i, where i is between 0 and size() - 1.

Time Complexity:
- O(n) per call to the constructor, where n is the size of the array.
- O(1) per call to size().
- O(log n) per call to all other operations.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary per call to all operations.

*/

#include <vector>

template<class T> class fenwick_tree {
  int len;
  std::vector<T> b1, b2;

  T sum(const std::vector<T> &b, int i) {
    T res = 0;
    for (; i != 0; i -= i & -i) {
      res += b[i];
    }
    return res;
  }

  void add(std::vector<T> &b, int i, const T &x) {
    for (; i <= len + 1; i += i & -i) {
      b[i] += x;
    }
  }

 public:
  fenwick_tree(int n) {
    len = n;
    b1.resize(n + 2);
    b2.resize(n + 2);
  }

  int size() const {
    return len;
  }

  void add(int lo, int hi, const T &x) {
    lo++;
    hi++;
    add(b1, lo, x);
    add(b1, hi + 1, -x);
    add(b2, lo, x*(lo - 1));
    add(b2, hi + 1, -x*hi);
  }

  void add(int i, const T &x) {
    return add(i, i, x);
  }

  void set(int i, const T & x) {
    add(i, x - at(i));
  }

  T sum(int hi) {
    hi++;
    return hi*sum(b1, hi) - sum(b2, hi);
  }

  T sum(int lo, int hi) {
    return sum(hi) - sum(lo - 1);
  }

  T at(int i) {
    return sum(i, i);
  }
};

/*** Example Usage and Output:

Values: 15 6 7 -5 4
Sum of range [0, 4] is 27.

***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  fenwick_tree<int> t(5);
  for (int i = 0; i < t.size(); i++) {
    t.set(i, a[i]);
  }
  t.add(0, 2, 5);  // 15 6 7 3 4
  t.set(3, -5);  // 15 6 7 -5 4
  cout << "Values: ";
  for (int i = 0; i < t.size(); i++) {
    cout << t.at(i) << " ";
  }
  cout << endl << "Sum of range [0, 4] is " << t.sum(0, 4) << "." << endl;
  return 0;
}
