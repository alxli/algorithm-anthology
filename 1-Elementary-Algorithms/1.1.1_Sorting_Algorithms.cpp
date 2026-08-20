/*

The following functions are equivalent to `std::sort()`, taking random-access iterators as a
half-open range $[`lo`, `hi`)$ to be sorted. The range is sorted into ascending order after the
function call. Optionally, a comparison function object specifying a strict weak ordering may be
specified to replace the default `operator<`.

- `quicksort(lo, hi, comp = std::less<>)` sorts the range using quicksort.
- `mergesort(lo, hi, comp = std::less<>)` sorts the range using merge sort, which is stable.
- `heapsort(lo, hi, comp = std::less<>)` sorts the range using heapsort.
- `combsort(lo, hi, comp = std::less<>)` sorts the range using comb sort.
- `insertion_sort(lo, hi, comp = std::less<>)` sorts the range using insertion sort, which is
  stable.
- `shellsort(lo, hi, comp = std::less<>)` sorts the range using shell sort.
- `counting_sort(lo, hi, keys, key)` sorts the range by the index that `key(element)` returns in
  $[0, `keys`)$, which is stable; unlike the shared interface above, it takes no comparator.
- `radix_sort(lo, hi)` sorts an integer range using least-significant-byte radix sort; unlike the
  shared interface above, it takes no comparator.
- `bitonic_sort(lo, hi, comp = std::less<>)` sorts a range of any length using a compare-exchange
  sequence generated from that length.

These functions are not meant to compete with standard library implementations in terms of speed.
Instead, they demonstrate how common sorting algorithms can be concisely implemented in C++.

*/

#include <algorithm>
#include <climits>
#include <functional>
#include <iterator>
#include <type_traits>
#include <vector>

/*

Quicksort repeatedly selects a pivot and partitions the range so that elements comparing less than
the pivot precede it, elements comparing equal stay in the middle, and elements comparing greater
follow it. Divide and conquer is then applied to the two outer ranges until the original range is
sorted. The swaps used during partitioning make quicksort unstable. Despite having a worst case of
O(n^2), quicksort is often faster in practice than merge sort and heapsort, which both have a worst
case time complexity of O(n log n).

The pivot chosen in this implementation is always a middle element of the range to be sorted.
Shuffling the input first or choosing each pivot randomly makes consistently unbalanced partitions
unlikely. Median-of-three is another practical choice, while median-of-medians can guarantee a
balanced pivot at greater constant-factor cost.

Time Complexity: O(n) best (all equal), O(n log n) average, and O(n^2) worst.
Space Complexity: O(log n) auxiliary stack space.

*/

template<typename It, typename Compare = std::less<>>
void quicksort(It lo, It hi, Compare comp = Compare{}) {
  using T = typename std::iterator_traits<It>::value_type;
  while (hi - lo >= 2) {
    T pivot = *(lo + (hi - lo) / 2);
    It lt = lo, mid = lo, gt = hi;
    while (mid != gt) {
      if (comp(*mid, pivot)) {
        std::iter_swap(lt++, mid++);
      } else if (comp(pivot, *mid)) {
        std::iter_swap(mid, --gt);
      } else {
        ++mid;
      }
    }
    if (lt - lo < hi - gt) {
      quicksort(lo, lt, comp);
      lo = gt;
    } else {
      quicksort(gt, hi, comp);
      hi = lt;
    }
  }
}

/*

Merge sort first divides a list into $n$ sublists of one element each, then recursively merges the
sublists into sorted order until only a single sorted sublist remains. Merge sort is a stable sort,
meaning that it preserves the relative order of elements which compare equal by `operator<` or the
custom comparator given.

An analogous function in the C++ standard library is `std::stable_sort()`, except that the
implementation here requires sufficient memory to be available. When O(n) auxiliary memory is not
available, `std::stable_sort()` falls back to a time complexity of O(n log^2 n) whereas the
implementation here will simply fail.

Time Complexity: O(n log n) in all cases.
Space Complexity: O(log n) auxiliary stack space and O(n) auxiliary heap space.

*/

