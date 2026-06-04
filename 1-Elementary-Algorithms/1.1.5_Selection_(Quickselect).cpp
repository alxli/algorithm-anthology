/*

`nth_element2()` is equivalent to `std::nth_element()`, taking random-access iterators `lo`, `nth`,
and `hi` as the range `[lo, hi)` to be partially sorted. The values in `[lo, hi)` are rearranged
such that the value pointed to by `nth` is the element that would be there if the range were sorted.
Furthermore, the range is partitioned such that no value in `[lo, nth)` compares greater than the
value pointed to by `nth` and no value in `(nth, hi)` compares less. This implementation requires
`operator <` to be defined on the iterator's value type.

The pivot is chosen uniformly at random, and the partition step groups equal keys together so arrays
with many duplicate values remain efficient.

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
  static std::mt19937 rng(1);
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

/*** Example Usage and Output:

2 3 3 4 5 9 6 7 6

***/

#include <cassert>
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
  vector<int> a{5, 6, 4, 3, 2, 6, 7, 9, 3};
  int n = a.size();
  nth_element2(a.begin(), a.begin() + n / 2, a.end());
  assert(a[n / 2] == 5);
  print_range(a.begin(), a.end());
  return 0;
}
