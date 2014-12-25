#include <algorithm> /* std::swap(), std::(stable_)partition */
#include <iterator>  /* std::iterator_traits<T> */

/*

2.1 Quicksort

Best: O(N*log(N))    Average: O(N*log(N))    Worst: O(N^2)
  Auxiliary Space: O(log(N))               Stable?: No

Notes: The pivot value chosen here is always half way
between lo and hi. Choosing random pivot values reduces
the chances of encountering the worst case performance
of O(N^2). Quicksort is faster in practice than other
O(N*log(N)) algorithms.

*/

template<class RAI> void quicksort(RAI lo, RAI hi) {
    if (lo + 1 >= hi) return;
    std::swap(*lo, *(lo + (hi - lo) / 2));
    RAI x = lo;
    for (RAI i = lo + 1; i < hi; i++)
        if (*i < *lo) std::swap(*(++x), *i);
    std::swap(*lo, *x);
    quicksort(lo, x);
    quicksort(x + 1, hi);
}

/*

2.2 - Merge Sort

Best: O(N*log(N))    Average: O(N*log(N))    Worst: O(N*log(N))
    Auxiliary Space: O(N)                  Stable?: Yes

Notes: Merge Sort has a better worse case complexity than
quicksort and is stable (i.e. it maintains the relative order
of equivalent values after the sort). This is different from
std::stable_sort() in that it will simply fail if extra memory
is not available. Meanwhile, std::stable_sort() will fall back
to a run time of O(N*log^2(N)) if out of memory.

*/

template<class RAI> void mergesort(RAI lo, RAI hi) {
    if (lo >= hi - 1) return;
    RAI mid = lo + (hi - lo - 1) / 2, a = lo, c = mid + 1;
    mergesort(lo, mid + 1);
    mergesort(mid + 1, hi);
    typedef typename std::iterator_traits<RAI>::value_type T;
    T *buff = new T[hi - lo], *b = buff;
    while (a <= mid && c < hi)
        *(b++) = (*c < *a) ? *(c++) : *(a++);
    if (a > mid)
        for (RAI k = c; k < hi; k++) *(b++) = *k;
    else
        for (RAI k = a; k <= mid; k++) *(b++) = *k;
    for (int i = hi - lo - 1; i >= 0; i--)
        *(lo + i) = buff[i];
}

/*

2.3 - Heap Sort

Best: O(N*log(N))     Average: O(N*log(N))     Worst: O(N*log(N))
    Auxiliary Space: O(1)                   Stable?: No

Notes: Heap Sort has a better worst case complexity than quicksort,
but also a better space complexity than merge sort. On average,
this will very likely run slower than a well implemented quicksort.

*/

template<class RAI> void heapsort(RAI lo, RAI hi) {
    typename std::iterator_traits<RAI>::value_type t;
    RAI n = hi, i = (n - lo) / 2 + lo, parent, child;
    while (true) {
        if (i <= lo) {
            if (--n == lo) return;
            t = *n, *n = *lo;
        } else t = *(--i);
        parent = i, child = lo + (i - lo) * 2 + 1;
        while (child < n) {
            if (child+1 < n && *child < *(child+1)) child++;
            if (!(t < *child)) break;
            *parent = *child, parent = child;
            child = lo + (parent - lo) * 2 + 1;
        }
        *(lo + (parent - lo)) = t;
    }
}

/*

2.4 - Comb Sort

Best: O(N)       Average: O(N^2/(2^p)) for p increments of gap
Worst: O(N^2)     Auxiliary Space: O(1)        Stable?: No

Notes: An improved bubble sort that can often be used to even
replace O(N*log(N)) sorts because it is so simple to memorize.

*/

template<class RAI> void combsort(RAI lo, RAI hi) {
    int gap = hi - lo, swapped = 1;
    while (gap > 1 || swapped) {
        if (gap > 1) gap = (int)((float)gap/1.3f);
        swapped = 0;
        for (RAI i = lo; i + gap < hi; i++)
            if (*(i + gap) < *i) {
                std::swap(*i, *(i + gap));
                swapped = 1;
            }
    }
}

/*

2.5 - Radix Sort (32-bit Signed Integers Only!)

Worst: O(d*N), where d is the number of bits/digits in each of the N values.
Since d is constant (32), we can say that radix sort runs in linear time.

Auxiliary Space: O(d)
Stable?: The LSD (least significant digit) radix sort is stable, but because
std::stable_partition() is slower than the unstable std::partition(), the
MSD (most significant digit) radix sort is much faster. Both are given here,
with the MSD version implemented recursively.

Notes: Radix sort usually employs a bucket sort or counting sort.
Here, we take advantage of the partition functions found in the C++ library.

*/

struct radix_comp { //UnaryPredicate for std::partition
    const int bit;  //bit position [0..31] to examine
    radix_comp(int offset) : bit(offset) { }
    bool operator()(int value) const {
        return (bit==31) ? (value<0) : !(value&(1<<bit));
    }
};

void lsd_radix_sort(int *lo, int *hi) {
    for (int lsb = 0; lsb < 32; ++lsb)
        std::stable_partition(lo, hi, radix_comp(lsb));
}

void msd_radix_sort(int *lo, int *hi, int msb = 31) {
    if (lo == hi || msb < 0) return;
    int *mid = std::partition(lo, hi, radix_comp(msb--));
    msd_radix_sort(lo, mid, msb); //sort left partition
    msd_radix_sort(mid, hi, msb); //sort right partition
}
