/*

1.1.1 - Sorting Algorithms

The following functions behave just like the sorting functions
found in the C++ standard library, taking RandomAccessIterators
as the range to be sorted, and optionally a comparator.

*/

#include <algorithm>  /* std::(stable_)partition, std::swap */
#include <functional> /* std::less<T> */
#include <iterator>   /* std::iterator_traits<T> */

/*

Quicksort

Best: O(N*log(N))    Average: O(N*log(N))    Worst: O(N^2)
     Auxiliary Space: O(log(N))          Stable?: No

Notes: The pivot value chosen here is always half way between
lo and hi. Choosing random pivot values reduces the chances of
encountering the worst case performance of O(N^2). Quicksort is
faster in practice than other O(N*log(N)) algorithms.

*/

template<class It, class Compare>
void quicksort(It lo, It hi, Compare cmp) {
  if (lo + 1 >= hi) return;
  It i, j, pivot = lo + (hi - lo) / 2;
  for (i = lo, j = hi - 1; ; ++i, ++j) {
    while (cmp(*i, *pivot)) ++i;
    while (cmp(*pivot, *j)) --j;
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

Best: O(N*log(N))    Average: O(N*log(N))    Worst: O(N*log(N))
     Auxiliary Space: O(N)               Stable?: Yes

Notes: Merge sort has a better worse case complexity than
quicksort and is stable (i.e. it maintains the relative order
of equivalent values after the sort). The implementation below
is different from std::stable_sort() in that it will simply fail
if extra memory is not available. Meanwhile, std::stable_sort()
will fall back to a run time of O(N*log^2(N)) if out of memory.

*/

template<class It, class Compare>
void mergesort(It lo, It hi, Compare cmp) {
  typedef typename std::iterator_traits<It>::value_type T;
  if (lo >= hi - 1) return;
  It mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
  mergesort(lo, mid + 1, cmp);
  mergesort(mid + 1, hi, cmp);
  T buf[hi - lo], *b = buf;
  while (a <= mid && c < hi)
    *(b++) = cmp(*c, *a) ? *(c++) : *(a++);
  if (a > mid)
    for (It k = c; k < hi; k++) *(b++) = *k;
  else
    for (It k = a; k <= mid; k++) *(b++) = *k;
  for (int i = hi - lo - 1; i >= 0; i--)
    *(lo + i) = buf[i];
}

template<class It> void mergesort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  mergesort(lo, hi, std::less<T>());
}

/*

Heap Sort

Best: O(N*log(N))     Average: O(N*log(N))     Worst: O(N*log(N))
     Auxiliary Space: O(1)                Stable?: No

Heap sort has a better worst case complexity than quicksort,
but also a better space complexity than merge sort. On average,
this will likely run slower than a well implemented quicksort.

*/

template<class It, class Compare>
void heapsort(It lo, It hi, Compare cmp) {
  typename std::iterator_traits<It>::value_type t;
  It n = hi, i = lo + (n - lo) / 2, parent, child;
  for (;;) {
    if (i <= lo) {
      if (--n == lo) return;
      t = *n, *n = *lo;
    } else t = *(--i);
    parent = i, child = lo + 2 * (i - lo) + 1;
    while (child < n) {
      if (child + 1 < n && cmp(*child, *(child + 1))) child++;
      if (!cmp(t, *child)) break;
      *parent = *child, parent = child;
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

Best: O(N)       Average: O(N^2/(2^p)) for p increments of gap
   Worst: O(N^2)     Auxiliary Space: O(1)     Stable?: No

Comb sort is an improved bubble sort that's simple to memorize.

*/

template<class It, class Compare>
void combsort(It lo, It hi, Compare cmp) {
  int gap = hi - lo, swapped = 1;
  while (gap > 1 || swapped) {
    if (gap > 1) gap = (int)((float)gap / 1.3f);
    swapped = 0;
    for (It i = lo; i + gap < hi; i++)
      if (cmp(*(i + gap), *i)) {
        std::swap(*i, *(i + gap));
        swapped = 1;
      }
  }
}

template<class It> void combsort(It lo, It hi) {
  typedef typename std::iterator_traits<It>::value_type T;
  combsort(lo, hi, std::less<T>());
}

/*

Radix Sort (32-bit Signed Integers Only!)

Worst: O(d*N), where d is the number of bits/digits in each
of the N values. Since d is constant (32), one can say that
radix sort runs in linear time.

Auxiliary Space: O(d)

Stable?: The LSD (least significant digit) radix sort is
stable, but because std::stable_partition() is slower than
the unstable std::partition(), the MSD (most significant
digit) radix sort is much faster. Both are given here, with
the MSD version implemented recursively.

Notes: Radix sort usually employs a bucket sort or counting
sort. Here, we take advantage of the partition functions
found in the C++ library.

*/

struct radix_comp { //UnaryPredicate for std::partition
  const int p; //bit position [0..31] to examine
  radix_comp(int offset): p(offset) {}
  bool operator () (int v) const {
    return (p == 31) ? (v < 0) : !(v & (1 << p));
  }
};

void lsd_radix_sort(int * lo, int * hi) {
  for (int lsb = 0; lsb < 32; lsb++)
    std::stable_partition(lo, hi, radix_comp(lsb));
}

void msd_radix_sort(int * lo, int * hi, int msb = 31) {
  if (lo == hi || msb < 0) return;
  int * mid = std::partition(lo, hi, radix_comp(msb--));
  msd_radix_sort(lo, mid, msb);
  msd_radix_sort(mid, hi, msb);
}

/*** Example Usage ***/

#include <iostream>
#include <vector>
using namespace std;

bool compare_as_ints(double i, double j) {
  return (int)i < (int)j;
}

template<class T> void print_range(T lo, T hi) {
  while (lo != hi) cout << *(lo++) << " ";
  cout << "\n";
}

int main () {

  { //can be used to sort arrays like std::sort()
    int a[] = {32, 71, 12, 45, 26, 80, 53, 33};
    quicksort(a, a + 8);
    print_range(a, a + 8);
  }

  { //STL contains works too
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

  { //doubles are fine also
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
    cout << "mergesort() with default comparison: ";
    mergesort(v.begin(), v.end());
    print_range(v.begin(), v.end());
  }

  {
    vector<double> v(a, a + 8);
    cout << "mergesort() with 'compare_as_ints' : ";
    mergesort(v.begin(), v.end(), compare_as_ints);
    print_range(v.begin(), v.end());
  }

  return 0;
}
