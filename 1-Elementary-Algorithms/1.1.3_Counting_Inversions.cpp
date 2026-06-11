/*

The number of inversions for a sequence `a` is the number of ordered pairs $(i, j)$ such that
$i < j$ and $a[i] > a[j]$. This is roughly how "close" an array is to being sorted, but is *not* the
minimum number of swaps required to sort the array. If the array is sorted, then the inversion count
is 0. If the array is sorted in decreasing order, then the inversion count is maximal. The following
two functions are each techniques to efficiently count inversions. In the merge sort approach,
whenever the merge step emits an element from the right half, that element jumps ahead of every
unmerged left-half element, and exactly that many inversions are added to the count.

- `inversions(lo, hi)` uses merge sort to return the number of inversions given two random-access
  iterators as a range `[lo, hi)`. The input range will be sorted after the function call. 
  Optionally, a comparison function object specifying a strict weak ordering may be specified to
  replace the default `operator<`.
- `inversions(a)` uses coordinate compression and a Fenwick tree to return the number of
  inversions in an integer vector without modifying it.

Time Complexity:
- O(n log n) per call to `inversions(lo, hi)`, where $n$ is the distance between `lo` and `hi`.
- O(n log n) per call to `inversions(a)`.

Space Complexity:
- O(n) auxiliary space and O(log n) stack space for `inversions(lo, hi)`.
- O(n) auxiliary heap space for `inversions(a)`.

*/

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

template<class It, class Compare = std::less<>>
long long inversions(It lo, It hi, Compare comp = Compare()) {
  if (hi - lo < 2) {
    return 0;
  }
  It mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  long long res = 0;
  res += inversions(lo, mid + 1, comp);
  res += inversions(mid + 1, hi, comp);
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> merged;
  while (a <= mid && c < hi) {
    if (comp(*c, *a)) {
      merged.push_back(*(c++));
      res += (mid - a) + 1;
    } else {
      merged.push_back(*(a++));
    }
  }
  if (a > mid) {
    for (It it = c; it != hi; ++it) {
      merged.push_back(*it);
    }
  } else {
    for (It it = a; it <= mid; ++it) {
      merged.push_back(*it);
    }
  }
  for (It it = lo; it != hi; ++it) {
    *it = merged[it - lo];
  }
  return res;
}

long long inversions(const std::vector<int> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> values(a.begin(), a.end());
  std::sort(values.begin(), values.end());
  values.erase(std::unique(values.begin(), values.end()), values.end());
  std::vector<int> bit(values.size() + 1, 0);
  long long res = 0;
  for (int i = n - 1; i >= 0; i--) {
    int id =
        static_cast<int>(std::lower_bound(values.begin(), values.end(), a[i]) - values.begin()) + 1;
    for (int j = id - 1; j > 0; j -= j & -j) {
      res += bit[j];
    }
    for (int j = id; j < static_cast<int>(bit.size()); j += j & -j) {
      bit[j]++;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  {
    std::vector<int> a{6, 9, 1, 14, 8, 12, 3, 2};
    assert(inversions(a.begin(), a.end()) == 16);
  }
  {
    std::vector<int> a{6, 9, 1, 14, 8, 12, 3, 2};
    assert(inversions(a) == 16);
  }
  return 0;
}
