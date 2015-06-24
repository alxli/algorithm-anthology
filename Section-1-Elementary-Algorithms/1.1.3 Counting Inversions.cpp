/*

1.1.3 - Counting Inversions

Counts how far (or close) an array is from being sorted.
If the array is already sorted then inversion count is 0.
If the array is sorted in reverse, then the inversion
count is maximal. Formally, two elements a[i] and a[j]
form an inversion if a[i] > a[j] and i < j. Note that the
inversion count is *not* the same as the minimum number
of swaps required to sort the array. This problem can be
solved using merge sort, or a bit of magic...

Time Complexity: O(n log n) on the size of the array.
Space Complexty: O(n) auxiliary.

*/

#include <iterator> /* iterator_traits */

//sorts [lo, hi) and returns the inversions count
template<class RAI>
long long mergesort_inversions(RAI lo, RAI hi) {
  typedef typename std::iterator_traits<RAI>::value_type T;
  if (lo >= hi - 1) return 0;
  RAI mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  long long res = 0;
  res += mergesort_inversions(lo, mid + 1);
  res += mergesort_inversions(mid + 1, hi);
  T buf[hi - lo], *b = buf;
  while (a <= mid && c < hi) {
    if (*c < *a) {
      *(b++) = *(c++);
      res += (mid - a) + 1;
    } else {
      *(b++) = *(a++);
    }
  }
  if (a > mid)
    for (RAI k = c; k < hi; k++) *(b++) = *k;
  else
    for (RAI k = a; k <= mid; k++) *(b++) = *k;
  for (int i = hi - lo - 1; i >= 0; i--)
    *(lo + i) = buf[i];
  return res;
}

/*

The following magic is courtesy of misof, and works
for any array of nonnegative integers. See link below:
http://codeforces.com/blog/entry/17881?#comment-232099

The complexity depends on the magnitude of the maximum
value in a[]. You should coordinate compress the values
of a[] into integers up to n for the best results. Note
that after calling the function, a[] will be 0'd out.

Time Complexity: O(n log n), where n is the magnitude
of the max value in the array.

Space Complexity: O(n) auxiliary on the magnitude of
the max value in the array.

*/

long long magic_inversions(int n, int a[]) {
  int mx = 0;
  for (int i = 0; i < n; i++)
    if (a[i] > mx) mx = a[i];
  int c[mx];
  long long res = 0;
  while (mx > 0) {
    for (int i = 0; i < mx; i++) c[i] = 0;
    for (int i = 0; i < n; i++) {
      if (a[i] % 2 == 0) res += c[a[i] / 2];
      else c[a[i] / 2]++;
    }
    mx = 0;
    for (int i = 0; i < n; i++)
      if ((a[i] /= 2) > mx) mx = a[i];
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  {
    int a[] = {6, 9, 1, 14, 8, 12, 3, 2};
    assert(mergesort_inversions(a, a + 8) == 16);
  }
  {
    int a[] = {6, 9, 1, 14, 8, 12, 3, 2};
    assert(magic_inversions(8, a) == 16);
  }
  return 0;
}
