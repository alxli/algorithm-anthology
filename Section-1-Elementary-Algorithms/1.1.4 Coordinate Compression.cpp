/*

1.1.4 - Coordinate Compression

Given an array of length n, reassigns values to the array elements
such that the magnitude of each new value is no more than n, while
preserving the relative order of each element as they existed in
the original array. Formally, if a[] is the original array and b[]
is the compressed array, for every pair i, j (0 <= i, j < n), the
result of comparing a[i] compared to a[j] (<, >=, ==, etc.) will
be exactly the same as the result of b[i] compared to b[j].

For the following implementations, values in the range [lo, hi)
will be changed to integers in the range [0, D), where D is the
number of distinct values in the range. lo and hi must be iterators
pointing to a numerical type that int can be assigned to.

Time Complexity: O(n log n) on the size of the array.
Space Complexity: O(n) auxiliary on the size of the array.

*/

#include <algorithm> /* lower_bound(), sort(), unique() */
#include <iterator>  /* std::iterator_traits */
#include <map>

//version 1 - using std::unique() and std::lower_bound()
template<class It> void compress2(It lo, It hi) {
  typename std::iterator_traits<It>::value_type a[hi - lo];
  int n = 0;
  for (It it = lo; it != hi; ++it) a[n++] = *it;
  std::sort(a, a + n);
  int n2 = std::unique(a, a + n) - a;
  for (It it = lo; it != hi; ++it)
    *it = (int)(std::lower_bound(a, a + n2, *it) - a);
}

//version 2 - using std::map
template<class It> void compress1(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  std::map<T, int> m;
  for (It it = lo; it != hi; ++it) m[*it] = 0;
  typename std::map<T, int>::iterator x = m.begin();
  for (int i = 0; x != m.end(); x++) x->second = i++;
  for (It it = lo; it != hi; ++it) *it = m[*it];
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

template<class It> void print(It lo, It hi) {
  while (lo != hi) cout << *lo++ << " ";
  cout << "\n";
}

int main() {
  {
    int a[] = {1, 30, 30, 7, 9, 8, 99, 99};
    compress1(a, a + 8);
    print(a, a + 8); //0 4 4 1 3 2 5 5
  }

  {
    int a[] = {1, 30, 30, 7, 9, 8, 99, 99};
    compress2(a, a + 8);
    print(a, a + 8); //0 4 4 1 3 2 5 5
  }

  { //works on doubles too
    double a[] = {0.5, -1.0, 3, -1.0, 20, 0.5};
    compress1(a, a + 6);
    print(a, a + 6); //1 0 2 0 3 1
  }
  return 0;
}
