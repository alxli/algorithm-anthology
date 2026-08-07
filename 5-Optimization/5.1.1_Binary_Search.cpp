/*

Binary search locates a target in sorted data, or more generally the transition point of a monotonic
Boolean predicate that changes from false to true or true to false at most once. Each step evaluates
the midpoint of the current interval and keeps the half that still contains the transition, halving
the search space until it closes on the answer. The same technique applies to non-increasing or
non-decreasing numeric functions. Unlike searching through an array, discrete binary search is not
restricted by available memory, making it useful for implicit domains that are too large to
materialize, including broad integer or real intervals.

- `first_true(lo, hi, pred)` takes signed integer boundaries for the search space $[`lo`, `hi`)$
  (i.e. including `lo`, but excluding `hi`) and returns the smallest integer `k` in $[`lo`, `hi`)$
  for which the predicate `pred(k)` tests true. If `pred(k)` tests false for the entire input range,
  then the original `hi` is returned. The caller must ensure `pred` is monotonically ascending on
  the input range, i.e. returning all false for some (possibly empty) prefix, followed by all true
  in some (possibly empty) suffix. E.g., patterns `01`, `00`, and `11` are allowed, but `10` is
  disallowed.
- `last_true(lo, hi, pred)` takes signed integer boundaries for the search space $[`lo`, `hi`)$
  (i.e. including `lo`, but excluding `hi`) and returns the largest integer `k` in $[`lo`, `hi`)$
  for which the predicate `pred(k)` tests true. If `pred(k)` tests false for the entire input range,
  then the original `lo - 1` is returned, so `lo` must exceed the minimum representable value. The
  caller must ensure `pred` is monotonically descending on the input range, i.e. returning all true
  for some (possibly empty) prefix, followed by all false in some (possibly empty) suffix. E.g.,
  patterns `10`, `00`, and `11` are allowed, but `01` is disallowed.
- `first_true_real(lo, hi, pred)` is the equivalent of `first_true()` on floating point predicates.
  Since any interval of real numbers is dense, the exact target cannot be found due to floating
  point error. Instead, a value that is very close to the border between false and true is returned.
  The precision of the answer depends on the number of repetitions the function performs. Since each
  repetition bisects the search space, the absolute error of the answer is $1/(2^n)$ times the
  distance between `lo` and `hi` after $n$ repetitions. Although the error can be controlled by
  looping until the distance shrinks to an arbitrary epsilon, it is simpler to let the loop run for
  a desired number of iterations until floating point arithmetic breaks down. $100$ iterations is
  usually sufficient, since the search space will be reduced to $2^{-100}$ (roughly $10^{-30}$)
  times its original size. This implementation can be modified to find the "last true" point by
  simply interchanging the assignments of `lo` and `hi` in the if-else statements.

Overflow warning: For both integer searches, `hi - lo` must be representable by `Int`. For example,
$[`INT_MIN`, `INT_MAX`)$ will overflow if passed as signed 32-bit integer boundaries.

Time Complexity:
- O(log n) calls to `pred()` per call to `first_true()` and `last_true()`, where $n$ is the distance
  between `lo` and `hi`.
- O(n) calls to `pred()` per call to `first_true_real()`, where $n$ is the number of iterations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

template<typename Int, typename Pred>
Int first_true(Int lo, Int hi, Pred pred) {  // 000[1]11
  while (lo < hi) {
    Int mid = lo + (hi - lo) / 2;
    if (pred(mid)) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }
  return lo;  // hi if all false
}

template<typename Int, typename Pred>
Int last_true(Int lo, Int hi, Pred pred) {  // 11[1]000
  while (lo < hi) {
    Int mid = lo + (hi - lo) / 2;
    if (pred(mid)) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  return lo - 1;
}

template<typename Pred>
double first_true_real(double lo, double hi, Pred pred) {  // 000[1]11
  double mid;
  for (int i = 0; i < 100; i++) {
    mid = (lo + hi) / 2.0;
    if (pred(mid)) {
      hi = mid;
    } else {
      lo = mid;
    }
  }
  return lo;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>
#include <vector>
using namespace std;

int main() {
  assert(first_true(0, 7, [](int x) { return x >= 3; }) == 3);
  assert(first_true(0, 7, [](int x) { return true; }) == 0);
  assert(first_true(0, 7, [](int x) { return false; }) == 7);
  assert(first_true(4, 4, [](int x) { return x >= 4; }) == 4);

  assert(last_true(0, 7, [](int x) { return x <= 5; }) == 5);
  assert(last_true(0, 7, [](int x) { return true; }) == 6);
  assert(last_true(0, 7, [](int x) { return false; }) == -1);
  assert(last_true(4, 4, [](int x) { return x <= 4; }) == 3);

  vector<int> a{1, 2, 2, 4, 7};
  int n = static_cast<int>(a.size());
  assert(first_true(0, n, [&](int i) { return a[i] >= 2; }) == 1);  // lower_bound(2)
  assert(first_true(0, n, [&](int i) { return a[i] > 2; }) == 3);   // upper_bound(2)

  double res = first_true_real(-10.0, 10.0, [](double x) { return x >= 1.2345; });
  assert(fabs(res - 1.2345) < 1e-15);
  return 0;
}
