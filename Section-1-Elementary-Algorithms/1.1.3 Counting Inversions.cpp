/*

The number of inversions in an array a[] is the number of ordered pairs
(i, j) such that i < j and a[i] > a[j]. This is roughly how "close" an
array is to being sorted, but is *not* the same as the minimum number
of swaps required to sort the array. If the array is sorted then the
inversion count is 0. If the array is sorted in decreasing order, then
the inversion count is maximal. The following are two methods of
efficiently counting the number of inversions.

*/

#include <algorithm> /* std::fill(), std::max() */
#include <iterator>  /* std::iterator_traits */

/*

Version 1: Merge sort

The input range [lo, hi) will become sorted after the function call,
and then the number of inversions will be returned. The iterator's
value type must have the less than < operator defined appropriately.

Explanation: http://www.geeksforgeeks.org/counting-inversions

Time Complexity: O(n log n) on the distance between lo and hi.
Space Complexty: O(n) auxiliary.

*/

template<class It> long long inversions(It lo, It hi) {
  if (hi - lo < 2) return 0;
  It mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  long long res = 0;
  res += inversions(lo, mid + 1);
  res += inversions(mid + 1, hi);
  typedef typename std::iterator_traits<It>::value_type T;
  T *buf = new T[hi - lo], *ptr = buf;
  while (a <= mid && c < hi) {
    if (*c < *a) {
      *(ptr++) = *(c++);
      res += (mid - a) + 1;
    } else {
      *(ptr++) = *(a++);
    }
  }
  if (a > mid) {
    for (It k = c; k < hi; k++)
      *(ptr++) = *k;
  } else {
    for (It k = a; k <= mid; k++)
      *(ptr++) = *k;
  }
  for (int i = hi - lo - 1; i >= 0; i--)
    *(lo + i) = buf[i];
  delete[] buf;
  return res;
}

/*

Version 2: Magic

The following magic is courtesy of misof, and works for any array of
nonnegative integers.

Explanation: http://codeforces.com/blog/entry/17881?#comment-232099

The complexity depends on the magnitude of the maximum value in a[].
Coordinate compression should be applied on the values of a[] so that
they are strictly integers with magnitudes up to n for best results.
Note that after calling the function, a[] will be entirely set to 0.

Time Complexity: O(m log m), where m is maximum value in the array.
Space Complexity: O(m) auxiliary.

*/

long long inversions(int n, int a[]) {
  int mx = 0;
  for (int i = 0; i < n; i++)
    mx = std::max(mx, a[i]);
  int *cnt = new int[mx];
  long long res = 0;
  while (mx > 0) {
    std::fill(cnt, cnt + mx, 0);
    for (int i = 0; i < n; i++) {
      if (a[i] % 2 == 0)
        res += cnt[a[i] / 2];
      else
        cnt[a[i] / 2]++;
    }
    mx = 0;
    for (int i = 0; i < n; i++)
      mx = std::max(mx, a[i] /= 2);
  }
  delete[] cnt;
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  {
    int a[] = {6, 9, 1, 14, 8, 12, 3, 2};
    assert(inversions(a, a + 8) == 16);
  }
  {
    int a[] = {6, 9, 1, 14, 8, 12, 3, 2};
    assert(inversions(8, a) == 16);
  }
  return 0;
}
