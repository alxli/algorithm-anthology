/*

1.2.4 - Subset Sum (Meet-in-the-Middle)

Given a sequence of n (not necessarily unique) integers and a number v,
determine the minimum possible sum of any subset of the given sequence
that is not less than v. This is a generalization of a more well-known
version of the subset sum problem which asks whether a subset summing
to 0 exists (equivalent here to seeing if v = 0 yields an answer of 0).
Both problems are NP-complete. A meet-in-the-middle algorithm divides
the array in two equal parts. All possible sums of the lower and higher
parts are precomputed and sorted in a table. Finally, the table is
searched to find the lower bound.

The following implementation accepts two random access iterators as the
sequence [lo, hi) of integers, and the number v. Note that since the
sums can get large, 64-bit integers are necessary to avoid overflow.

Time Complexity: O(n * 2^(n/2)) on the distance between lo and hi.
Space Complexity: O(n) auxiliary.

*/

#include <algorithm> /* std::max(), std::sort() */
#include <limits>    /* std::numeric_limits */

template<class It>
long long sum_lower_bound(It lo, It hi, long long v) {
  int n = hi - lo;
  int llen = 1 << (n / 2);
  int hlen = 1 << (n - n / 2);
  long long *lsum = new long long[llen];
  long long *hsum = new long long[hlen];
  std::fill(lsum, lsum + llen, 0);
  std::fill(hsum, hsum + hlen, 0);
  for (int mask = 0; mask < llen; mask++)
    for (int i = 0; i < n / 2; i++)
      if ((mask >> i) & 1)
        lsum[mask] += *(lo + i);
  for (int mask = 0; mask < hlen; mask++)
    for (int i = 0; i < n - n / 2; i++)
      if ((mask >> i) & 1)
        hsum[mask] += *(lo + i + n / 2);
  std::sort(lsum, lsum + llen);
  std::sort(hsum, hsum + llen);
  int l = 0, r = hlen - 1;
  long long curr = std::numeric_limits<long long>::min();
  while (l < llen && r >= 0) {
    if (lsum[l] + hsum[r] <= v) {
      curr = std::max(curr, lsum[l] + hsum[r]);
      l++;
    } else {
      r--;
    }
  }
  delete[] lsum;
  delete[] hsum;
  return curr;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  int a[] = {9, 1, 5, 0, 1, 11, 5};
  assert(sum_lower_bound(a, a + 7, 8) == 7);
  int b[] = {-7, -3, -2, 5, 8};
  assert(sum_lower_bound(b, b + 5, 0) == 0);
  return 0;
}