template<typename It, typename Compare = std::less<>>
void mergesort(It lo, It hi, Compare comp = Compare{}) {
  if (hi - lo < 2) {
    return;
  }
  It mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  mergesort(lo, mid + 1, comp);
  mergesort(mid + 1, hi, comp);
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> merged;
  merged.reserve(hi - lo);
  while (a <= mid && c < hi) {
    merged.push_back(comp(*c, *a) ? *c++ : *a++);
  }
  merged.insert(merged.end(), a, mid + 1);
  merged.insert(merged.end(), c, hi);
  std::copy(merged.begin(), merged.end(), lo);
}

/*

Heapsort first rearranges an array to satisfy the max-heap property. Then, it repeatedly pops the
max element of the heap (the left, unsorted subrange), moving it to the beginning of the right,
sorted subrange until the entire range is sorted. Heapsort has a better worst case time complexity
than quicksort and also a better space complexity than merge sort, but its swaps make it unstable.

The C++ standard library equivalent is calling `std::make_heap(lo, hi)`, followed by
`std::sort_heap(lo, hi)`.

`sift_down()` restores the heap property below one node. The first loop applies it to each non-leaf
from right to left, which builds the heap in O(n) time. The second loop repeatedly moves the maximum
to the back of the unsorted range and restores the heap among the remaining elements.

Time Complexity: O(n log n) in all cases.
Space Complexity: O(1) auxiliary.

*/

template<typename It, typename Compare = std::less<>>
void heapsort(It lo, It hi, Compare comp = Compare{}) {
  if (hi - lo < 2) {
    return;
  }
  using T = typename std::iterator_traits<It>::value_type;
  auto sift_down = [&](It root, It end) {
    T value = *root;
    auto parent = root - lo, n = end - lo;
    while (2 * parent + 1 < n) {
      auto child = 2 * parent + 1;
      if (child + 1 < n && comp(*(lo + child), *(lo + child + 1))) {
        child++;
      }
      if (!comp(value, *(lo + child))) {
        break;
      }
      *(lo + parent) = *(lo + child);
      parent = child;
    }
    *(lo + parent) = value;
  };
  for (It root = lo + (hi - lo) / 2; root != lo;) {
    sift_down(--root, hi);
  }
  for (It end = hi; --end != lo;) {
    std::iter_swap(lo, end);
    sift_down(lo, end);
  }
}

/*

Comb sort is an improved bubble sort. While bubble sort compares only adjacent elements, comb sort
compares elements separated by a fixed gap, decreasing that gap after every pass. Large gaps move
small values near the end toward the front quickly, avoiding the slow movement of such values in
bubble sort. These nonadjacent swaps make comb sort unstable. The shrink factor $1.3$ is a common
empirical choice.

Time Complexity: O(n log n) best and O(n^2) worst.
Space Complexity: O(1) auxiliary.

*/

template<typename It, typename Compare = std::less<>>
void combsort(It lo, It hi, Compare comp = Compare{}) {
  int gap = static_cast<int>(hi - lo);
  bool swapped = true;
  while (gap > 1 || swapped) {
    if (gap > 1) {
      gap = gap * 10 / 13;
    }
    swapped = false;
    for (It it = lo; it + gap < hi; ++it) {
      if (comp(*(it + gap), *it)) {
        std::iter_swap(it, it + gap);
        swapped = true;
      }
    }
  }
}

/*

Insertion sort builds the sorted range one element at a time. It scans left to right, and for each
element shifts the larger elements of the already-sorted prefix one position to the right to open a
slot where the element belongs. Although its average and worst cases are quadratic, it is simple,
stable, in-place, and online (it can sort a stream as elements arrive).

Its strength is being adaptive: on an already-sorted or nearly-sorted range, each element travels
only a short distance, giving O(n) best-case time and O(n + d) in general for $d$ total inversions.
This is why it outperforms the asymptotically faster sorts on small or nearly-sorted ranges, and why
hybrid sorts such as introsort and Timsort fall back to it once a subrange is small enough.

The comparison `comp(key, *(j - 1))` is strict, so an element never moves past an earlier element it
compares equal to, which keeps the sort stable.

Time Complexity: O(n + d), i.e. O(n) best and O(n^2) average/worst.
Space Complexity: O(1) auxiliary.

*/

template<typename It, typename Compare = std::less<>>
void insertion_sort(It lo, It hi, Compare comp = Compare{}) {
  using T = typename std::iterator_traits<It>::value_type;
  for (It i = lo; i != hi; ++i) {
    T key = *i;
    It j = i;
    while (j != lo && comp(key, *(j - 1))) {
      *j = *(j - 1);
      --j;
    }
    *j = key;
  }
}

