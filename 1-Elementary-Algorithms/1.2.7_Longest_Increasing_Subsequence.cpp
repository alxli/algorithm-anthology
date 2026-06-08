/*

Given two random-access iterators `lo` and `hi` specifying a range `[lo, hi)`, determine a longest
subsequence of the range such that all of its elements are in strictly ascending order. This
implementation requires `operator<` to be defined on the iterator's value type. The subsequence is
not necessarily contiguous or unique, so only one such answer will be found. The answer is computed
using binary search and dynamic programming.

Time Complexity:
- O(n log n) per call to `longest_increasing_subsequence()`, where $n$ is the distance between `lo`
  and `hi`.

Space Complexity:
- O(n) auxiliary heap space for `longest_increasing_subsequence()`.

*/

#include <iterator>
#include <vector>

template<class It>
auto longest_increasing_subsequence(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  int len = 0, n = hi - lo;
  if (n == 0) {
    return std::vector<T>();
  }
  // tail[i] = index (into [lo, hi)) of the last element of the best known LIS of length i+1.
  // prev[i] = index of the predecessor of element i in its LIS (or -1 if first).
  std::vector<int> prev(n), tail(n);
  for (int i = 0; i < n; i++) {
    int left = -1, right = len;
    while (right - left > 1) {
      int mid = left + (right - left) / 2;
      if (*(lo + tail[mid]) < *(lo + i)) {
        left = mid;
      } else {
        right = mid;
      }
    }
    if (len < right + 1) {
      len = right + 1;
    }
    prev[i] = right > 0 ? tail[right - 1] : -1;
    tail[right] = i;
  }
  std::vector<T> res(len);
  for (int i = tail[len - 1]; i != -1; i = prev[i]) {
    res[--len] = *(lo + i);
  }
  return res;
}

/*** Example Usage and Output:

-5 1 9 10 11 13

***/

#include <iostream>
using namespace std;

template<class It>
void print_range(It lo, It hi) {
  while (lo != hi) {
    cout << *lo++ << " ";
  }
  cout << endl;
}

int main() {
  vector<int> a{-2, -5, 1, 9, 10, 8, 11, 10, 13, 11};
  vector<int> res = longest_increasing_subsequence(a.begin(), a.end());
  print_range(res.begin(), res.end());
  return 0;
}
