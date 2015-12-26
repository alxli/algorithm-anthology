/*

1.1.5 - Selection (Quickselect)

Quickselect (also known as Hoare's algorithm) is a selection algorithm
which rearranges the elements in a sequence such that the element at
the nth position is the element that would be there if the sequence
were sorted. The other elements in the sequence are partioned around
the nth element. That is, they are left in no particular order, except
that no element before the nth element is is greater than it, and no
element after it is less.

The following implementation is equivalent to std::nth_element(),
taking in two random access iterators as the range and performing the
described operation in expected linear time.

Time Complexity (Average): O(n) on the distance between lo and hi.
Time Complexity (Worst): O(n^2), although this *almost never* occurs.
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
    if (*j <= mid)
      std::swap(*(++i), *j);
  return i;
}

template<class It> void nth_element2(It lo, It n, It hi) {
  for (;;) {
    It k = rand_partition(lo, hi);
    if (n < k)
      hi = k;
    else if (n > k)
      lo = k + 1;
    else
      return;
  }
}

/*** Example Usage

Sample Output:
2 3 1 5 4 6 8 7 9

***/

#include <iostream>
using namespace std;

template<class It> void print_range(It lo, It hi) {
  while (lo != hi)
    cout << *(lo++) << " ";
  cout << "\n";
}

int main () {
  int a[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
  random_shuffle(a, a + 9);
  nth_element2(a, a + 5, a + 9);
  print_range(a, a + 9);
  return 0;
}
