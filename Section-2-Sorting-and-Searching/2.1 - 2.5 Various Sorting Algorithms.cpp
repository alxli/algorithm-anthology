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

