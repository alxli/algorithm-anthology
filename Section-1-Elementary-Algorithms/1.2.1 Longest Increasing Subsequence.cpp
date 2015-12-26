/*

1.2.1 - Longest Increasing Subsequence

Given an array a[] of size n, determine a longest subsequence of a[]
such that all of its elements are in ascending order. This subsequence
is not necessarily contiguous or unique, so only one such answer will
be found. The problem is efficiently solved using dynamic programming
and binary searching, since it has the following optimal substructure
with respect to the i-th position in the array:

  LIS[i] = 1 + max(LIS[j] for all j < i and a[j] < a[i])
  Otherwise if such a j does not exist, then LIS[i] = 1.

Explanation: https://en.wikipedia.org/wiki/Longest_increasing_subsequence

Time Complexity: O(n log n) on the size of the array.
Space Complexity: O(n) auxiliary.

*/

#include <vector>

std::vector<int> tail, prev;

template<class T> int lower_bound(int len, T a[], int key) {
  int lo = -1, hi = len;
  while (hi - lo > 1) {
    int mid = (lo + hi) / 2;
    if (a[tail[mid]] < key)
      lo = mid;
    else
      hi = mid;
  }
  return hi;
}

template<class T> std::vector<T> lis(int n, T a[]) {
  tail.resize(n);
  prev.resize(n);
  int len = 0;
  for (int i = 0; i < n; i++) {
    int pos = lower_bound(len, a, a[i]);
    if (len < pos + 1) len = pos + 1;
    prev[i] = pos > 0 ? tail[pos - 1] : -1;
    tail[pos] = i;
  }
  std::vector<T> res(len);
  for (int i = tail[len - 1]; i != -1; i = prev[i])
    res[--len] = a[i];
  return res;
}

/*** Example Usage

Sample Output:
-5 1 9 10 11 13

***/

#include <iostream>
using namespace std;

template<class It> void print_range(It lo, It hi) {
  while (lo != hi)
    cout << *(lo++) << " ";
  cout << "\n";
}

int main () {
  int a[] = {-2, -5, 1, 9, 10, 8, 11, 10, 13, 11};
  vector<int> res = lis(10, a);
  print_range(res.begin(), res.end());
  return 0;
}
