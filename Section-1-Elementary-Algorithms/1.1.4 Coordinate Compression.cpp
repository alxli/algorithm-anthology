/*

1.1.4 - Coordinate Compression

Given an array a[] of size n, reassign integers to each value of a[]
such that the magnitude of each new value is no more than n, while the
relative order of each value as they were in the original array is
preserved. That is, if a[] is the original array and b[] is the result
array, then for every pair (i, j), the result of comparing a[i] < a[j]
will be exactly the same as the result of b[i] < b[j]. Furthermore,
no value of b[] will exceed the *number* of distinct values in a[].

In the following implementations, values in the range [lo, hi) will be
converted to integers in the range [0, d), where d is the number of
distinct values in the original range. lo and hi must be random access
iterators pointing to a numerical type that int can be assigned to.

Time Complexity: O(n log n) on the distance between lo and hi.
Space Complexity: O(n) auxiliary.

*/

#include <algorithm> /* std::lower_bound(), std::sort(), std::unique() */
#include <iterator>  /* std::iterator_traits */
#include <map>

//version 1 - using std::sort(), std::unique() and std::lower_bound()
template<class It> void compress1(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  T *a = new T[hi - lo];
  int n = 0;
  for (It it = lo; it != hi; ++it)
    a[n++] = *it;
  std::sort(a, a + n);
  int n2 = std::unique(a, a + n) - a;
  for (It it = lo; it != hi; ++it)
    *it = (int)(std::lower_bound(a, a + n2, *it) - a);
  delete[] a;
}

//version 2 - using std::map
template<class It> void compress2(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  std::map<T, int> m;
  for (It it = lo; it != hi; ++it)
    m[*it] = 0;
  typename std::map<T, int>::iterator x = m.begin();
  for (int i = 0; x != m.end(); x++)
    x->second = i++;
  for (It it = lo; it != hi; ++it)
    *it = m[*it];
}

/*** Example Usage

Sample Output:

0 4 4 1 3 2 5 5
0 4 4 1 3 2 5 5
1 0 2 0 3 1

***/

#include <iostream>
using namespace std;

template<class It> void print_range(It lo, It hi) {
  while (lo != hi)
    cout << *(lo++) << " ";
  cout << "\n";
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
  { //works on doubles too
    double a[] = {0.5, -1.0, 3, -1.0, 20, 0.5};
    compress1(a, a + 6);
    print_range(a, a + 6);
  }
  return 0;
}
