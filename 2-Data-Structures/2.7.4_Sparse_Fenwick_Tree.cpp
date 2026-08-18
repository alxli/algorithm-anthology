/*

Maintain a numerical array over a huge index range while supporting range increments and range-sum
queries. This is the sparse version of the two-tree Fenwick range-update/range-query trick: only
Fenwick nodes reached by previous operations are stored, using `std::unordered_map` instead of dense
vectors.

- `SparseFenwick<T, N>()` constructs an array with 0-based indices $[0, `N`)$, implicitly
  initialized to $0$.
- `add(i, x)` adds `x` to the value at index `i`.
- `add(lo, hi, x)` adds `x` to the values at all indices in $[`lo`, `hi`]$.
- `set(i, x)` assigns the value at index `i` to `x`.
- `at(i)` returns the value at index `i`.
- `sum(hi)` returns the sum of all values at indices $[0, `hi`]$.
- `sum(lo, hi)` returns the sum of all values at indices $[`lo`, `hi`]$.
- `max_prefix(c)` returns the largest boundary `hi` such that `sum(0, hi - 1)` $\leq$ `c`, assuming
  prefix sums are nondecreasing and `c` is nonnegative. It may return any boundary in $[0, `N`]$.

The value type `T` must represent $0$ and support addition, subtraction, and multiplication by an
integer index.

Overflow warning: all products of values by indices up to `N`, and all resulting sums, must fit in
`T`.

Time Complexity:
- O(log N) expected per call to all member functions.

Space Complexity:
- O(q log N) expected for storage after $q$ updates.
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <unordered_map>

template<typename T, int N = 1000000001>
class SparseFenwick {
  static_assert(0 < N && N < (1 << 30));

  std::unordered_map<int, T> tmul, tadd;

  T get(const std::unordered_map<int, T> &tree, int i) const {
    if (auto it = tree.find(i); it != tree.end()) {
      return it->second;
    }
    return 0;
  }

  // Adds the linear correction mul*i + add to every index from at onward.
  void add_suffix(int at, const T &mul, const T &add) {
    assert(1 <= at && at <= N + 1);
    for (int i = at; i <= N; i += i & -i) {
      tmul[i] += mul;
      tadd[i] += add;
    }
  }

 public:
  void add(int i, const T &x) { add(i, i, x); }

  void add(int lo, int hi, const T &x) {
    assert(0 <= lo && lo <= hi && hi < N);
    lo++;
    hi++;
    add_suffix(lo, x, x * (lo - 1));  // Overflow warning.
    add_suffix(hi + 1, -x, -x * hi);
  }

  void set(int i, const T &x) { add(i, x - at(i)); }
  T at(int i) const { return sum(i, i); }

  T sum(int hi) const {
    assert(-1 <= hi && hi < N);
    T mul = 0, add = 0;
    hi++;
    for (int i = hi; i > 0; i -= i & -i) {
      mul += get(tmul, i);
      add += get(tadd, i);
    }
    return mul * hi - add;
  }

  T sum(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi < N);
    return sum(hi) - sum(lo - 1);
  }

  int max_prefix(T c) const {
    T mul = 0, add = 0;
    int pos = 0, pw = 1;
    while (pw * 2 <= N) {
      pw *= 2;
    }
    for (; pw > 0; pw >>= 1) {
      int next = pos + pw;
      if (next <= N) {
        T nmul = mul + get(tmul, next), nadd = add + get(tadd, next);
        if (next * nmul - nadd <= c) {
          mul = nmul;
          add = nadd;
          pos = next;
        }
      }
    }
    return pos;
  }
};

/*** Example Usage ***/

#include <vector>
using namespace std;

int main() {
  vector<int> a{10, 1, 2, 3, 4};
  SparseFenwick<long long> t;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    t.set(i, a[i]);
  }
  t.add(0, 2, 5);
  t.set(3, -5);
  vector<int> expected{15, 6, 7, -5, 4};
  for (int i = 0; i < 5; i++) {
    assert(t.at(i) == expected[i]);
  }
  assert(t.sum(0, 4) == 27);
  t.add(500000001, 500000010, 3);
  t.add(500000011, 500000015, 5);
  t.set(500000000, 10);
  assert(t.sum(0, 1000000000) == 92);

  SparseFenwick<long long> freq;
  freq.add(1, 1, 1);
  freq.add(3, 3, 3);
  freq.add(6, 6, 1);
  assert(freq.max_prefix(0) == 1);
  assert(freq.max_prefix(3) == 3);
  assert(freq.max_prefix(4) == 6);
  return 0;
}
