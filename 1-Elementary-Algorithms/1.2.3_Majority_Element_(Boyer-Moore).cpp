/*

Given two ForwardIterators lo and hi specifying a range [lo, hi) of n elements,
return an iterator to the first occurrence of the majority element, or the
iterator hi if there is no majority element. The majority element is defined as
an element which occurs strictly more than floor(n/2) times in the range. This
implementation requires operator == to be defined on the iterator's value type.

Time Complexity:
- O(n) per call to majority(), where n is the size of the array.

Space Complexity:
- O(1) auxiliary.

*/

template<class It>
It majority(It lo, It hi) {
  int count = 0;
  It candidate = lo;
  for (It it = lo; it != hi; ++it) {
    if (count == 0) {
      candidate = it;
      count = 1;
    } else if (*it == *candidate) {
      count++;
    } else {
      count--;
    }
  }
  count = 0;
  for (It it = lo; it != hi; ++it) {
    if (*it == *candidate) {
      count++;
    }
  }
  if (count <= (hi - lo)/2) {
    return hi;
  }
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
