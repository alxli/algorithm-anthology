/*

1.1.5 - Kth Order Statistics

The following is equivalent in behavior to the std::nth_element()
function in the C++ standard library. They rearrange the elements
in the range [lo, hi), in such a way that the element at the nth
position is the element that would be in that position in a sorted
sequence. The other elements are left without any specific order,
except that none of the elements preceding nth are greater than
it, and none of the elements following it are less than it.

Time Complexity: O(n) expected time on the distance between
lo and hi. However, its worst case is technically O(n^2).
This has the same running time as std::nth_element().

Space Complexity: O(1) auxiliary.

*/

#include <algorithm> /* std::swap() */
#include <cstdlib>   /* rand() */
#include <iterator>  /* std::iterator_traits */

int rand32(int n) {
  return ((rand() & 0x7fff) | ((rand() & 0x7fff) << 15)) % n;
}

template<class It> It rand_partition(It lo, It hi) {
  std::swap(*(lo + rand32(hi - lo)), *(hi - 1));
  typename std::iterator_traits<It>::value_type mid = *(hi - 1);
  It i = lo - 1;
  for (It j = lo; j != hi; ++j)
    if (*j <= mid) std::swap(*(++i), *j);
  return i;
}

template<class It> void nth_element2(It lo, It n, It hi) {
  for (;;) {
    It k = rand_partition(lo, hi);
    if (n < k) hi = k;
    else if (n > k) lo = k + 1;
    else return;
  }
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

template<class It> void print(It lo, It hi) {
  while (lo != hi) cout << *lo++ << " ";
  cout << "\n";
}

int main () {
  int a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  random_shuffle(a, a + 9);
  nth_element2(a, a + 5, a + 9);
  print(a, a + 9);
  return 0;
}
