/*

Given random-access iterators `lo` and `hi` specifying a range `[lo, hi)` of integers, return the
maximum sum of any subset of the range that is at most a given integer `v`. This is a common
meet-in-the-middle version of subset sum: enumerate subset sums for each half, sort one side, and
binary search for the best compatible partner. Note that 64-bit integers are used in intermediate
calculations to avoid overflow.

Time Complexity:
- O(2^(n/2)) per call to `max_subset_sum_at_most()`, where $n$ is the distance between `lo` and
  `hi`.

Space Complexity:
- O(2^(n/2)) auxiliary heap space.

*/

#include <algorithm>
#include <climits>
#include <vector>

template<class It>
long long max_subset_sum_at_most(It lo, It hi, long long v) {
  int n = hi - lo, llen = 1 << (n / 2), hlen = 1 << (n - n / 2);
  std::vector<long long> lsum(llen), hsum(hlen);
  for (int mask = 1; mask < llen; mask++) {
    int bit = __builtin_ctz(mask);
    lsum[mask] = lsum[mask ^ (1 << bit)] + *(lo + bit);
  }
  for (int mask = 1; mask < hlen; mask++) {
    int bit = __builtin_ctz(mask);
    hsum[mask] = hsum[mask ^ (1 << bit)] + *(lo + n / 2 + bit);
  }
  std::sort(hsum.begin(), hsum.end());
  long long res = LLONG_MIN;
  for (long long x : lsum) {
    auto it = std::upper_bound(hsum.begin(), hsum.end(), v - x);
    if (it != hsum.begin()) {
      res = std::max(res, x + *--it);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  std::vector<int> a{9, 1, 5, 0, 1, 11, 5};
  assert(max_subset_sum_at_most(a.begin(), a.end(), 8) == 7);
  std::vector<int> b{-7, -3, -2, 5, 8};
  assert(max_subset_sum_at_most(b.begin(), b.end(), 0) == 0);
  return 0;
}
