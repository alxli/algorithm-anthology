/*

nth_element2() is equivalent to std::nth_element(), taking RandomAccessIterators
lo, nth, and hi as the range [lo, hi) to be partially sorted. The values in
[lo, hi) are rearranged such that the value pointed to by nth is the element
that would be there if the range were sorted. Furthermore, the range is
partitioned such that no value in [lo, nth) compares greater than the value
pointed to by nth and no value in (nth, hi) compares less.

This implementation is not intended to compete with nth_element() in terms of
efficiency. Instead, it is meant to demonstrate how Quickselect can be concisely
implemented in C++. Nonetheless, both functions run in expected linear time.

Time Complexity (Average): O(n) on the distance between lo and hi.
Time Complexity (Worst): O(n^2), though this will almost never occur.
Space Complexity: O(1) auxiliary.

*/

#include <algorithm>  // std::swap()
#include <cstdlib>  // rand()
#include <iterator>  // std::iterator_traits

int rand32() {
  return (rand() & 0x7fff) | ((rand() & 0x7fff) << 15);
}

template<class It> void nth_element2(It lo, It nth, It hi) {
  for (;;) {
    std::swap(*(lo + rand32() % (hi - lo)), *(hi - 1));
    typename std::iterator_traits<It>::value_type mid = *(hi - 1);
    It k = lo - 1;
    for (It it = lo; it != hi; ++it) {
      if (*it <= mid)
        std::swap(*(++k), *it);
    }
    if (nth < k)
      hi = k;
    else if (nth > k)
      lo = k + 1;
    else
      return;
  }
}

/*** Example Usage and Output:

The median is 5.
3 2 3 4 5 6 6 9 7

***/

#include <iostream>
using namespace std;

template<class It> void print_range(It lo, It hi) {
  while (lo != hi)
    cout << *(lo++) << " ";
  cout << endl;
}

int main () {
  int n = 9;
  int a[] = {5, 6, 4, 3, 2, 6, 7, 9, 3};
  nth_element2(a, a + n/2, a + n);
  cout << "The median is " << a[n/2] << "." << endl;
  print_range(a, a + n);
  return 0;
}
