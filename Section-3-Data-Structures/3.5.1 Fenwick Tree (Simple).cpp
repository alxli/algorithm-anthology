/*

Maintain an array of numerical type, allowing for updates of individual indices
(point update) and queries for the sum of contiguous sub-arrays (range queries).
This implementation assumes that the array is 1-based (i.e. has valid indices
from 1 to MAXN - 1, inclusive).

- initialize() resets the data structure.
- a[i] stores the value at index i, where i is between 1 and MAXN - 1.
- add(i, x) adds x to the value at index i (i.e. a[i] += x).
- set(i, x) assigns x to the value at index i (i.e. a[i] = x).
- sum(hi) returns the sum of all values at indices from 1 to hi, inclusive.
- sum(lo, hi) returns the sum of all values at indices from lo to hi, inclusive.

Time Complexity:
- O(n) per call to initialize(), where n is the size of the array.
- O(log n) per call to all other operations.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

const int MAXN = 1000;
int a[MAXN + 1], bits[MAXN + 1];

void initialize() {
  for (int i = 0; i <= MAXN; i++) {
    a[i] = bits[i] = 0;
  }
}

void add(int i, int x) {
  a[i] += x;
  for (; i <= MAXN; i += i & -i) {
    bits[i] += x;
  }
}

void set(int i, int x) {
  add(i, x - a[i]);
}

int sum(int hi) {
  int res = 0;
  for (; hi > 0; hi -= hi & -hi) {
    res += bits[hi];
  }
  return res;
}

int sum(int lo, int hi) {
  return sum(hi) - sum(lo - 1);
}

/*** Example Usage and Output:

Values: 5 1 2 3 4
Sum of range [2, 4] is 6.

***/

#include <iostream>
using namespace std;

int main() {
  int v[] = {10, 1, 2, 3, 4};
  initialize();
  for (int i = 1; i <= 5; i++) {
    set(i, v[i - 1]);
  }
  add(1, -5);
  cout << "Values: ";
  for (int i = 1; i <= 5; i++) {
    cout << a[i] << " ";
  }
  cout << endl << "Sum of range [2, 4] is " << sum(2, 4) << "." << endl;
  return 0;
}
