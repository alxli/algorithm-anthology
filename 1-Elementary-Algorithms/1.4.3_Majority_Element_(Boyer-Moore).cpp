/*

Given a range of $n$ elements, find the majority element: an element that occurs strictly more than
$\lfloor n/2 \rfloor$ times in the range.

The Boyer-Moore voting algorithm performs a first pass while maintaining a single candidate and a
counter, incrementing it on a match and decrementing on a mismatch, replacing the candidate when the
counter reaches zero; any majority element necessarily survives as the final candidate. The second
pass counts the candidate's occurrences to confirm it truly holds a majority. The range is therefore
traversed twice, so it only needs to be supplied as ForwardIterators rather than random-access.

If a majority element is guaranteed in advance to exist, the verification pass can be dropped and
the candidate returned directly. The candidate phase alone is single-pass, so that variant could be
written as a "true" streaming API with one element per call. 

- `majority(lo, hi)` returns an iterator to the first occurrence of the majority element of the
  range [`lo`, `hi`), or `hi` if no majority element exists. Requires `operator==` on the value
  type.

Time Complexity:
- O(n) per call, where $n$ is the size of the array.

Space Complexity:
- O(1) auxiliary.

*/

template<typename It>
It majority(It lo, It hi) {
  int count = 0, n = 0;
  It candidate = lo;
  for (It it = lo; it != hi; ++it) {
    n++;  // Tally the size to avoid a O(n) std::distance() pass in case of ForwardIterators.
    if (count == 0) {
      candidate = it;
      count = 1;
    } else if (*it == *candidate) {
      count++;
    } else {
      count--;
    }
  }
  // Second pass: verify the candidate. Omit this if a majority is guaranteed to exist.
  count = 0;
  for (It it = lo; it != hi; ++it) {
    if (*it == *candidate) {
      count++;
    }
  }
  if (count <= n / 2) {
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
