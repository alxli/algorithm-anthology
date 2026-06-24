/*

Given a range of $n$ numerical elements, reassign each element to an integer in the domain $[0, k)$,
where $k$ is the number of distinct elements in the original range, while preserving the initial
relative ordering of elements. That is, if $a$ is the array of original values and $b$ is the array
of compressed values, then every pair of indices $i, j$ in $[0, n)$ shall satisfy $a[i] < a[j]$ if
and only if $b[i] < b[j]$.

All implementations below take ranges as ForwardIterators and require `operator<` on the value
type. The two `compress()` functions rewrite a range in place and then discard the mapping.

- `compress1(lo, hi)` performs the compression by sorting the array, removing duplicates, and binary
  searching for the position of each original value.
- `compress2(lo, hi)` achieves the same result by inserting all values into an `std::map`, which
  automatically removes duplicate values and supports efficient lookups of the compressed values.

`CoordinateCompressor` is a class version that retains the sorted table of distinct values so that
arbitrary values can be mapped to and from compressed ranks long after construction (e.g. for
offline queries arriving separately from the array being compressed).

- `CoordinateCompressor<T>(lo, hi)` constructs a compressor over the values of $[`lo`, `hi`)$, ready
  for querying. Alternatively, use the default constructor and register values with `add(x)`, then
  call `build()` once after all values are registered and before the first query.
- `size()` returns the number of distinct registered values $k$.
- `value(r)` returns the original value with rank `r`, inverting `rank()`.
- `rank(x)` returns the compressed value (rank) of `x` in $[0, k)$. `x` must have been registered.
- `contains(x)` returns whether `x` is a registered value.

Time Complexity:
- O(n log n) per call to `compress1(lo, hi)`, `compress2(lo, hi)`, and
  `CoordinateCompressor(lo, hi)`, where $n$ is the distance between `lo` and `hi`.
- O(1) amortized per call to `add(x)`, and O(m log m) per call to `build()`, where $m$ is the total
  number of values registered.
- O(log k) per call to `rank(x)` and `contains(x)`, where $k$ is the number of distinct values.
- O(1) per call to `value(r)` and `size()`.

Space Complexity:
- O(n) auxiliary heap space for `compress1()` and `compress2()`.
- O(k) storage for `CoordinateCompressor`.

*/

#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <vector>

template<typename It>
void compress1(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> v(lo, hi);
  std::sort(v.begin(), v.end());
  v.resize(std::unique(v.begin(), v.end()) - v.begin());
  for (It it = lo; it != hi; ++it) {
    *it = static_cast<int>(std::lower_bound(v.begin(), v.end(), *it) - v.begin());
  }
}

template<typename It>
void compress2(It lo, It hi) {
  using T = typename std::iterator_traits<It>::value_type;
  std::map<T, int> m;
  for (It it = lo; it != hi; ++it) {
    m[*it] = 0;
  }
  int id = 0;
  for (auto &[key, val] : m) {
    val = id++;
  }
  for (It it = lo; it != hi; ++it) {
    *it = m[*it];
  }
}

template<typename T>
class CoordinateCompressor {
  std::vector<T> values;

 public:
  CoordinateCompressor() {}

  template<typename It>
  CoordinateCompressor(It lo, It hi) : values(lo, hi) {
    build();
  }

  int size() const { return static_cast<int>(values.size()); }
  const T &value(int r) const { return values[r]; }
  void add(const T &x) { values.push_back(x); }

  void build() {
    std::sort(values.begin(), values.end());
    values.resize(std::unique(values.begin(), values.end()) - values.begin());
  }

  int rank(const T &x) const {
    int r = static_cast<int>(std::lower_bound(values.begin(), values.end(), x) - values.begin());
    assert(r < size() && !(x < values[r]));  // x must be registered.
    return r;
  }

  bool contains(const T &x) const { return std::binary_search(values.begin(), values.end(), x); }
};

/*** Example Usage and Output:

0 4 4 1 3 2 5 5
0 4 4 1 3 2 5 5
1 0 2 0 3 1

***/

#include <iostream>
using namespace std;

template<typename It>
void print_range(It lo, It hi) {
  while (lo != hi) {
    cout << *lo++ << " ";
  }
  cout << endl;
}

int main() {
  {
    vector<int> a{1, 30, 30, 7, 9, 8, 99, 99};
    compress1(a.begin(), a.end());
    print_range(a.begin(), a.end());
  }
  {
    vector<int> a{1, 30, 30, 7, 9, 8, 99, 99};
    compress2(a.begin(), a.end());
    print_range(a.begin(), a.end());
  }
  {  // Non-integral types work too, as long as ints can be assigned to them.
    vector<double> a{0.5, -1.0, 3, -1.0, 20, 0.5};
    compress1(a.begin(), a.end());
    print_range(a.begin(), a.end());
  }
  {
    vector<int> a{10, 5, 30, 5, 20};
    CoordinateCompressor<int> cc(a.begin(), a.end());
    assert(cc.size() == 4);
    assert(cc.rank(5) == 0);
    assert(cc.rank(30) == 3);
    assert(cc.value(2) == 20);
    assert(cc.contains(10) && !cc.contains(7));
  }
  {  // Register values incrementally, then build once before querying.
    CoordinateCompressor<double> cc;
    cc.add(0.5);
    cc.add(-1.0);
    cc.add(0.5);
    cc.build();
    assert(cc.size() == 2);
    assert(cc.rank(0.5) == 1);
    assert(cc.value(0) == -1.0);
  }
  return 0;
}