/*

Shell sort is insertion sort performed on subsequences of elements a fixed gap apart, repeating with
smaller gaps until the final pass uses a gap of one and is an ordinary insertion sort. The earlier
passes are what make that final pass cheap: each one leaves the range closer to sorted, and
insertion sort runs in time proportional to the number of remaining inversions. Comparing elements a
gap apart also moves a value many positions in one step, which is what insertion sort alone cannot
do. Like comb sort above, the nonadjacent swaps make it unstable.

The gap sequence determines the bound, and choosing it well is the whole difficulty. This
implementation uses Ciura's empirical sequence, extended upward by a factor of $2.25$, which is the
fastest known for small and medium ranges though it has no proven bound. The theoretical choices are
Sedgewick's sequence at O(n^{4/3}) and Pratt's at O(n log^2 n), the latter being slower in practice
despite the better bound.

Time Complexity: O(n) best and O(n^2) worst (though the chosen gaps are much faster on average).
Space Complexity: O(1) auxiliary.

*/

template<typename It, typename Compare = std::less<>>
void shellsort(It lo, It hi, Compare comp = Compare{}) {
  static const int GAPS[] = {1, 4, 10, 23, 57, 132, 301, 701, 1577, 3548, 7983, 17962};
  int n = static_cast<int>(hi - lo), count = sizeof(GAPS) / sizeof(GAPS[0]);
  for (int g = count - 1; g >= 0; g--) {
    int gap = GAPS[g];
    if (gap >= n) {
      continue;
    }
    for (It it = lo + gap; it < hi; ++it) {
      typename std::iterator_traits<It>::value_type key = *it;
      It j = it;
      while (j - lo >= gap && comp(key, *(j - gap))) {
        *j = *(j - gap);
        j -= gap;
      }
      *j = key;
    }
  }
}

/*

Counting sort sorts values from a small integer range by counting how many times each value occurs,
converting those counts into starting offsets with a prefix sum, and then placing each element at
the offset for its key. It never compares two elements, so the O(n log n) lower bound for comparison
sorts does not apply, and it runs in linear time whenever the range of keys is proportional to the
number of elements.

Placing elements in a second pass over the input from right to left keeps the sort stable, which is
what makes it usable as the inner pass of the radix sort below. The `key` function maps an element
to an index in $[0, `keys`)$, so records can be sorted by one field while carrying the rest.

Time Complexity: O(n + k) for $n$ elements with $k$ possible keys.
Space Complexity: O(n + k) auxiliary.

*/

template<typename It, typename KeyFn>
void counting_sort(It lo, It hi, int keys, KeyFn key) {
  if (hi - lo < 2) {
    return;
  }
  std::vector<int> count(keys);
  for (It it = lo; it != hi; ++it) {
    count[key(*it)]++;
  }
  for (int i = 1; i < keys; ++i) {
    count[i] += count[i - 1];
  }
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> res(hi - lo);
  for (It it = hi; it != lo;) {
    --it;
    res[--count[key(*it)]] = *it;
  }
  std::copy(res.begin(), res.end(), lo);
}

/*

Radix sort is used to sort integer elements with a constant number of bits in linear time. This
implementation works on ranges pointing to any signed or unsigned integer primitive. Signed values
are handled by sorting on a key that flips the sign bit, which maps the two's-complement order onto
the unsigned order so the most negative value sorts first.

Digits are processed from least significant to most significant, and each counting-sort pass is
stable. After one pass, the values are sorted by the processed digit; stability ensures that a later
pass preserves the ordering already established by all less-significant digits. Inductively, after
the final pass the values are sorted by the entire key.

This implementation uses one byte per digit, so digits can be extracted with shifts and masks and
the counting table has $2^8 = 256$ entries.

Time Complexity: O(n*w) for $n$ integers of $w$ bits each.
Space Complexity: O(n + 2^b) auxiliary for a radix of $b$ bits, i.e. O(n) for constant $b$.

*/

