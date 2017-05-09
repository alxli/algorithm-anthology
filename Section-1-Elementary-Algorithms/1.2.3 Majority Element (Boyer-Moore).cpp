/*

Given two InputIterators lo and hi specifying a range [lo, hi) of n elements,
returns a reference to the first occurrence of the majority element, or the
iterator hi if there is no majority element. The majority element is defined as
an element which occurs strictly greater than floor(n/2) times in the range.

Time Complexity: O(n) on the size of the array.
Space Complexty: O(1) auxiliary.

*/

template<class It> It majority(It lo, It hi) {
  int cnt = 0;
  It candidate = lo;
  for (It it = lo; it != hi; ++it) {
    if (cnt == 0) {
      candidate = it;
      cnt = 1;
    } else if (*it == *candidate) {
      cnt++;
    } else {
      cnt--;
    }
  }
  cnt = 0;
  for (It it = lo; it != hi; ++it) {
    if (*it == *candidate) {
      cnt++;
    }
  }
  if (cnt <= (hi - lo)/2)
    return hi;
  return candidate;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  int a[] = {3, 2, 3, 1, 3};
  assert(*majority(a, a + 5) == 3);
  int b[] = {2, 3, 3, 3, 2, 1};
  assert(majority(b, b + 6) == b + 6);
  return 0;
}
