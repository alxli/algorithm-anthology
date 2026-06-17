/*

Given a range of elements, determine a longest subsequence of the range where all elements are in
strictly ascending order. The subsequence is not necessarily contiguous or unique, so only one such
answer will be found. This implementation requires random-access iterators and `operator<` on the
value type.

The answer is computed by scanning left to right while maintaining, for each length, the smallest
possible tail value of an increasing subsequence of that length. Each element extends or improves
one of these tails, located in O(log n) by binary search; predecessor links then reconstruct the
subsequence.

- `longest_increasing_subsequence(lo, hi)` returns a longest strictly increasing subsequence of
  [`lo`, `hi`) as a `std::vector` of the selected values, in order.

Time Complexity:
- O(n log n) per call to `longest_increasing_subsequence()`, where $n$ is the distance between `lo`
  and `hi`.

Space Complexity:
- O(n) auxiliary heap space for `longest_increasing_subsequence()`.

*/

#include <algorithm>
#include <iterator>
#include <vector>

template<class It>
auto longest_increasing_subsequence(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  int len = 0, n = static_cast<int>(hi - lo);
  if (n == 0) {
    return std::vector<T>();
  }
  // tail[i] = index (into [lo, hi)) of the last element of the best known LIS of length i+1.
  // prev[i] = index of the predecessor of element i in its LIS (or -1 if first).
  std::vector<int> prev(n), tail(n);
  for (int i = 0; i < n; i++) {
    // Find the tail to extend or improve. Comparing by value through the stored indices keeps
    // `tail` index-based for reconstruction; swap `<` for `<=` to allow a non-decreasing result.
    int pos = static_cast<int>(
        std::lower_bound(
            tail.begin(), tail.begin() + len, i, [&](int t, int x) { return *(lo + t) < *(lo + x); }
        ) -
        tail.begin()
    );
    if (pos == len) {
      len++;
    }
    prev[i] = pos > 0 ? tail[pos - 1] : -1;
    tail[pos] = i;
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
