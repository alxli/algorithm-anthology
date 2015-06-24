/*

1.2.1 - Longest Increasing Subsequence

Find a subarray of a given array n such that the subarray's
elements are in sorted order, lowest to highest, and in which
the subsequence is as long as possible. This subsequence is not
necessarily contiguous, or unique, so the following function
will find one such sequence.

Time Complexity: O(n log n) on the size of the array.
Space Complexity: O(n) auxiliary.

*/

#include <vector>

int lower_bound(int a[], int tail[], int len, int key) {
  int lo = -1, hi = len;
  while (hi - lo > 1) {
    int mid = (lo + hi) / 2;
    if (a[tail[mid]] < key) lo = mid;
    else hi = mid;
  }
  return hi;
}

template<class T> std::vector<T> lis(int n, T a[]) {
  int tail[n], prev[n];
  int len = 0;
  for (int i = 0; i < n; i++) {
    int pos = lower_bound(a, tail, len, a[i]);
    if (len < pos + 1) len = pos + 1;
    prev[i] = pos > 0 ? tail[pos - 1] : -1;
    tail[pos] = i;
  }
  std::vector<T> res(len);
  for (int i = tail[len - 1]; i != -1; i = prev[i])
    res[--len] = a[i];
  return res;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

template<class It> void print(It lo, It hi) {
  while (lo != hi) cout << *lo++ << " ";
  cout << "\n";
}

int main () {
  int a[] = {-2, -5, 1, 9, 10, 8, 11, 10, 13, 11};
  vector<int> res = lis(10, a);
  print(res.begin(), res.end()); //-5 1 9 10 11 13
  return 0;
}
