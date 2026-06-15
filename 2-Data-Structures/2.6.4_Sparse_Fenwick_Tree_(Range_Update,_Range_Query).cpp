/*

Maintain a numerical array over a huge index range while supporting range increments and range-sum
queries. This is the sparse version of the two-tree Fenwick range-update/range-query trick: only
Fenwick nodes reached by previous operations are stored, using `std::unordered_map` instead of dense
vectors.

- `SparseFenwickRURQ<T, N>()` constructs an array with 0-based indices 0 to `N - 1` (inclusive),
  implicitly initialized to 0.
- `at(i)` returns the value at index `i`.
- `add(i, x)` adds `x` to the value at index `i`.
- `add(lo, hi, x)` adds `x` to the values at all indices from `lo` to `hi`, inclusive.
- `set(i, x)` assigns the value at index `i` to `x`.
- `sum(hi)` returns the sum of all values at indices from 0 to `hi`, inclusive.
- `sum(lo, hi)` returns the sum of all values at indices from `lo` to `hi`, inclusive.
- `max_prefix(c)` returns the largest boundary `hi` such that `sum(0, hi - 1) <= c`, assuming
  prefix sums are nondecreasing. It may return any value from 0 to `N`.

Time Complexity:
- O(log N) per call to all member functions.

Space Complexity:
- O(n log n) for storage of the array elements, where $n$ is the number of distinct indices that
  have been accessed across all of the operations so far.
- O(1) auxiliary for all operations.

*/

#include <unordered_map>

template<class T, int N = 1000000001>
class SparseFenwickRURQ {
  std::unordered_map<int, T> tmul, tadd;

  T get(const std::unordered_map<int, T> &tree, int i) const {
    if (auto it = tree.find(i); it != tree.end()) {
      return it->second;
    }
    return 0;
  }

  void add_helper(int at, const T &mul, const T &add) {
    for (int i = at; i <= N; i += i & -i) {
      tmul[i] += mul;
      tadd[i] += add;
    }
  }

 public:
  void add(int lo, int hi, const T &x) {
    lo++;
    hi++;
    add_helper(lo, x, x * (lo - 1));
    add_helper(hi + 1, -x, -x * hi);
  }

  void add(int i, const T &x) { add(i, i, x); }
  void set(int i, const T &x) { add(i, x - at(i)); }

  T sum(int hi) {
    T mul = 0, add = 0;
    hi++;
    for (int i = hi; i > 0; i -= i & -i) {
      mul += get(tmul, i);
      add += get(tadd, i);
    }
    return mul * hi - add;
  }

  T sum(int lo, int hi) { return sum(hi) - sum(lo - 1); }
  T at(int i) { return sum(i, i); }

  int max_prefix(T c) {
    T mul = 0, add = 0;
    int pos = 0, pw = 1;
    while (pw * 2 <= N) {
      pw *= 2;
    }
    for (; pw > 0; pw >>= 1) {
      int next = pos + pw;
      if (next <= N) {
        int key = next;
        T nmul = mul + get(tmul, key), nadd = add + get(tadd, key);
        if (key * nmul - nadd <= c) {
          mul = nmul;
          add = nadd;
          pos = next;
        }
      }
    }
    return pos;
  }
};

/*** Example Usage and Output:

Values: 15 6 7 -5 4

***/

#include <cassert>
#include <iostream>
#include <vector>
using namespace std;

int main() {
  vector<int> a{10, 1, 2, 3, 4};
  SparseFenwickRURQ<int> t;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    t.set(i, a[i]);
  }
  t.add(0, 2, 5);
  t.set(3, -5);
  cout << "Values: ";
  for (int i = 0; i < 5; i++) {
    cout << t.at(i) << " ";
  }
  cout << endl;
  assert(t.sum(0, 4) == 27);
  t.add(500000001, 500000010, 3);
  t.add(500000011, 500000015, 5);
  t.set(500000000, 10);
  assert(t.sum(0, 1000000000) == 92);

  SparseFenwickRURQ<long long> freq;
  freq.add(1, 1, 1);
  freq.add(3, 3, 3);
  freq.add(6, 6, 1);
  assert(freq.max_prefix(0) == 1);
  assert(freq.max_prefix(3) == 3);
  assert(freq.max_prefix(4) == 6);
  return 0;
}
