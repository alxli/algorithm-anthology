/*

Maintain an array of numerical type, allowing for updates of individual indices (point update) and
queries for the sum of contiguous sub-arrays (range queries). This implementation assumes that the
array is 1-based (i.e. has valid indices from 1 to `n`, inclusive).

- `initialize(n)` resets the dat structure.
- `vals[i]` stores the value at index `i`.
- `add(i, x)` adds `x` to the value at index `i`.
- `set(i, x)` assigns the value at index `i` to `x`.
- `sum(hi)` returns the sum of all values at indices from 1 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.

Time Complexity:
- O(n) per call to `initialize(n)`, where $n$ is the size of the array.
- O(log n) per call to all other operations.

Space Complexity:
- O(n) for storage of the array elements.
- O(1) auxiliary for all operations.

*/

#include <vector>

std::vector<int> vals, tree;

void initialize(int n) {
  vals.assign(n + 2, 0);
  tree.assign(n + 2, 0);
}

void add(int i, int x) {
  vals[i] += x;
  for (; i < static_cast<int>(tree.size()); i += i & -i) {
    tree[i] += x;
  }
}

void set(int i, int x) {
  add(i, x - vals[i]);
}

int sum(int hi) {
  int res = 0;
  for (; hi > 0; hi -= hi & -hi) {
    res += tree[hi];
  }
  return res;
}

int sum(int lo, int hi) {
  return sum(hi) - sum(lo - 1);
}

/*** Example Usage and Output:

Values: 5 1 2 3 4

***/

#include <cassert>
#include <iostream>
using namespace std;

int main() {
  vector<int> v{10, 1, 2, 3, 4};
  int n = 5;
  initialize(n);
  for (int i = 1; i < n; i++) {
    set(i, v[i - 1]);
  }
  add(1, -5);
  cout << "Values: ";
  for (int i = 1; i < n; i++) {
    cout << vals[i] << " ";
  }
  cout << endl;
  assert(sum(2, 4) == 6);
  return 0;
}
