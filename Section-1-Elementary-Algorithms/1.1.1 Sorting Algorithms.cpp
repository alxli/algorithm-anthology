/*

1.1.1 - Sorting Algorithms

The following functions are to be used like std::sort(), taking two
RandomAccessIterators as the range to be sorted, and optionally a
comparison function object to replace the default < operator.

They are not intended to compete with the standard library sorting
functions in terms of speed, but are merely demonstrations of how to
implement common sorting algorithms concisely in C++.

*/

#include <algorithm>  /* std::(stable_)partition(), std::swap() */
#include <functional> /* std::less<T> */
#include <iterator>   /* std::iterator_traits<T> */

/*

Quicksort

Time Complexity (Best): O(n log n)
Time Complexity (Average): O(n log n)
Time Complexity (Worst): O(n^2)
Space Complexity: O(log n) auxiliary.
Stable?: No

Quicksort repeatedly selects a pivot and "partitions" the range so that
all values comparing less than the pivot come before it, and all values
comparing greater comes after it. Divide and conquer is then applied to
both sides of the pivot until the original range is sorted. Despite
having a worst case of O(n^2), quicksort is faster in practice than
merge sort and heapsort, which each has a worst case of O(n log n).

The pivot chosen in this implementation is always the middle element
of the range to be sorted. To reduce the likelihood of encountering the
worst case, the algorithm should be modified to select a random pivot,
or use the "median of three" method.

*/

template<class It, class Compare>
void quicksort(It lo, It hi, Compare cmp) {
  if (hi - lo < 2) return;
  typedef typename std::iterator_traits<It>::value_type T;
  T pivot = *(lo + (hi - lo) / 2);
  It i, j;
  for (i = lo, j = hi - 1; ; ++i, --j) {
    while (cmp(*i, pivot)) ++i;
    while (cmp(pivot, *j)) --j;
    if (i >= j) break;
    std::swap(*i, *j);
  }
  quicksort(lo, i, cmp);
  quicksort(i, hi, cmp);
}

template<class It> void quicksort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  quicksort(lo, hi, std::less<T>());
}

/*

Merge Sort

Time Complexity (Best): O(n log n)
Time Complexity (Average): O(n log n)
Time Complexity (Worst): O(n log n)
Space Complexity: O(n) auxiliary.
Stable?: Yes

Merge sort works by first dividing a list into n sublists, each with
one element, then recursively merging sublists to produce new sorted
sublists until only a single sorted sublist remains. Merge sort has a
better worse case than quicksort, and is also stable, meaning that it
will preserve the relative ordering of elements considered equal by
the < operator or comparator (a < b and b < a both return false).

While std::stable_sort() is a corresponding function in the standard
library, the implementation below differs in that it will simply fail
if extra memory is not available. Meanwhile, std::stable_sort() will
not fail, but instead fall back to a time complexity of O(n log^2 n).

*/

template<class It, class Compare>
void mergesort(It lo, It hi, Compare cmp) {
  if (lo >= hi - 1) return;
  It mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  mergesort(lo, mid + 1, cmp);
  mergesort(mid + 1, hi, cmp);
  typedef typename std::iterator_traits<It>::value_type T;
  T *buf = new T[hi - lo], *b = buf;
  while (a <= mid && c < hi)
    *(b++) = cmp(*c, *a) ? *(c++) : *(a++);
  if (a > mid)
    for (It k = c; k < hi; k++) *(b++) = *k;
  else
    for (It k = a; k <= mid; k++) *(b++) = *k;
  for (int i = hi - lo - 1; i >= 0; i--)
    *(lo + i) = buf[i];
  delete[] buf;
}

template<class It> void mergesort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  mergesort(lo, hi, std::less<T>());
}

/*

Heapsort

Time Complexity (Best): O(n log n)
Time Complexity (Average): O(n log n)
Time Complexity (Worst): O(n log n)
Space Complexity: O(1) auxiliary.
Stable?: No

Heapsort first rearranges an array to satisfy the heap property, and
then the max element of the heap is repeated removed and added to the
end of the resulting sorted list. A heapified array has the root node
at index 0. The two children of the node at index n are respectively
located at indices 2n + 1 and 2n + 2. Each node is greater than both
of its children. This leads to a structure that takes O(log n) to
insert any element or remove the max element. Heapsort has a better
worst case complexity than quicksort, but a better space complexity
complexity than merge sort.

The standard library equivalent is calling std::make_heap(), followed
by std::sort_heap() on the input range.

*/

