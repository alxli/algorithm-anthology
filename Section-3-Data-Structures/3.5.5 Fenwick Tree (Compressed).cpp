/*

Maintain an array of numerical type, allowing for contiguous sub-arrays to be
simultaneously incremented by arbitrary values (range update) and queries for
the sum of contiguous sub-arrays (range query). This implementation uses
std::map for coordinate compression, allowing for large indices to be accessed
with efficient space complexity. That is, all array indices from 0 to MAXN,
inclusive, are accessible.

- add(i, x) adds x to the value at index i (i.e. a[i] += x).
- add(lo, hi, x) adds x to all indices from lo to hi, inclusive.
- set(i, x) assigns x to the value at index i (i.e. a[i] = x).
- sum(hi) returns the sum of all values at indices from 0 to hi, inclusive.
- sum(lo, hi) returns the sum of all values at indices from lo to hi, inclusive.
- at(i) returns the value at index i, where i is between 0 and MAXN.

Time Complexity:
- O(log^2 MAXN) per call to all member functions. If std::map is replaced with
  std::unordered_map, then the amortized running time will become O(log MAXN).

Space Complexity:
- O(n log MAXN) for storage of the array elements, where n is the number of
  distinct indices that have been accessed across all of the operations so far.
- O(1) auxiliary for all operations.

*/

#include <map>

template<class T> class fenwick_tree {
  static const int MAXN = 1000000001;
  std::map<int, T> tmul, tadd;

  void add_helper(int at, int mul, T add) {
    for (int i = at; i <= MAXN; i |= i + 1) {
      tmul[i] += mul;
      tadd[i] += add;
    }
  }

 public:
  void add(int lo, int hi, const T &x) {
    add_helper(lo, x, -x*(lo - 1));
    add_helper(hi, -x, x*hi);
  }

  void add(int i, const T &x) {
    return add(i, i, x);
  }

  void set(int i, const T &x) {
    add(i, x - at(i));
  }

  T sum(int hi) {
    T mul = 0, add = 0;
    int start = hi;
    for (int i = hi; i >= 0; i = (i & (i + 1)) - 1) {
      if (tmul.find(i) != tmul.end()) {
        mul += tmul[i];
      }
      if (tadd.find(i) != tadd.end()) {
        add += tadd[i];
      }
    }
    return mul*start + add;
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
Sum of the whole array is 92.

***/

#include <iostream>
using namespace std;

int main() {
  int a[] = {10, 1, 2, 3, 4};
  fenwick_tree<int> t;
  for (int i = 0; i < 5; i++) {
    t.set(i, a[i]);
  }
  t.add(0, 2, 5);  // 15 6 7 3 4
  t.set(3, -5);  // 15 6 7 -5 4
  cout << "Values: ";
  for (int i = 0; i < 5; i++) {
    cout << t.at(i) << " ";
  }
  cout << endl << "Sum of range [0, 4] is " << t.sum(0, 4) << "." << endl;
  t.add(500000001, 500000010, 3);
  t.add(500000011, 500000015, 5);
  t.set(500000000, 10);
  cout << "Sum of the whole array is " << t.sum(0, 1000000000) << "." << endl;
  return 0;
}
