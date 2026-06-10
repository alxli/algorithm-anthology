/*

These functions are equivalent to `std::sort()`, taking random-access iterators as a range
`[lo, hi)` to be sorted. Elements between `lo` and `hi` (including the element pointed to by `lo`
but excluding the element pointed to by `hi`) will be sorted into ascending order after the function
call. Optionally, a comparison function object specifying a strict weak ordering may be specified to
replace the default `operator<`.

- `quicksort(lo, hi)` sorts the range using quicksort.
- `mergesort(lo, hi)` sorts the range using merge sort, which is stable.
- `heapsort(lo, hi)` sorts the range using heapsort.
- `combsort(lo, hi)` sorts the range using comb sort.
- `radix_sort(lo, hi)` sorts the range using least-significant-byte radix sort.

`radix_sort()` is the exception to the shared interface above: it takes no comparator and requires
an integer value type. These functions are not meant to compete with standard library
implementations in terms of speed. Instead, they are meant to demonstrate how common sorting
algorithms can be concisely implemented in C++.

*/

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

/*

Quicksort repeatedly selects a pivot and partitions the range so that elements comparing less than
the pivot precede it, elements comparing equal stay in the middle, and elements comparing greater
follow it. Divide and conquer is then applied to the two outer ranges until the original range is
sorted. Despite having a worst case of O(n^2), quicksort is often faster in practice than merge sort
and heapsort, which both have a worst case time complexity of O(n log n).

The pivot chosen in this implementation is always a middle element of the range to be sorted. To
reduce the likelihood of encountering the worst case, the pivot can be chosen in better ways (e.g.
randomly, or using the "median of three" technique).

Time Complexity (Average): O(n log n).
Time Complexity (Worst): O(n^2).
Space Complexity: O(log n) auxiliary stack space.
Stable?: No.

*/

