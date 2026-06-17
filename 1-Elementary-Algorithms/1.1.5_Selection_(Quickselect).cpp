/*

Partially sort a range so that one chosen position ends up holding its final sorted element, like
`std::nth_element()`.

- `nth_element2(lo, nth, hi)` rearranges the range [`lo`, `hi`) such that the value pointed to by
  `nth` is the element that would be there if the range were fully sorted. Furthermore, the range is
  partitioned such that no value in [`lo`, `nth`) compares greater than the value pointed to by
  `nth` and no value in (`nth`, `hi`) compares less. This requires `operator<` defined on the
  iterator's value type.

The pivot is chosen uniformly at random, and the partition step groups equal keys together so arrays
with many duplicate values remain efficient. After each partition, the search continues in only the
side that contains `nth`, shrinking the expected remaining work geometrically for a linear average.

Time Complexity:
- O(n) on average per call to `nth_element2()`, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <iterator>
#include <random>

template<class It>
void nth_element2(It lo, It nth, It hi) {
  static std::mt19937 rng(std::random_device{}());
  while (hi - lo > 1) {
    std::uniform_int_distribution<int> dist(0, hi - lo - 1);
    auto pivot = *(lo + dist(rng));
    It lt = lo, i = lo, gt = hi;
    while (i != gt) {
      if (*i < pivot) {
        std::iter_swap(lt++, i++);
      } else if (pivot < *i) {
        std::iter_swap(i, --gt);
      } else {
        ++i;
      }
    }
    if (nth < lt) {
      hi = lt;
    } else if (nth >= gt) {
      lo = gt;
    } else {
      return;
    }
  }
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<int> a{5, 6, 4, 3, 2, 6, 7, 9, 3};
  int n = static_cast<int>(a.size());
  nth_element2(a.begin(), a.begin() + n / 2, a.end());
  assert(a[n / 2] == 5);
  // Values left of the median are <= it and values right are >= it (the exact
  // order within each side is randomized, since the pivot is chosen at random).
  for (int i = 0; i < n / 2; i++) {
    assert(a[i] <= a[n / 2]);
  }
  for (int i = n / 2 + 1; i < n; i++) {
    assert(a[i] >= a[n / 2]);
  }
  return 0;
}
