/*

1.2.3 - Majority Element (Boyer-Moore Algorithm)

Given a sequence of n elements, the majority vote problem asks to find
an element that occurs more frequently than all others, or determine
that no such element exists. Formally, a value must occur strictly
greater than floor(n/2) times to be considered the majority element.
Boyer-Moore majority vote algorithm scans through the sequence and
keeps track of a running counter for the most likely candidate so far.
Whenever a value is equal to the current candidate, the counter is
incremented, otherwise the counter is decremented. When the counter is
zero, the candidate is eliminated and a new candidate is considered.

The following implementation takes two random access iterators as the
sequence [lo, hi) of elements and returns an iterator pointing to one
instance of the majority element if it exists, or the iterator hi if
there is no majority.

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
  for (It it = lo; it != hi; ++it)
    if (*it == *candidate) cnt++;
  if (cnt <= (hi - lo) / 2)
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
