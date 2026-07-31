/*

Given a range of elements, determine a longest subsequence where all elements are in strictly
increasing order. The subsequence is not necessarily contiguous or unique, so only one such answer
will be found. The answer is computed by scanning left to right while maintaining, for each length,
the smallest possible tail value of an increasing subsequence of that length. Each element extends
or improves one of these tails, located in O(log n) by binary search; predecessor links then
reconstruct the subsequence. The comparator `comp` defines the ordering and defaults to
`std::less<>`. This implementation requires random-access iterators.

- `longest_increasing_subsequence(lo, hi, comp = std::less<>())` returns the indices relative to
  `lo` of one longest strictly increasing subsequence of $[`lo`, `hi`)$ according to `comp`, in
  order.

Time Complexity:
- O(n log n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(n) auxiliary and O(n) for the returned indices.

*/

#include <algorithm>
#include <functional>
#include <vector>

template<typename It, typename Compare = std::less<>>
std::vector<int> longest_increasing_subsequence(It lo, It hi, Compare comp = Compare()) {
  int n = static_cast<int>(hi - lo);
  if (n == 0) {
    return {};
  }
  int len = 0;
  // prev[i] = index of the predecessor of element i in its LIS (or -1 for the first LIS element).
  // tail[i] = index (into [lo, hi)) of the last element of the best known LIS of length i + 1.
  std::vector<int> prev(n), tail(n);
  for (int i = 0; i < n; i++) {
    // Find the tail to extend or improve. Comparing by value through the stored indices keeps tail
    // index-based for reconstruction; use an upper-bound search to allow a non-decreasing result.
    int pos = static_cast<int>(
        std::lower_bound(
            tail.begin(), tail.begin() + len, i,
            [&](int t, int x) { return comp(*(lo + t), *(lo + x)); }
        ) -
        tail.begin()
    );
    if (pos == len) {
      len++;
    }
    prev[i] = pos > 0 ? tail[pos - 1] : -1;
    tail[pos] = i;
  }
  std::vector<int> res(len);
  for (int i = tail[len - 1]; i != -1; i = prev[i]) {
    res[--len] = i;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{-2, -5, 1, 9, 10, 8, 11, 10, 13, 11};
  assert((longest_increasing_subsequence(a.begin(), a.end()) == vector<int>{1, 2, 3, 4, 6, 8}));

  vector<int> b{1, 4, 3, 2};
  assert(
      (longest_increasing_subsequence(b.begin(), b.end(), greater<int>()) == vector<int>{1, 2, 3})
  );
  return 0;
}
