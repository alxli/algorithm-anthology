/*

Partition a range around a pivot, or select the element with a given sorted rank without fully
sorting the range. A comparator `comp` defines the ordering (defaults to `std::less<>`). Three-way
partitioning rearranges the elements into consecutive groups that precede, are equivalent to, and
follow the pivot. `std::partition()` can be used to place elements satisfying a unary predicate
before those that do not, but forms only two groups and does not preserve relative order. Forming
three groups can be done by first partitioning by `comp(x, pivot)`, then partitioning the remaining
suffix by `!comp(pivot, x)`; using `std::stable_partition()` for both calls preserves relative order
within each group.

`partition_three_way()` implements the Dutch National Flag algorithm, forming the three groups in
one pass with O(1) auxiliary space. At every iteration, $[`lo`, `lt`)$ contains elements preceding
the pivot, $[`lt`, `cur`)$ contains equivalent elements, $[`cur`, `gt`)$ remains unclassified, and
$[`gt`, `hi`)$ contains elements following the pivot. The current element `*cur` is compared with
the pivot. If it precedes the pivot, it is swapped with `*lt` and both `lt` and `cur` advance. If it
is equivalent, only `cur` advances. Otherwise, it is swapped with `*--gt` without advancing `cur`,
since the incoming element remains unclassified.

Quickselect repeatedly applies this partition and continues only into the group containing the
desired rank. This is the same task as `std::nth_element()`: after the call, `*nth` is the value
that would appear there in comparator order, no earlier value follows it, and no later value
precedes it. Choosing pivots uniformly at random gives expected linear time, while the three-way
split avoids unnecessary work on duplicate-heavy inputs.

- `partition_three_way(lo, hi, pivot, comp = std::less<>())` rearranges $[`lo`, `hi`)$ in-place and
  returns a pair of iterators (`mid1`, `mid2`). The three resulting ranges contain elements that
  precede, are equivalent to, and follow `pivot` according to `comp`, respectively. The resulting
  partition is not stable.
- `nth_element2(lo, nth, hi, comp = std::less<>())` rearranges $[`lo`, `hi`)$ in-place around the
  0-based rank represented by iterator `nth` according to `comp`. This requires random-access
  iterators.

Time Complexity:
- O(n) per call to `partition_three_way()`, where $n$ is the range length.
- O(n) expected per call to `nth_element2()`, and O(n^2) in the worst case.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <functional>
#include <random>
#include <utility>

template<typename It, typename T, typename Compare = std::less<>>
std::pair<It, It> partition_three_way(It lo, It hi, const T &pivot, Compare comp = Compare()) {
  It lt = lo, cur = lo, gt = hi;
  while (cur != gt) {
    if (comp(*cur, pivot)) {
      std::iter_swap(lt++, cur++);
    } else if (comp(pivot, *cur)) {
      std::iter_swap(cur, --gt);
    } else {
      ++cur;
    }
  }
  return {lt, gt};
}

template<typename It, typename Compare = std::less<>>
void nth_element2(It lo, It nth, It hi, Compare comp = Compare()) {
  static std::mt19937 rng(std::random_device{}());
  while (hi - lo > 1) {
    std::uniform_int_distribution<int> dist(0, hi - lo - 1);
    auto pivot = *(lo + dist(rng));
    auto [lt, gt] = partition_three_way(lo, hi, pivot, comp);
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
  // On a range containing only 0, 1, and 2, partitioning around 1 sorts the range.
  partition_three_way(values.begin(), values.end(), 1);
  assert(is_sorted(values.begin(), values.end()));

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

  vector<int> descending{1, 4, 2, 3};
  nth_element2(descending.begin(), descending.begin() + 1, descending.end(), greater<int>());
  assert(descending[1] == 3);
  return 0;
}
