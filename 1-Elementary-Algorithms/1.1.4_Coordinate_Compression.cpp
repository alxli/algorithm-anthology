/*

Given a range of $n$ numerical elements, reassign each element to an integer in the domain $[0, k)$,
where $k$ is the number of distinct elements in the original range, while preserving the initial
relative ordering of elements. That is, if $a$ is the array of original values and $b$ is the array
of compressed values, then every pair of indices $i, j$ in $[0, n)$ shall satisfy $a[i] < a[j]$ if
and only if $b[i] < b[j]$.

All implementations below take ranges as ForwardIterators. The comparator `comp` defines the value
ordering: `comp(a, b)` is true when `a` precedes `b`. The two `compress()` functions rewrite a range
in place and then discard the mapping.

- `compress1(lo, hi, comp = std::less<>())` performs the compression by sorting the array, removing
  duplicates, and binary searching for the position of each original value.
- `compress2(lo, hi, comp = std::less<>())` achieves the same result by inserting all values into an
  `std::map`, which automatically removes duplicate values and supports efficient lookups of the
  compressed values.

`Compressor` is a class version that retains the sorted table of distinct values so that arbitrary
values can be mapped to and from compressed ranks long after construction (e.g. for offline queries
arriving separately from the array being compressed). It uses `std::less<T>` by default; to
customize the ordering, instantiate `Compressor<T, Compare>` and pass the comparator to either
constructor.

- `Compressor<T>()` constructs an empty compressor. Register values with `add(x)`, then call
  `build()` once before the first query.
- `Compressor<T>(lo, hi)` constructs a compressor from the half-open iterator range $[`lo`, `hi`)$.
- `size()` returns the number of distinct registered values $k$.
- `value(r)` returns the original value with rank `r`, inverting `rank()`.
- `contains(x)` returns whether `x` is a registered value.
- `rank(x)` returns the compressed value (rank) of `x` in $[0, k)$. `x` must have been registered.

Time Complexity:
- O(n log n) per call to `compress1(lo, hi)`, `compress2(lo, hi)`, and `Compressor(lo, hi)`, where
  $n$ is the distance between `lo` and `hi`.
- O(1) amortized per call to `add()`, and O(m log m) per call to `build()`, where $m$ is the total
  number of values registered.
- O(log k) per call to `rank()` and `contains()`, where $k$ is the number of distinct values.
- O(1) per call to `size()` and `value()`.

Space Complexity:
- O(n) auxiliary for `compress1()` and `compress2()`.
- O(k) storage for `Compressor`.

*/

#include <algorithm>
#include <cassert>
#include <functional>
#include <iterator>
#include <map>
#include <vector>

template<typename It, typename Compare = std::less<>>
void compress1(It lo, It hi, Compare comp = Compare{}) {
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> v(lo, hi);
  std::sort(v.begin(), v.end(), comp);
  v.erase(
      std::unique(
          v.begin(), v.end(), [&](const T &a, const T &b) { return !comp(a, b) && !comp(b, a); }
      ),
      v.end()
  );
  for (It it = lo; it != hi; ++it) {
    *it = static_cast<int>(std::lower_bound(v.begin(), v.end(), *it, comp) - v.begin());
  }
}

template<typename It, typename Compare = std::less<>>
void compress2(It lo, It hi, Compare comp = Compare{}) {
  using T = typename std::iterator_traits<It>::value_type;
  std::map<T, int, Compare> m(comp);
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

template<typename T, typename Compare = std::less<T>>
class Compressor {
  Compare comp;
  std::vector<T> v;

 public:
  explicit Compressor(Compare comp = Compare{}) : comp(std::move(comp)) {}

  template<typename It>
  Compressor(It lo, It hi, Compare comp = Compare{}) : comp(std::move(comp)), v(lo, hi) {
    build();
  }

  void build() {
    std::sort(v.begin(), v.end(), comp);
    v.erase(
        std::unique(
            v.begin(), v.end(), [&](const T &a, const T &b) { return !comp(a, b) && !comp(b, a); }
        ),
        v.end()
    );
  }

  void add(const T &x) { v.push_back(x); }
  int size() const { return static_cast<int>(v.size()); }
  const T &value(int r) const { return v[r]; }
  bool contains(const T &x) const { return std::binary_search(v.begin(), v.end(), x, comp); }

  int rank(const T &x) const {
    int r = static_cast<int>(std::lower_bound(v.begin(), v.end(), x, comp) - v.begin());
    assert(r < size() && !comp(x, v[r]));  // x must be registered.
    return r;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  {
    vector<int> a{1, 30, 30, 7, 9, 8, 99, 99};
    compress1(a.begin(), a.end());
    assert((a == vector<int>{0, 4, 4, 1, 3, 2, 5, 5}));
  }
  {
    vector<int> a{1, 30, 30, 7, 9, 8, 99, 99};
    compress2(a.begin(), a.end());
    assert((a == vector<int>{0, 4, 4, 1, 3, 2, 5, 5}));
  }
  {  // Non-integral types work too, as long as ints can be assigned to them.
    vector<double> a{0.5, -1.0, 3, -1.0, 20, 0.5};
    compress1(a.begin(), a.end());
    assert((a == vector<double>{1, 0, 2, 0, 3, 1}));
  }
  {
    vector<int> a{10, 5, 30, 5, 20};
    Compressor<int> cc(a.begin(), a.end());
    assert(cc.size() == 4);
    assert(cc.rank(5) == 0);
    assert(cc.rank(30) == 3);
    assert(cc.value(2) == 20);
    assert(cc.contains(10) && !cc.contains(7));
  }
  {  // Register values incrementally, then build once before querying.
    Compressor<double> cc;
    cc.add(0.5);
    cc.add(-1.0);
    cc.add(0.5);
    cc.build();
    assert(cc.size() == 2);
    assert(cc.rank(0.5) == 1);
    assert(cc.value(0) == -1.0);
  }
  {
    vector<int> a{10, 30, 20, 10};
    Compressor<int, greater<int>> cc(a.begin(), a.end());
    assert(cc.rank(30) == 0 && cc.rank(10) == 2);
    assert(cc.value(1) == 20);
    compress1(a.begin(), a.end(), greater<int>());
    assert((a == vector<int>{2, 0, 1, 2}));
  }
  return 0;
}
