/*

Partition a range around a pivot, or select the element with a given sorted rank without fully
sorting the range. Three-way partitioning, also known as the Dutch National Flag algorithm,
rearranges the elements into consecutive groups less than, equal to, and greater than the pivot. A
single pass maintains these three groups plus an unclassified region. When an element greater than
the pivot is swapped with the end of the unclassified region, the incoming element must still be
examined before the scan advances.

Quickselect repeatedly applies this partition and continues only into the group containing the
desired rank. This is the same task as `std::nth_element()`: after the call, `*nth` is the value
that would appear there in sorted order, all earlier positions contain values no larger than it, and
all later positions contain values no smaller than it. Choosing pivots uniformly at random gives
expected linear time, while the three-way split avoids unnecessary work on duplicate-heavy inputs.

- `partition_three_way(lo, hi, pivot)` rearranges $[`lo`, `hi`)$ in-place and returns iterators
  $(`mid1`, `mid2`)$, where $[`lo`, `mid1`)$ is less than `pivot`, $[`mid1`, `mid2`)$ is equal to
  `pivot`, and $[`mid2`, `hi`)$ is greater than `pivot`.
- `sort_012(lo, hi)` sorts a range consisting only of the values $0$, $1$, and $2$.
- `nth_element2(lo, nth, hi)` rearranges the range $[`lo`, `hi`)$ around the 0-based rank
  represented by iterator `nth`. This requires random-access iterators and `operator<` on the value
  type.

Time Complexity:
- O(n) per call to `partition_three_way()` or `sort_012()`, where $n$ is the range length.
- O(n) expected per call to `nth_element2()`, and O(n^2) in the worst case.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <iterator>
#include <random>
#include <utility>

template<typename It, typename T>
std::pair<It, It> partition_three_way(It lo, It hi, const T &pivot) {
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
  return {lt, gt};
}

template<typename It>
void sort_012(It lo, It hi) {
  partition_three_way(lo, hi, 1);
}

template<typename It>
void nth_element2(It lo, It nth, It hi) {
  static std::mt19937 rng(std::random_device{}());
  while (hi - lo > 1) {
    std::uniform_int_distribution<int> dist(0, hi - lo - 1);
    auto pivot = *(lo + dist(rng));
    auto [lt, gt] = partition_three_way(lo, hi, pivot);
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
  vector<int> values{2, 0, 1, 2, 1, 0, 1};
  sort_012(values.begin(), values.end());
  for (int i = 0; i < 7; i++) {
    assert(values[i] == (i < 2 ? 0 : i < 5 ? 1 : 2));
  }

  vector<int> b{4, 2, 5, 3, 3, 1};
  auto [mid1, mid2] = partition_three_way(b.begin(), b.end(), 3);
  for (auto it = b.begin(); it != mid1; ++it) {
    assert(*it < 3);
  }
  for (auto it = mid1; it != mid2; ++it) {
    assert(*it == 3);
  }
  for (auto it = mid2; it != b.end(); ++it) {
    assert(*it > 3);
  }

  vector<int> a{5, 6, 4, 3, 2, 6, 7, 9, 3};
  int n = static_cast<int>(a.size());
  nth_element2(a.begin(), a.begin() + n / 2, a.end());
  assert(a[n / 2] == 5);
  // Values left of the median are <=, and values right are >= (the exact order within each side is
  // randomized, since the pivot is chosen at random).
  for (int i = 0; i < n / 2; i++) {
    assert(a[i] <= a[n / 2]);
  }
  for (int i = n / 2 + 1; i < n; i++) {
    assert(a[i] >= a[n / 2]);
  }
  return 0;
}
