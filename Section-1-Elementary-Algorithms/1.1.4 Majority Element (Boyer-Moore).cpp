/*

1.1.4 - Majority Element (Boyer-Moore Algorithm)

The majority vote problem is to determine in any
given sequence of choices whether there is a
choice with more occurrences than all the others,
and if so, to determine this choice. In other
words, an element must occur more than floor(N/2)
times in a length N array to be considered the
majority.

Time Complexity: O(n) on the size of the array.
Space Complexty: O(1) auxiliary space.

*/

#include <iterator> /* std::iterator_traits */

//returns an iterator to the majority element in
//the range [lo, hi), or hi if none exists.
template<class It> It majority(It lo, It hi) {
  int cnt = 0;
  It candidate = lo;
  for (It i = lo; i != hi; ++i) {
    if (cnt == 0) {
      candidate = i;
      cnt = 1;
    } else {
      if (*i == *candidate) cnt++;
      else cnt--;
    }
  }
  cnt = 0;
  for (It i = lo; i != hi; ++i)
    if (*i == *candidate) cnt++;
  if (cnt < (hi - lo + 1) / 2) return hi;
  return candidate;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  int a[] = {3, 2, 3, 1, 3};
  int b[] = {2, 2, 3, 1, 3};
  assert(*majority(a, a + 5) == 3);
  assert(majority(b, b + 5) == b + 5);
  return 0;
}
