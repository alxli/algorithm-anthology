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
- `quickselect(lo, nth, hi, comp = std::less<>())` rearranges $[`lo`, `hi`)$ in-place around the
  0-based rank represented by iterator `nth` according to `comp`. This requires random-access
  iterators.

Random pivots leave an adversary free to force O(n^2), which the median-of-medians rule removes.
Split the range into groups of five, sort each, and pivot on the median of the group medians: at
least three elements in half of the groups precede it, so every partition discards at least three
tenths of the range. Finding that median is itself a selection, solved by recursing on the $n/5$
medians gathered to the front, and $T(n) = T(n/5) + T(7n/10) + O(n)$ is linear because the two
fractions sum to less than one, which five is the smallest group size to achieve. Each group is
sorted directly, since at five elements `std::sort` is an inlined insertion sort. This guarantee is
not free, but on random input the cost over a random pivot is modest.

- `quickselect_linear(lo, nth, hi, comp = std::less<>())` does the same in linear time even in the
  worst case, choosing each pivot by the median-of-medians rule instead of at random.

Time Complexity:
- O(n) per call to `partition_three_way()`, where $n$ is the range length.
- O(n) expected per call to `quickselect()`, and O(n^2) in the worst case.
- O(n) per call to `quickselect_linear()` in all cases.

Space Complexity:
- O(1) auxiliary for `partition_three_way()` and `quickselect()`.
- O(log n) auxiliary stack space for `quickselect_linear()`.

*/

#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
#include <random>
#include <utility>

template<typename It, typename T, typename Compare = std::less<>>
std::pair<It, It> partition_three_way(It lo, It hi, const T &pivot, Compare comp = Compare{}) {
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
void quickselect(It lo, It nth, It hi, Compare comp = Compare{}) {
  using T = typename std::iterator_traits<It>::value_type;
  static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  while (hi - lo > 1) {
    std::uniform_int_distribution<int> dist(0, hi - lo - 1);
    T pivot = *(lo + dist(rng));
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

template<typename It, typename Compare = std::less<>>
void quickselect_linear(It lo, It nth, It hi, Compare comp = Compare{}) {
  while (hi - lo > 5) {
    int groups = 0;
    for (It g = lo; g < hi; g += 5, groups++) {
      It ge = hi - g < 5 ? hi : g + 5;
      std::sort(g, ge, comp);  // At most 5 elements, so this is an O(1) step.
      std::iter_swap(lo + groups, g + (ge - g) / 2);
    }
    It mid = lo + groups / 2;
    quickselect_linear(lo, mid, lo + groups, comp);
    using T = typename std::iterator_traits<It>::value_type;
    T pivot = *mid;
    std::pair<It, It> parts = partition_three_way(lo, hi, pivot, comp);
    if (nth < parts.first) {
      hi = parts.first;
    } else if (nth < parts.second) {
      return;
    } else {
      lo = parts.second;
    }
  }
  std::sort(lo, hi, comp);
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
  assert(all_of(b.begin(), mid1, [](int x) { return x < 3; }));
  assert(all_of(mid1, mid2, [](int x) { return x == 3; }));
  assert(all_of(mid2, b.end(), [](int x) { return x > 3; }));

  vector<int> a{5, 6, 4, 3, 2, 6, 7, 9, 3};
  int n = static_cast<int>(a.size());
  quickselect(a.begin(), a.begin() + n / 2, a.end());
  assert(a[n / 2] == 5);
  // Values left of the median are <=, and values right are >= (the exact order within each side is
  // randomized, since the pivot is chosen at random).
  assert(all_of(a.begin(), a.begin() + n / 2, [&](int x) { return x <= a[n / 2]; }));
  assert(all_of(a.begin() + n / 2 + 1, a.end(), [&](int x) { return x >= a[n / 2]; }));

  // The median-of-medians variant gives the same answer with a worst case that is still linear.
  vector<int> c{5, 6, 4, 3, 2, 6, 7, 9, 3};
  quickselect_linear(c.begin(), c.begin() + n / 2, c.end());
  assert(c[n / 2] == 5);

  vector<int> desc{1, 4, 2, 3};
  quickselect(desc.begin(), desc.begin() + 1, desc.end(), greater<int>());
  assert(desc[1] == 3);
  return 0;
}
