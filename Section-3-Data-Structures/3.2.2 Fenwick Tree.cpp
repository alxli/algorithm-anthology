/*

Maintain an array of integral type, allowing the sum of an arbitrary consecutive
sub-array to be dynamically queried in logarithmic time. This implementation
assumes that the maintained array is 0-based (i.e. has valid indices from 0 to
size() - 1, inclusive).

- size() returns the size of the array.
- at(i) returns the value at index i, where i is between 0 and size() - 1.
- add(i, x) adds x to the value at index i (i.e. a[i] += x).
- set(i, x) assigns x to the value at index i (i.e. a[i] = x).
- sum(hi) returns the sum of all values at indices from 0 to hi, inclusive.
- sum(lo, hi) returns the sum of all values at indices from lo to hi, inclusive.

Time Complexity:
- O(1) per call to the constructor, size(), and at().
- O(log n) per call to add(), set(), sum(hi), and sum(lo, hi), where n is the
  size of the array.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary per call to all other operations.

*/

#include <vector>

template<class Int> class fenwick_tree {
  int len;
  std::vector<int> data, bits;

 public:
  fenwick_tree(int n) {
    len = n;
    data.resize(n + 1);
    bits.resize(n + 1);
  }

  int size() const {
    return len;
  }

  Int at(int i) const {
    return data[i + 1];
  }

  void add(int i, const Int &x) {
    data[++i] += x;
    for (; i <= len; i += i & -i) {
      bits[i] += x;
    }
  }

  void set(int i, const Int &x) {
    Int inc = x - data[i + 1];
    add(i, inc);
  }

  Int sum(int hi) {
    Int res = 0;
    for (hi++; hi > 0; hi -= hi & -hi) {
      res += bits[hi];
    }
    return res;
  }

  Int sum(int lo, int hi) {
    return sum(hi) - sum(lo - 1);
  }
};

/*** Example Usage and Output:

Values: 5 1 2 3 4
Sum of range [1, 3] is 6.

***/

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
  cout << "." << endl
       << "Sum of range [1, 3] is " << t.sum(1, 3) << "." << endl;
  return 0;
}
