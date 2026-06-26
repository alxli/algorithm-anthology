/*

Given a static array, precompute a table that answers range minimum queries in constant time. More
generally, the sparse table works for any range query whose combining operation is both associative
and idempotent, meaning `combine(x, x) == x`. The canonical examples are "min", "max", "gcd", and
the bitwise "and" and "or".

For each level $j$, the entry `dp[j][i]` holds the result of `combine()` over the half-open range
$[i, i + 2^j)$, built by combining two ranges of length $2^{j-1}$. A query over $[`lo`, `hi`]$ is
answered by combining the two overlapping ranges of length $2^j$ that are anchored at `lo` and at
`hi`, where $2^j$ is the largest power of two not exceeding the length of the range. Because these
two ranges overlap whenever the query length is not a power of two, the elements in the overlap are
folded in twice; this is harmless only when `combine()` is idempotent. To support a non-idempotent
but associative operation such as "sum" or "XOR" in constant time, use the disjoint sparse table in
the next section instead, whose ranges meet at a center and never overlap.

The query operation is defined by an associative and idempotent function `combine(a, b)`. The
default code below returns the "min" of the range; for "gcd", `combine(a, b)` should return
`gcd(a, b)`.

- `SparseTable<T>(lo, hi)` builds the table from two random-access iterators.
- `size()` returns the size of the array.
- `query(lo, hi)` returns `combine()` applied to all indices from `lo` to `hi`, inclusive.

Time Complexity:
- O(n log n) per call to the constructor, where $n$ is the size of the array.
- O(1) per call to `size()` and `query()`.

Space Complexity:
- O(n log n) for storage of the table.
- O(1) auxiliary for `query()`.

*/

#include <algorithm>
#include <cassert>
#include <utility>
#include <vector>

template<typename T>
class SparseTable {
  static T combine(const T &a, const T &b) { return std::min(a, b); }

  int len;
  std::vector<int> log2;  // Can also use 31 - __builtin_clz(n) instead of precomputing log2[n].
  std::vector<std::vector<T>> dp;  // dp[j][i] = combine() over [i, i + 2^j).

 public:
  template<typename It>
  SparseTable(It lo, It hi) {
    std::vector<T> a(lo, hi);
    len = static_cast<int>(a.size());
    log2.assign(len + 1, 0);
    for (int i = 2; i <= len; i++) {
      log2[i] = log2[i >> 1] + 1;
    }
    dp.assign(log2[len] + 1, std::vector<T>(len));
    dp[0] = std::move(a);
    for (int j = 1; (1 << j) <= len; j++) {
      for (int i = 0; i + (1 << j) <= len; i++) {
        dp[j][i] = combine(dp[j - 1][i], dp[j - 1][i + (1 << (j - 1))]);
      }
    }
  }

  int size() const { return len; }

  T query(int lo, int hi) const {
    assert(0 <= lo && lo <= hi && hi < len);
    int j = log2[hi - lo + 1];
    return combine(dp[j][lo], dp[j][hi - (1 << j) + 1]);
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{6, -2, 1, 8, 10};
  SparseTable<int> t(a.begin(), a.end());
  assert(t.size() == 5);
  assert(t.query(0, 3) == -2);
  assert(t.query(2, 4) == 1);
  assert(t.query(3, 3) == 8);
  return 0;
}
