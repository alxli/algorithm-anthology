/*

Given two InputIterators lo and hi specifying a range [lo, hi) of n (not
necessarily distinct) integers and an integer v, returns the minimum possible
sum of any subset of the given sequence that is greater than or equal to v.
This is a generalization of a more well-known version of the subset sum problem
which asks whether a subset summing to 0 exists (equivalent in this case to
checking if v = 0 yields an answer of 0). Both problems are NP-complete. This
meet-in-the-middle algorithm divides the array in two equal parts. All possible
sums of the lower and higher parts are precomputed into a table and sorted.
Finally, the table is searched to find the lower bound.

Note that since the sums can get large, 64-bit integers are used in intermediate
calculations to avoid overflow.

Time Complexity:
- O(n*2^(n/2)) per call to sum_lower_bound(), where n is the distance between lo
  and hi.

Space Complexity:
- O(n) auxiliary heap space, where n is the number of array elements.

*/

#include <algorithm>
#include <limits>
#include <vector>

template<class It>
long long sum_lower_bound(It lo, It hi, long long v) {
  int n = hi - lo;
  int llen = 1 << (n/2);
  int hlen = 1 << (n - n/2);
  std::vector<long long> lsum(llen), hsum(hlen);
  for (int mask = 0; mask < llen; mask++) {
    for (int i = 0; i < n/2; i++) {
      if ((mask >> i) & 1) {
        lsum[mask] += *(lo + i);
      }
    }
  }
  for (int mask = 0; mask < hlen; mask++) {
    for (int i = 0; i < (n - n/2); i++) {
      if ((mask >> i) & 1) {
        hsum[mask] += *(lo + i + n/2);
      }
    }
  }
  std::sort(lsum.begin(), lsum.end());
  std::sort(hsum.begin(), hsum.end());
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
