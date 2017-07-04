/*

Let k be the number of distinct values in a range [lo, hi) of n total values.
Coordinate compression takes only the integers from [0, n) and reassigns them to
every value in [lo, hi) such that the relative ordering of the original range is
preserved. That is, if a[] is an array containing the original values and b[] is
an array containing the compressed values, then every pair i, j (0 <= i, j < n)
satisfies a[i] < a[j] if and only if b[i] < b[j].

Both implementations below are equivalent, taking RandomAccessIterators lo and
hi as the range [lo, hi) to be compressed. Operator < must be defined on the
iterator's value type. Version 1 performs the compression by sorting the array,
removing duplicates, and binary searching for the position of each original
value. Version 2 achieves the same result by inserting all values in a balanced
binary search tree (std::map) which automatically removes duplicate values and
supports efficient lookups of the compressed values.

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
  std::vector<T> v;
  for (It it = lo; it != hi; ++it) {
    v.push_back(*it);
  }
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
