/*

The number of inversions for a sequence $a$ is the number of ordered pairs $(i, j)$ such that
$i < j$ and $a[i] > a[j]$. This is roughly how "close" an array is to being sorted, and is exactly
the number of exchanges a sort makes when only *adjacent* elements may be swapped. It is *not* the
minimum number of swaps to sort when any two elements may be exchanged, which is $n$ minus the
number of cycles in the permutation; see `min_swaps_to_sort()` in section 6.2.3. If the array is
sorted, then the inversion count is $0$. If the array is sorted in decreasing order, then the
inversion count is maximal ($\binom{n}{2}$ when the values are distinct).

The following two functions are each techniques to efficiently count inversions. In the merge sort
approach, whenever the merge step emits an element from the right half, that element jumps ahead of
every unmerged left-half element, and exactly that many inversions are added to the count. The
merging below proceeds bottom-up over runs of doubling width, which needs only a single scratch
buffer for the whole sort instead of one per level of recursion.

- `inversions(lo, hi, comp = std::less<>)` uses merge sort to return the number of inversions given
  two random-access iterators as a range $[`lo`, `hi`)$. The input range will be sorted after the
  function call. Optionally, a comparison function object specifying a strict weak ordering may be
  specified to replace the default `operator<`.
- `inversions(a)` uses coordinate compression and a Fenwick tree to return the number of inversions
  in an integer vector without modifying it.

Overflow warning: The range length and doubled merge widths must fit in `int`.

Time Complexity:
- O(n log n) per call to `inversions(lo, hi)`, where $n$ is the distance between `lo` and `hi`.
- O(n log n) per call to `inversions(a)`.

Space Complexity:
- O(n) auxiliary heap space for both functions.

*/

#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <vector>

template<typename It, typename Compare = std::less<>>
int64_t inversions(It lo, It hi, Compare comp = Compare{}) {
  using T = typename std::iterator_traits<It>::value_type;
  int n = static_cast<int>(hi - lo);
  std::vector<T> buf(n);
  int64_t res = 0;
  for (int width = 1; width < n; width *= 2) {
    for (int i = 0; i + width < n; i += 2 * width) {
      int mid = i + width, end = std::min(i + 2 * width, n);
      int a = i, c = mid, k = i;
      while (a < mid && c < end) {
        if (comp(lo[c], lo[a])) {
          buf[k++] = lo[c++];
          res += mid - a;
        } else {
          buf[k++] = lo[a++];
        }
      }
      while (a < mid) {
        buf[k++] = lo[a++];
      }
      while (c < end) {
        buf[k++] = lo[c++];
      }
      std::copy(buf.begin() + i, buf.begin() + end, lo + i);
    }
  }
  return res;
}

int64_t inversions(const std::vector<int> &a) {
  int n = static_cast<int>(a.size());
  std::vector<int> v(a);
  std::sort(v.begin(), v.end());
  v.resize(std::unique(v.begin(), v.end()) - v.begin());
  std::vector<int> bit(v.size() + 1);
  int64_t res = 0;
  for (int i = n - 1; i >= 0; i--) {
    int id = static_cast<int>(std::lower_bound(v.begin(), v.end(), a[i]) - v.begin()) + 1;
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
using namespace std;

int main() {
  {
    vector<int> a{6, 9, 1, 14, 8, 12, 3, 2};
    assert(inversions(a.begin(), a.end()) == 16);
    assert(is_sorted(a.begin(), a.end()));
  }
  {
    vector<int> a{6, 9, 1, 14, 8, 12, 3, 2};
    assert(inversions(a) == 16);
    assert((a == vector<int>{6, 9, 1, 14, 8, 12, 3, 2}));
  }
  {
    vector<int> a{2, 2, 1};
    assert(inversions(a) == 2);  // Equal elements are not inversions.
  }
  return 0;
}
