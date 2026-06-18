/*

Select the element with a given sorted rank without fully sorting the range. This is the same task
as `std::nth_element()`: after the call, `*nth` is the value that would appear at position `nth` in
sorted order, all earlier positions contain values no larger than it, and all later positions
contain values no smaller than it. The order within each side is unspecified.

Quickselect repeatedly partitions around a pivot, then continues only into the side containing the
desired rank. This implementation chooses the pivot uniformly at random and uses a 3-way partition
(`less than`, `equal to`, `greater than`) so duplicate-heavy inputs do not cause unnecessary work.

- `nth_element2(lo, nth, hi)` rearranges the range [`lo`, `hi`) around the 0-based rank represented
  by iterator `nth`. This requires random-access iterators and `operator<` on the value type.

Time Complexity:
- O(n) expected per call to `nth_element2()`, where $n$ is the distance between `lo` and `hi`.
- O(n^2) in the worst case.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <iterator>
#include <random>

template<typename It>
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
