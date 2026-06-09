/*

Finds the first value where a monotone Boolean predicate becomes true when an upper bound is not
known in advance. Exponential search first grows the search range by powers of two until it brackets
the transition point, then finishes with ordinary binary search.

This is useful for answer-search problems over unbounded or very large integer domains. The
predicate must eventually become true, or the search may overflow or run forever unless an explicit
limit is added.

- `exponential_search_first_true(lo, pred)` returns the smallest integer `x` greater than or equal
  to `lo` such that `pred(x)` is true.

Time Complexity:
- O(log n) calls to `pred()`, where $n$ is the distance from `lo` to the first true value.

Space Complexity:
- O(1) auxiliary.

*/

template<class Int, class Pred>
Int exponential_search_first_true(Int lo, Pred pred) {  // 000[1]11
  if (pred(lo)) {
    return lo;
  }
  Int step = 1;
  while (!pred(lo + step)) {
    step *= 2;
  }
  Int l = lo + step / 2, r = lo + step;
  while (l < r) {
    Int mid = l + (r - l) / 2;
    if (pred(mid)) {
      r = mid;
    } else {
      l = mid + 1;
    }
  }
  return l;
}

/*** Example Usage ***/

#include <cassert>

bool at_least_1000(int x) {
  return x >= 1000;
}

bool square_large_enough(long long x) {
  return x * x >= 123456789LL;
}

int main() {
  assert(exponential_search_first_true(0, at_least_1000) == 1000);
  assert(exponential_search_first_true(5, at_least_1000) == 1000);
  assert(exponential_search_first_true(0LL, square_large_enough) == 11112);
  return 0;
}
