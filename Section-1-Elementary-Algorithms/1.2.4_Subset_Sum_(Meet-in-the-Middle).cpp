/*

Given random-access iterators lo and hi specifying a range [lo, hi) of integers,
return the minimum sum of any subset of the range that is greater than or equal
to a given integer v. This is a generalization of the NP-complete subset sum
problem, which asks whether a subset summing to 0 exists (equivalent in this
case to checking if v = 0 yields an answer of 0). This implementation uses a
meet-in-the-middle algorithm to precompute and search for a lower bound. Note
that 64-bit integers are used in intermediate calculations to avoid overflow.

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
  int n = hi - lo, llen = 1 << (n/2), hlen = 1 << (n - n/2);
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
  int l = 0, h = hlen - 1;
  long long curr = std::numeric_limits<long long>::min();
  while (l < llen && h >= 0) {
    if (lsum[l] + hsum[h] <= v) {
      curr = std::max(curr, lsum[l] + hsum[h]);
      l++;
    } else {
      h--;
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
