/*

Partitions a range into three consecutive groups according to a pivot value: elements less than the
pivot, elements equal to the pivot, and elements greater than the pivot. This is the Dutch National
Flag algorithm, and is the core partitioning step used in three-way quicksort and arrays with many
duplicate keys. A single pass maintains the three groups in-place using two boundary iterators,
swapping each examined element into the region where it belongs.

- `partition_three_way(lo, hi, pivot)` rearranges the range [`lo`, `hi`) in-place and returns
  iterators (`mid_lo`, `mid_hi`), where [`lo`, `mid_lo`) is less than `pivot`, [`mid_lo`, `mid_hi`)
  is equal to `pivot`, and [`mid_hi`, `hi`) is greater than `pivot`.
- `sort_012(lo, hi)` sorts a range consisting of only the values 0, 1, and 2.

Time Complexity:
- O(n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
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

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<int> a{2, 0, 1, 2, 1, 0, 1};
  sort_012(a.begin(), a.end());
  for (int i = 0; i < 7; i++) {
    assert(a[i] == (i < 2 ? 0 : i < 5 ? 1 : 2));
  }

  vector<int> b{4, 2, 5, 3, 3, 1};
  auto [mid_lo, mid_hi] = partition_three_way(b.begin(), b.end(), 3);
  for (auto it = b.begin(); it != mid_lo; ++it) {
    assert(*it < 3);
  }
  for (auto it = mid_lo; it != mid_hi; ++it) {
    assert(*it == 3);
  }
  for (auto it = mid_hi; it != b.end(); ++it) {
    assert(*it > 3);
  }
  return 0;
}