template<class It, class Compare>
void heapsort(It lo, It hi, Compare cmp) {
  typename std::iterator_traits<It>::value_type t;
  It i = lo + (hi - lo) / 2, j = hi, parent, child;
  for (;;) {
    if (i <= lo) {
      if (--j == lo) return;
      t = *j;
      *j = *lo;
    } else {
      t = *(--i);
    }
    parent = i;
    child = lo + 2 * (i - lo) + 1;
    while (child < j) {
      if (child + 1 < j && cmp(*child, *(child + 1))) child++;
      if (!cmp(t, *child)) break;
      *parent = *child;
      parent = child;
      child = lo + 2 * (parent - lo) + 1;
    }
    *(lo + (parent - lo)) = t;
  }
}

template<class It> void heapsort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  heapsort(lo, hi, std::less<T>());
}

/*

Comb Sort

Time Complexity (Best): O(n)
Time Complexity (Average): O(n^2 / 2^p) for p increments.
Time Complexity (Worst): O(n^2)
Space Complexity: O(1) auxiliary.
Stable?: No

Comb sort is an improved bubble sort. While bubble sort increases the
gap between swapped elements for every inner loop iteration, comb sort
uses a fixed gap for the inner loop and decreases the gap size by a
shrink factor for every iteration of the outer loop.

Even though the average time complexity is theoretically O(n^2), if the
increments (gap sizes) are relatively prime and the shrink factor is
sensible (1.3 is empirically determined to be the best), then it will
require astronomically large n to make the algorithm exceed O(n log n)
steps. In practice, comb sort is only 2-3 times slower than merge sort.

*/

template<class It, class Compare>
void combsort(It lo, It hi, Compare cmp) {
  int gap = hi - lo;
  bool swapped = true;
  while (gap > 1 || swapped) {
    if (gap > 1) gap = (int)((float)gap / 1.3f);
    swapped = false;
    for (It i = lo; i + gap < hi; i++)
      if (cmp(*(i + gap), *i)) {
        std::swap(*i, *(i + gap));
        swapped = true;
      }
  }
}

template<class It> void combsort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  combsort(lo, hi, std::less<T>());
}

/*

Radix Sort

Time Complexity (Worst): O(n * w) for n integers of w bits.
Space Complexity: O(n + w) auxiliary.

Radix sort can be used to sort integer keys with a constant number of
bits in linear time. The keys are grouped by the individual digits
which share the same significant position and value. The implementation
below only works on 32-bit signed integers, employing std::partition()
and std::stable_partition() instead of a custom bucket/counting sort.

The least significant digit (LSD) radix sort is a stable sort, but
because std::stable_partition() is slower than std::partition(), the
most significant digit (MSD) radix sort is much faster. Note that the
LSD version also uses auxiliary memory due to std::stable_partition(),
which will fall back to O(n log n) running time if none is available.

*/

struct radix_comp {
  const int p; //bit position (0..31) to examine
  radix_comp(int offset): p(offset) {}
  bool operator () (int v) const {
    return (p == 31) ? (v < 0) : !(v & (1 << p));
  }
};

void msd_radix_sort(int * lo, int * hi, int msb = 31) {
  if (lo == hi || msb < 0) return;
  int *mid = std::partition(lo, hi, radix_comp(msb--));
  msd_radix_sort(lo, mid, msb);
  msd_radix_sort(mid, hi, msb);
}

void lsd_radix_sort(int * lo, int * hi) {
  for (int lsb = 0; lsb < 32; lsb++)
    std::stable_partition(lo, hi, radix_comp(lsb));
}

/*** Example Usage ***/

#include <iostream>
#include <vector>
using namespace std;

template<class T> void print_range(T lo, T hi) {
  while (lo != hi) cout << *(lo++) << " ";
  cout << "\n";
}

bool compare_as_ints(double i, double j) {
  return (int)i < (int)j;
}

int main () {
  { //can be used to sort arrays like std::sort()
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(a, a + 8);
    print_range(a, a + 8);
  }
  { //STL containers work too
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    vector<int> v(a, a + 8);
    quicksort(v.begin(), v.end());
    print_range(v.begin(), v.end());
  }
  { //reverse iterators work as expected
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    vector<int> v(a, a + 8);
    heapsort(v.rbegin(), v.rend());
    print_range(v.begin(), v.end());
  }
  { //doubles are also fine
    double a[] = {1.1, -5.0, 6.23, 4.123, 155.2};
    vector<double> v(a, a + 5);
    combsort(v.begin(), v.end());
    print_range(v.begin(), v.end());
  }
  { //radix sort only works on 32-bit signed integers
    int a[] = {1, 886, 6, -50, 7, -11, 123, 4};
    msd_radix_sort(a, a + 8);
    print_range(a, a + 8);
  }

  //example from cplusplus.com/reference/algorithm/stable_sort/
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
  return 0;
}
