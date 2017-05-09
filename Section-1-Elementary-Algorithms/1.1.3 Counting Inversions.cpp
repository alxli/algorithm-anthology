/*

The number of inversions for an array a[] is defined as the number of ordered
pairs (i, j) such that i < j and a[i] > a[j]. This is roughly how "close" an
array is to being sorted, but is *not* the minimum number of swaps required to
sort the array. If the array is sorted, then the inversion count is 0. If the
array is sorted in decreasing order, then the inversion count is maximal. The
following two functions are each techniques to efficiently count inversion.

*/

#include <algorithm>  // std::fill(), std::max()
#include <iterator>  // std::iterator_traits
#include <vector>

/*

Version 1: Merge Sort

Returns the number of inversions of the range [lo, hi), where lo and hi are
RandomAccessIterators. Note that the range [lo, hi) will become sorted after
the function call. The value type of the input iterators must have operator<
defined appropriately.

Time Complexity: O(n log n) on the distance between lo and hi.
Space Complexity: O(n) auxiliary.

*/

template<class It> long long inversions(It lo, It hi) {
  if (hi - lo < 2)
    return 0;
  It mid = lo + (hi - lo - 1)/2, a = lo, c = mid + 1;
  long long res = 0;
  res += inversions(lo, mid + 1);
  res += inversions(mid + 1, hi);
  typedef typename std::iterator_traits<It>::value_type T;
  std::vector<T> merged;
  while (a <= mid && c < hi) {
    if (*c < *a) {
      merged.push_back(*(c++));
      res += (mid - a) + 1;
    } else {
      merged.push_back(*(a++));
    }
  }
  if (a > mid) {
    for (It k = c; k < hi; ++k) {
      merged.push_back(*k);
    }
  } else {
    for (It k = a; k <= mid; ++k) {
      merged.push_back(*k);
    }
  }
  for (int i = 0; i < hi - lo; i++) {
    *(lo + i) = merged[i];
  }
  return res;
}

/*

Version 2: Power-of-Two Trick

Returns the number of inversions for an array a[] of n nonnegative integers.
Note that after calling the function, every value of a[] will be set to 0.

Here, the time and space complexities depend on the magnitude of the maximum
value in a[]. Therefore for a running time of O(n log n), coordinate compression
may be applied to a[] so its maximum is strictly less than the length n itself.

Time Complexity: O(m log m), where m is maximum value in the array.
Space Complexity: O(m) auxiliary.

*/

long long inversions(int n, int a[]) {
  int mx = 0;
  for (int i = 0; i < n; i++) {
    mx = std::max(mx, a[i]);
  }
  long long res = 0;
  std::vector<int> cnt(mx);
  while (mx > 0) {
    std::fill(cnt.begin(), cnt.end(), 0);
    for (int i = 0; i < n; i++) {
      if (a[i] % 2 == 0) {
        res += cnt[a[i] / 2];
      } else {
        cnt[a[i] / 2]++;
      }
    }
    mx = 0;
    for (int i = 0; i < n; i++) {
      mx = std::max(mx, a[i] /= 2);
    }
  }
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