template<typename It>
void radix_sort(It lo, It hi) {
  if (hi - lo < 2) {
    return;
  }
  const int radix_bits = 8;
  const int radix_base = 1 << radix_bits;  // e.g. 2^8 = 256
  const int radix_mask = radix_base - 1;   // e.g. 2^8 - 1 = 0xFF
  using T = typename std::iterator_traits<It>::value_type;
  static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>);
  using U = typename std::make_unsigned_t<T>;
  const int num_bits = sizeof(T) * CHAR_BIT;
  // Sort on an unsigned key. For signed types, flipping the sign bit sends the most negative value
  // to 0, mapping the signed order onto the unsigned order; logical shifts then extract each digit.
  auto key = [](T x) -> U {
    U u = static_cast<U>(x);
    return std::is_signed<T>::value ? (u ^ (U{1} << (num_bits - 1))) : u;
  };
  std::vector<T> buf(hi - lo);
  for (int pos = 0; pos < num_bits; pos += radix_bits) {
    std::vector<int> count(radix_base);
    for (It it = lo; it != hi; ++it) {
      count[(key(*it) >> pos) & radix_mask]++;
    }
    std::vector<T *> bucket(radix_base);
    T *curr = buf.data();
    for (int i = 0; i < radix_base; curr += count[i++]) {
      bucket[i] = curr;
    }
    for (It it = lo; it != hi; ++it) {
      *bucket[(key(*it) >> pos) & radix_mask]++ = *it;
    }
    std::copy(buf.begin(), buf.end(), lo);
  }
}

/*

Bitonic sort is a sorting network: a fixed sequence of compare-exchanges on fixed positions, each
swapping its pair when out of order. The sequence depends only on how many elements there are, never
on what they are, so no comparison branches on data. It sorts the first half ascending and the
second half descending, leaving a bitonic sequence that rises and then falls, which a fixed pattern
of halving compare-exchanges merges. Splitting each merge at the largest power of two below its
length is what admits lengths that are not powers of two, which the textbook formulation requires.

Its O(n log^2 n) comparators exceed the O(n log n) comparisons of a comparison sort, so run one at a
time it loses to every sort above, as the benchmark below shows. What it buys instead is that each
group of comparators is independent and may run at once, which is why networks are the standard
choice across SIMD lanes and on a GPU. The same independence pays off at a small size fixed at
compile time: unrolled into branchless minimum and maximum, a network sorts eight integers about an
order of magnitude faster than a general sort. Driving that same comparator sequence from a lookup
table forfeits it, since each comparator then costs an indexed load and an unpredictable branch.

Time Complexity: O(n log^2 n) in all cases.
Space Complexity: O(log n) auxiliary stack space.

*/

template<typename It, typename Compare = std::less<>>
void bitonic_sort(It lo, It hi, Compare comp = Compare{}) {
  auto merge = [&](auto &&merge, It lo, int n, bool up) {
    if (n < 2) {
      return;
    }
    int m = 1;
    while (2 * m < n) {
      m *= 2;
    }
    for (int i = 0; i < n - m; i++) {
      if (comp(*(lo + i + m), *(lo + i)) == up) {
        std::iter_swap(lo + i, lo + i + m);
      }
    }
    merge(merge, lo, m, up);
    merge(merge, lo + m, n - m, up);
  };
  auto rec = [&](auto &&rec, It lo, int n, bool up) {
    if (n < 2) {
      return;
    }
    rec(rec, lo, n / 2, !up);
    rec(rec, lo + n / 2, n - n / 2, up);
    merge(merge, lo, n, up);
  };
  rec(rec, lo, static_cast<int>(hi - lo), true);
}

/*** Example Usage and Output:

mergesort() with default comparisons: 1.32 1.41 1.62 1.73 2.58 2.72 3.14 4.67
mergesort() with integer comparisons: 1.41 1.73 1.32 1.62 2.72 2.58 3.14 4.67
------
Sorting five million integers...
std::sort():      0.285s
quicksort():      0.325s
mergesort():      0.517s
heapsort():       0.509s
combsort():       0.469s
shellsort():      0.574s
counting_sort():  0.044s
radix_sort():     0.024s
bitonic_sort():   1.040s

***/

#include <cassert>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <utility>
#include <vector>
using namespace std;

template<typename It>
void print_range(It lo, It hi) {
  while (lo != hi) {
    cout << *lo++ << " ";
  }
  cout << endl;
}

