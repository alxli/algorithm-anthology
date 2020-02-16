/*

These functions are equivalent to std::sort(), taking random-access iterators
as a range [lo, hi) to be sorted. Elements between lo and hi (including the
element pointed to by lo but excluding the element pointed to by hi) will be
sorted into ascending order after the function call. Optionally, a comparison
function object specifying a strict weak ordering may be specified to replace
the default operator <.

These functions are not meant to compete with standard library implementations
in terms terms of speed. Instead, they are meant to demonstrate how common
sorting algorithms can be concisely implemented in C++.

*/

#include <algorithm>
#include <functional>
#include <iterator>
#include <vector>

/*

Quicksort repeatedly selects a pivot and partitions the range so that elements
comparing less than the pivot precede the pivot, and elements comparing greater
or equal follow it. Divide and conquer is then applied to both sides of the
pivot until the original range is sorted. Despite having a worst case of O(n^2),
quicksort is often faster in practice than merge sort and heapsort, which both
have a worst case time complexity of O(n log n).

The pivot chosen in this implementation is always a middle element of the range
to be sorted. To reduce the likelihood of encountering the worst case, the pivot
can be chosen in better ways (e.g. randomly, or using the "median of three"
technique).

Time Complexity (Average): O(n log n).
Time Complexity (Worst): O(n^2).
Space Complexity: O(log n) auxiliary stack space.
Stable?: No.

*/

template<class It, class Compare>
void quicksort(It lo, It hi, Compare comp) {
  if (hi - lo < 2) {
    return;
  }
  typedef typename std::iterator_traits<It>::value_type T;
  T pivot = *(lo + (hi - lo)/2);
  It i, j;
  for (i = lo, j = hi - 1; ; ++i, --j) {
    while (comp(*i, pivot)) {
      ++i;
    }
    while (comp(pivot, *j)) {
      --j;
    }
    if (i >= j) {
      break;
    }
    std::swap(*i, *j);
  }
  quicksort(lo, i, comp);
  quicksort(i, hi, comp);
}

template<class It>
void quicksort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  quicksort(lo, hi, std::less<T>());
}

/*

Merge sort first divides a list into n sublists of one element each, then
recursively merges the sublists into sorted order until only a single sorted
sublist remains. Merge sort is a stable sort, meaning that it preserves the
relative order of elements which compare equal by operator < or the custom
comparator given.

An analogous function in the C++ standard library is std::stable_sort(), except
that the implementation here requires sufficient memory to be available. When
O(n) auxiliary memory is not available, std::stable_sort() falls back to a time
complexity of O(n log^2 n) whereas the implementation here will simply fail.

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
  It mid = lo + (hi - lo - 1)/2, a = lo, c = mid + 1;
  mergesort(lo, mid + 1, comp);
  mergesort(mid + 1, hi, comp);
  typedef typename std::iterator_traits<It>::value_type T;
  std::vector<T> merged;
  while (a <= mid && c < hi) {
    merged.push_back(comp(*c, *a) ? *c++ : *a++);
  }
  if (a > mid) {
    for (It it = c; it < hi; ++it) {
      merged.push_back(*it);
    }
  } else {
    for (It it = a; it <= mid; ++it) {
      merged.push_back(*it);
    }
  }
  for (int i = 0; i < hi - lo; i++) {
    *(lo + i) = merged[i];
  }
}

template<class It>
void mergesort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  mergesort(lo, hi, std::less<T>());
}

/*

Heapsort first rearranges an array to satisfy the max-heap property. Then, it
repeatedly pops the max element of the heap (the left, unsorted subrange),
moving it to the beginning of the right, sorted subrange until the entire range
is sorted. Heapsort has a better worst case time complexity than quicksort and
also a better space complexity than merge sort.

The C++ standard library equivalent is calling std::make_heap(lo, hi), followed
by std::sort_heap(lo, hi).

Time Complexity (Average): O(n log n).
Time Complexity (Worst): O(n log n).
Space Complexity: O(1) auxiliary.
Stable?: No.

*/

template<class It, class Compare>
void heapsort(It lo, It hi, Compare comp) {
  typename std::iterator_traits<It>::value_type tmp;
  It i = lo + (hi - lo)/2, j = hi, parent, child;
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
    child = lo + 2*(i - lo) + 1;
    while (child < j) {
      if (child + 1 < j && comp(*child, *(child + 1))) {
        child++;
      }
      if (!comp(tmp, *child)) {
        break;
      }
      *parent = *child;
      parent = child;
      child = lo + 2*(parent - lo) + 1;
    }
    *(lo + (parent - lo)) = tmp;
  }
}

template<class It>
void heapsort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  heapsort(lo, hi, std::less<T>());
}