template<class It, class Compare>
void quicksort(It lo, It hi, Compare comp) {
  while (hi - lo >= 2) {
    auto pivot = *(lo + (hi - lo) / 2);
    It lt = lo, i = lo, gt = hi;
    while (i != gt) {
      if (comp(*i, pivot)) {
        std::iter_swap(lt++, i++);
      } else if (comp(pivot, *i)) {
        std::iter_swap(i, --gt);
      } else {
        ++i;
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

template<class It>
void quicksort(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  quicksort(lo, hi, std::less<T>());
}

/*

Merge sort first divides a list into n sublists of one element each, then recursively merges the
sublists into sorted order until only a single sorted sublist remains. Merge sort is a stable sort,
meaning that it preserves the relative order of elements which compare equal by `operator<` or the
custom comparator given.

An analogous function in the C++ standard library is `std::stable_sort()`, except that the
implementation here requires sufficient memory to be available. When O(n) auxiliary memory is not
available, `std::stable_sort()` falls back to a time complexity of O(n log^2 n) whereas the
implementation here will simply fail.

Time Complexity (Average): O(n log n).
Time Complexity (Worst): O(n log n).
Space Complexity: O(log n) auxiliary stack space and O(n) auxiliary heap space.
Stable?: Yes.

*/

template<class It, class Compare>
void mergesort(It lo, It hi, Compare comp) {
  if (hi - lo < 2) {
    return;
  }
  It mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  mergesort(lo, mid + 1, comp);
  mergesort(mid + 1, hi, comp);
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> merged;
  while (a <= mid && c < hi) {
    merged.push_back(comp(*c, *a) ? *c++ : *a++);
  }
  merged.insert(merged.end(), a, mid + 1);
  merged.insert(merged.end(), c, hi);
  std::copy(merged.begin(), merged.end(), lo);
}

template<class It>
void mergesort(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  mergesort(lo, hi, std::less<T>());
}

/*

Heapsort first rearranges an array to satisfy the max-heap property. Then, it repeatedly pops the
max element of the heap (the left, unsorted subrange), moving it to the beginning of the right,
sorted subrange until the entire range is sorted. Heapsort has a better worst case time complexity
than quicksort and also a better space complexity than merge sort.

The C++ standard library equivalent is calling `std::make_heap(lo, hi)`, followed by
`std::sort_heap(lo, hi)`.

The loop below has two modes controlled by whether `i > lo`. While `i > lo`, it is heapifying
(Floyd's O(n) bottom-up heap build: sift down each non-leaf from right to left). Once `i == lo`,
the heap is built, and the loop switches to extraction: repeatedly swap the root to the back of
the unsorted region (`*j`) and sift down the new root.

Time Complexity (Average): O(n log n).
Time Complexity (Worst): O(n log n).
Space Complexity: O(1) auxiliary.
Stable?: No.

*/

template<class It, class Compare>
void heapsort(It lo, It hi, Compare comp) {
  using T = typename std::iterator_traits<It>::value_type;
  T tmp;
  It i = lo + (hi - lo) / 2, j = hi, parent, child;
  for (;;) {
    if (i <= lo) {
      if (--j == lo) {
        return;
      }
      tmp = *j;
      *j = *lo;
    } else {
      tmp = *(--i);
    }
    parent = i;
    child = lo + 2 * (i - lo) + 1;
    while (child < j) {
      if (child + 1 < j && comp(*child, *(child + 1))) {
        child++;
      }
      if (!comp(tmp, *child)) {
        break;
      }
      *parent = *child;
      parent = child;
      child = lo + 2 * (parent - lo) + 1;
    }
    *(lo + (parent - lo)) = tmp;
  }
}

template<class It>
void heapsort(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  heapsort(lo, hi, std::less<T>());
}

/*

Comb sort is an improved bubble sort. While bubble sort increments the gap between swapped elements
for every inner loop iteration, comb sort fixes the gap size in the inner loop, decreasing it by a
particular shrink factor in every iteration of the outer loop. The shrink factor of 1.3 is
empirically determined to be the most effective.

Even though the worst case time complexity is O(n^2), a well chosen shrink factor ensures that the
gap sizes are co-prime, in turn requiring astronomically large n to make the algorithm exceed O(n
log n) steps. On random arrays, comb sort is only 2-3 times slower than merge sort. Its short code
length relative to its good performance makes it a worthwhile algorithm to remember.

Time Complexity (Worst): O(n^2).
Space Complexity: O(1) auxiliary.
Stable?: No.

*/

template<class It, class Compare>
void combsort(It lo, It hi, Compare comp) {
  int gap = hi - lo;
  bool swapped = true;
  while (gap > 1 || swapped) {
    if (gap > 1) {
      gap = gap * 10 / 13;
    }
    swapped = false;
    for (It it = lo; it + gap < hi; ++it) {
      if (comp(*(it + gap), *it)) {
        std::swap(*it, *(it + gap));
        swapped = true;
      }
    }
  }
}

template<class It>
void combsort(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  combsort(lo, hi, std::less<T>());
}

/*

Radix sort is used to sort integer elements with a constant number of bits in linear time. This
implementation only works on ranges pointing to unsigned integer primitives. The elements in the
input range do not strictly have to be unsigned types, as long as their values are nonnegative
integers.

In this implementation, a power of two is chosen to be the base for the sort so that bitwise
operations can be easily used to extract digits. This avoids the need to use modulo and
exponentiation, which are much more expensive operations. In practice, it's been demonstrated that
$2^8$ is the best choice for sorting 32-bit integers (approximately 5 times faster than
`std::sort()`, and typically 2-4 times faster than radix sort using any other power of two chosen as
the base).

Time Complexity: O(n*w) for $n$ integers of $w$ bits each.
Space Complexity: O(n + 2^b) auxiliary for a radix of $b$ bits, i.e. O(n) for constant $b$.

*/

template<class It>
void radix_sort(It lo, It hi) {
  if (hi - lo < 2) {
    return;
  }
  const int radix_bits = 8;
  const int radix_base = 1 << radix_bits;  // e.g. 2^8 = 256
  const int radix_mask = radix_base - 1;   // e.g. 2^8 - 1 = 0xFF
  int num_bits = 8 * sizeof(*lo);          // 8 bits per byte
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> buf(hi - lo);
  for (int pos = 0; pos < num_bits; pos += radix_bits) {
    std::vector<int> count(radix_base);
    for (It it = lo; it != hi; ++it) {
      count[(*it >> pos) & radix_mask]++;
    }
    std::vector<T *> bucket(radix_base);
    T *curr = buf.data();
    for (int i = 0; i < radix_base; curr += count[i++]) {
      bucket[i] = curr;
    }
    for (It it = lo; it != hi; ++it) {
      *bucket[(*it >> pos) & radix_mask]++ = *it;
    }
    std::copy(buf.begin(), buf.end(), lo);
  }
}

/*** Example Usage and Output:

mergesort() with default comparisons: 1.32 1.41 1.62 1.73 2.58 2.72 3.14 4.67 
mergesort() with 'compare_as_ints()': 1.41 1.73 1.32 1.62 2.72 2.58 3.14 4.67 
------
Sorting five million integers...
std::sort():  0.223s
quicksort():  0.253s
mergesort():  0.621s
heapsort():   0.439s
combsort():   0.365s
radix_sort(): 0.016s

***/

#include <cassert>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <vector>
using namespace std;

template<class It>
void print_range(It lo, It hi) {
  while (lo != hi) {
    cout << *lo++ << " ";
  }
  cout << endl;
}

template<class It>
bool sorted(It lo, It hi) {
  while (++lo != hi) {
    if (*lo < *(lo - 1)) {
      return false;
    }
  }
  return true;
}

bool compare_as_ints(double i, double j) {
  return static_cast<int>(i) < static_cast<int>(j);
}

int main() {
  {  // Can be used to sort arrays like std::sort().
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(a.begin(), a.end());
    assert(sorted(a.begin(), a.end()));
  }
  {  // STL containers work too.
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(a.begin(), a.end());
    assert(sorted(a.begin(), a.end()));
  }
  {  // Reverse iterators work as expected.
    vector<int> a{32, 71, 12, 45, 26, 80, 53, 33};
    heapsort(a.rbegin(), a.rend());
    assert(sorted(a.rbegin(), a.rend()));
  }
  {  // We can sort doubles just as well.
    vector<double> a{1.1, -5.0, 6.23, 4.123, 155.2};
    combsort(a.begin(), a.end());
    assert(sorted(a.begin(), a.end()));
  }
  {  // Must use radix_sort with unsigned values, but sorting in reverse works!
    vector<unsigned int> a{32, 71, 12, 45, 26, 80, 53, 33};
    radix_sort(a.rbegin(), a.rend());
    assert(sorted(a.rbegin(), a.rend()));
  }

  // Example from: http://www.cplusplus.com/reference/algorithm/stable_sort
  const vector<double> a{3.14, 1.41, 2.72, 4.67, 1.73, 1.32, 1.62, 2.58};
  {
    vector<double> v(a);
    cout << "mergesort() with default comparisons: ";
    mergesort(v.begin(), v.end());
    print_range(v.begin(), v.end());
  }
  {
    vector<double> v(a);
    cout << "mergesort() with 'compare_as_ints()': ";
    mergesort(v.begin(), v.end(), compare_as_ints);
    print_range(v.begin(), v.end());
  }
  cout << "------" << endl;

  vector<int> v, v2;
  for (int i = 0; i < 5000000; i++) {
    v.push_back((rand() & 0x7fff) | ((rand() & 0x7fff) << 15));
  }
  v2 = v;
  cout << "Sorting five million integers..." << endl;
  cout.precision(3);

#define test(sort_function)                                             \
  {                                                                     \
    clock_t start = clock();                                            \
    sort_function(v.begin(), v.end());                                  \
    double t = static_cast<double>((clock() - start)) / CLOCKS_PER_SEC; \
    cout << setw(14) << left << #sort_function "(): ";                  \
    cout << fixed << t << "s" << endl;                                  \
    assert(sorted(v.begin(), v.end()));                                 \
    v = v2;                                                             \
  }
  test(std::sort);
  test(quicksort);
  test(mergesort);
  test(heapsort);
  test(combsort);
  test(radix_sort);
  return 0;
}