int main() {
  {  // Can be used to sort arrays like std::sort().
    int a[]{32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(begin(a), end(a));
    assert(is_sorted(begin(a), end(a)));
  }
  {  // STL containers work too.
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
  }
  {  // Reverse iterators work as expected.
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33};
    heapsort(a.rbegin(), a.rend());
    assert(is_sorted(a.rbegin(), a.rend()));
  }
  {  // Insertion sort is adaptive: an already-sorted range is left untouched in O(n).
    vector<int> a{12, 26, 32, 33, 45, 53, 71, 80};
    insertion_sort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
  }
  {  // We can sort doubles just as well.
    vector<double> a{1.1, -5.0, 6.23, 4.123, 155.2};
    combsort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
  }
  {  // Shell sort moves values far in one step, so it is not adaptive but is nearly in-place.
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33};
    shellsort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
  }
  {  // Counting sort keys records by one field, and equal keys keep their input order.
    vector<pair<int, char>> a{{2, 'a'}, {0, 'b'}, {2, 'c'}, {1, 'd'}, {0, 'e'}};
    counting_sort(a.begin(), a.end(), 3, [](const pair<int, char> &x) { return x.first; });
    vector<pair<int, char>> expected{{0, 'b'}, {0, 'e'}, {1, 'd'}, {2, 'a'}, {2, 'c'}};
    assert(a == expected);
  }
  {  // radix_sort() handles signed integers (including negatives), unlike a plain counting sort.
    vector<int> a{32, -71, 12, -45, 26, -80, 53, 33};
    radix_sort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
  }
  {  // Bitonic sort is oblivious too, and takes any length rather than only a power of two.
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33, 19};
    bitonic_sort(a.begin(), a.end());
    assert(is_sorted(a.begin(), a.end()));
  }
  {  // Empty and singleton ranges are valid no-ops.
    vector<int> empty, single{42};
    quicksort(empty.begin(), empty.end());
    mergesort(empty.begin(), empty.end());
    heapsort(empty.begin(), empty.end());
    insertion_sort(empty.begin(), empty.end());
    combsort(empty.begin(), empty.end());
    radix_sort(empty.begin(), empty.end());
    bitonic_sort(empty.begin(), empty.end());
    mergesort(single.begin(), single.end());
    assert(empty.empty() && single[0] == 42);
  }

  // Stable sort.
  const vector<double> a{3.14, 1.41, 2.72, 4.67, 1.73, 1.32, 1.62, 2.58};
  {
    vector<double> v(a);
    cout << "mergesort() with default comparisons: ";
    mergesort(v.begin(), v.end());
    print_range(v.begin(), v.end());
  }
  {
    vector<double> v(a);
    cout << "mergesort() with integer comparisons: ";
    mergesort(v.begin(), v.end(), [](double i, double j) {
      return static_cast<int>(i) < static_cast<int>(j);
    });
    print_range(v.begin(), v.end());
  }
  cout << "------" << endl;

  mt19937 rng(1234567);  // Fixed seed for reproducibility.
  vector<int> data(5000000);
  const int maxval = 10000000;
  for (int &x : data) {
    x = static_cast<int>(rng() % maxval);  // limit magnitude for counting sort
  }
  cout << "Sorting five million integers..." << endl;
  cout.precision(3);
  auto benchmark = [&](const string &name, auto sort) {
    vector<int> v = data;
    clock_t start = clock();
    sort(v.begin(), v.end());
    double t = static_cast<double>(clock() - start) / CLOCKS_PER_SEC;
    cout << setw(18) << left << name + "(): " << fixed << t << "s" << endl;
    assert(is_sorted(v.begin(), v.end()));
  };
  benchmark("std::sort", [](auto lo, auto hi) { sort(lo, hi); });
  benchmark("quicksort", [](auto lo, auto hi) { quicksort(lo, hi); });
  benchmark("mergesort", [](auto lo, auto hi) { mergesort(lo, hi); });
  benchmark("heapsort", [](auto lo, auto hi) { heapsort(lo, hi); });
  benchmark("combsort", [](auto lo, auto hi) { combsort(lo, hi); });
  benchmark("shellsort", [](auto lo, auto hi) { shellsort(lo, hi); });
  benchmark("counting_sort", [](auto lo, auto hi) {
    counting_sort(lo, hi, maxval, [](int x) { return x; });
  });
  benchmark("radix_sort", [](auto lo, auto hi) { radix_sort(lo, hi); });
  benchmark("bitonic_sort", [](auto lo, auto hi) { bitonic_sort(lo, hi); });
  return 0;
}