/*

Comb sort is an improved bubble sort. While bubble sort increments the gap
between swapped elements for every inner loop iteration, comb sort fixes the gap
size in the inner loop, decreasing it by a particular shrink factor in every
iteration of the outer loop. The shrink factor of 1.3 is empirically determined
to be the most effective.

Even though the worst case time complexity is O(n^2), a well chosen shrink
factor ensures that the gap sizes are co-prime, in turn requiring astronomically
large n to make the algorithm exceed O(n log n) steps. On random arrays, comb
sort is only 2-3 times slower than merge sort. Its short code length length
relative to its good performance makes it a worthwhile algorithm to remember.

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
      gap = (int)((double)gap / 1.3);
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
  typedef typename std::iterator_traits<It>::value_type T;
  combsort(lo, hi, std::less<T>());
}

/*

Radix sort is used to sort integer elements with a constant number of bits in
linear time. This implementation only works on ranges pointing to unsigned
integer primitives. The elements in the input range do not strictly have to be
unsigned types, as long as their values are nonnegative integers.

In this implementation, a power of two is chosen to be the base for the sort
so that bitwise operations can be easily used to extract digits. This avoids the
need to use modulo and exponentiation, which are much more expensive operations.
In practice, it's been demonstrated that 2^8 is the best choice for sorting
32-bit integers (approximately 5 times faster than std::sort(), and typically
2-4 faster than radix sort using any other power of two chosen as the base).

Time Complexity: O(n*w) for n integers of w bits each.
Space Complexity: O(n + w) auxiliary.

*/

template<class UnsignedIt>
void radix_sort(UnsignedIt lo, UnsignedIt hi) {
  if (hi - lo < 2) {
    return;
  }
  const int radix_bits = 8;
  const int radix_base = 1 << radix_bits;  // e.g. 2^8 = 256
  const int radix_mask = radix_base - 1;  // e.g. 2^8 - 1 = 0xFF
  int num_bits = 8*sizeof(*lo);  // 8 bits per byte
  typedef typename std::iterator_traits<UnsignedIt>::value_type T;
  T *buf = new T[hi - lo];
  for (int pos = 0; pos < num_bits; pos += radix_bits) {
    int count[radix_base] = {0};
    for (UnsignedIt it = lo; it != hi; ++it) {
      count[(*it >> pos) & radix_mask]++;
    }
    T *bucket[radix_base], *curr = buf;
    for (int i = 0; i < radix_base; curr += count[i++]) {
      bucket[i] = curr;
    }
    for (UnsignedIt it = lo; it != hi; ++it) {
      *bucket[(*it >> pos) & radix_mask]++ = *it;
    }
    std::copy(buf, buf + (hi - lo), lo);
  }
  delete[] buf;
}

/*** Example Usage and Output:

mergesort() with default comparisons: 1.32 1.41 1.62 1.73 2.58 2.72 3.14 4.67
mergesort() with 'compare_as_ints()': 1.41 1.73 1.32 1.62 2.72 2.58 3.14 4.67
------
Sorting five million integers...
std::sort():  0.355s
quicksort():  0.426s
mergesort():  1.263s
heapsort():   1.093s
combsort():   0.827s
radix_sort(): 0.076s

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
  return (int)i < (int)j;
}

int main () {
  {  // Can be used to sort arrays like std::sort().
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(a, a + 8);
    assert(sorted(a, a + 8));
  }
  {  // STL containers work too.
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    vector<int> v(a, a + 8);
    quicksort(v.begin(), v.end());
    assert(sorted(v.begin(), v.end()));
  }
  {  // Reverse iterators work as expected.
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    vector<int> v(a, a + 8);
    heapsort(v.rbegin(), v.rend());
    assert(sorted(v.rbegin(), v.rend()));
  }
  {  // We can sort doubles just as well.
    double a[] = {1.1, -5.0, 6.23, 4.123, 155.2};
    vector<double> v(a, a + 5);
    combsort(v.begin(), v.end());
    assert(sorted(v.begin(), v.end()));
  }
  {  // Must use radix_sort with unsigned values, but sorting in reverse works!
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    vector<int> v(a, a + 8);
    radix_sort(v.rbegin(), v.rend());
    assert(sorted(v.rbegin(), v.rend()));
  }

  // Example from: http://www.cplusplus.com/reference/algorithm/stable_sort
  double a[] = {3.14, 1.41, 2.72, 4.67, 1.73, 1.32, 1.62, 2.58};
  {
    vector<double> v(a, a + 8);
    cout << "mergesort() with default comparisons: ";
    mergesort(v.begin(), v.end());
    print_range(v.begin(), v.end());
  }
  {
    vector<double> v(a, a + 8);
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

#define test(sort_function) {                            \
  clock_t start = clock();                               \
  sort_function(v.begin(), v.end());                     \
  double t = (double)(clock() - start) / CLOCKS_PER_SEC; \
  cout << setw(14) << left << #sort_function "(): ";     \
  cout << fixed << t << "s" << endl;                     \
  assert(sorted(v.begin(), v.end()));                    \
  v = v2;                                                \
}
  test(std::sort);
  test(quicksort);
  test(mergesort);
  test(heapsort);
  test(combsort);
  test(radix_sort);
  return 0;
}
