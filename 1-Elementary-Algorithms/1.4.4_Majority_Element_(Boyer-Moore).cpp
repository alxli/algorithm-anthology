/*

Given two random-access iterators `lo` and `hi` specifying a range `[lo, hi)` of $n$ elements,
return an iterator to the first occurrence of the majority element, or the iterator `hi` if there
is no majority element. The majority element is defined as an element which occurs strictly more
than $\lfloor n/2 \rfloor$ times in the range. This implementation requires `operator==` to be
defined on the iterator's value type.

Time Complexity:
- O(n) per call to `majority()`, where $n$ is the size of the array.

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
  if (count <= (hi - lo) / 2) {
    return hi;
  }
  return candidate;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>

int main() {
  std::vector<int> a{3, 2, 3, 1, 3};
  assert(*majority(a.begin(), a.end()) == 3);
  std::vector<int> b{2, 3, 3, 3, 2, 1};
  assert(majority(b.begin(), b.end()) == b.end());
  return 0;
}
