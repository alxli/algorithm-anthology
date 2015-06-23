/*

1.1.6 - Subset Sum (Meet-in-the-Middle)

Given an unsorted array of integers and a value v,
find the minimum sum of any subset of values in
the array that is not less than v. A meet-in-the-
middle algorithm divides the array in two equal
parts. All possible sums are precomputed and stored
in a table and sorted by the sum. To find the
answer, the table is searched.

Time Complexity: O(n * 2^(n / 2)) on the length of
the array.

Space Complexity: O(n) auxiliary on the length of
the array.

*/

#include <algorithm> /* std::max(), std::sort() */
#include <climits>   /* LL_MIN */

template<class It>
long long sum_lower_bound(It lo, It hi, long long v) {
  int n = hi - lo;
  int llen = 1 << (n / 2);
  int rlen = 1 << (n - n / 2);
  long long lsum[llen], rsum[rlen];
  std::fill(lsum, lsum + llen, 0);
  std::fill(rsum, rsum + rlen, 0);
  for (int i = 0; i < llen; ++i)
    for (int j = 0; j < n / 2; ++j)
      if ((i & (1 << j)) > 0)
        lsum[i] += *(lo + j);
  for (int i = 0; i < rlen; ++i)
    for (int j = 0; j < n - n / 2; ++j)
      if ((i & (1 << j)) > 0)
        rsum[i] += *(lo + j + n / 2);
  std::sort(lsum, lsum + llen);
  std::sort(rsum, rsum + llen);
  int l = 0, r = rlen - 1;
  long long cur = LLONG_MIN;
  while (l < llen && r >= 0) {
    if (lsum[l] + rsum[r] <= v) {
      cur = std::max(cur, lsum[l] + rsum[r]);
      l++;
    } else {
      r--;
    }
  }
  return cur;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

int main() {
  long long a[] = {9, 1, 5, 0, 1, 11, 5};
  cout << sum_lower_bound(a, a + 7, 8) << "\n"; //7
  return 0;
}
