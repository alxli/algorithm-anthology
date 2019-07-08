/*

Given two ForwardIterators as a range [lo, hi) of n numerical elements, reassign
each element in the range to an integer in [0, k), where k is the number of
distinct elements in the original range, while preserving the initial relative
ordering of elements. That is, if a[] is an array of the original values and b[]
is the compressed values, then every pair of indices i, j (0 <= i, j < n) shall
satisfy a[i] < a[j] if and only if b[i] < b[j].

Both implementations below require operator < to be defined on the iterator's
value type. Version 1 performs the compression by sorting the array, removing
duplicates, and binary searching for the position of each original value.
Version 2 achieves the same result by inserting all values in a balanced binary
search tree (std::map) which automatically removes duplicate values and supports
efficient lookups of the compressed values.

Time Complexity:
- O(n log n) per call to either function, where n is the distance between lo and
  hi.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <algorithm>
#include <iterator>
#include <map>
#include <vector>

template<class It> void compress1(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  std::vector<T> v(lo, hi);
  std::sort(v.begin(), v.end());
  v.resize(std::unique(v.begin(), v.end()) - v.begin());
  for (It it = lo; it != hi; ++it) {
    *it = (int)(std::lower_bound(v.begin(), v.end(), *it) - v.begin());
  }
}

template<class It> void compress2(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  std::map<T, int> m;
  for (It it = lo; it != hi; ++it) {
    m[*it] = 0;
  }
  typename std::map<T, int>::iterator it = m.begin();
  for (int id = 0; it != m.end(); it++) {
    it->second = id++;
  }
  for (It it = lo; it != hi; ++it) {
    *it = m[*it];
  }
}

/*** Example Usage and Output:

0 4 4 1 3 2 5 5
0 4 4 1 3 2 5 5
1 0 2 0 3 1

***/

#include <iostream>
using namespace std;

template<class It> void print_range(It lo, It hi) {
  while (lo != hi) {
    cout << *lo++ << " ";
  }
  cout << endl;
}

int main() {
  {
    int a[] = {1, 30, 30, 7, 9, 8, 99, 99};
    compress1(a, a + 8);
    print_range(a, a + 8);
  }
  {
    int a[] = {1, 30, 30, 7, 9, 8, 99, 99};
    compress2(a, a + 8);
    print_range(a, a + 8);
  }
  {  // Non-integral types work too, as long as ints can be assigned to them.
    double a[] = {0.5, -1.0, 3, -1.0, 20, 0.5};
    compress1(a, a + 6);
    print_range(a, a + 6);
  }
  return 0;
}
