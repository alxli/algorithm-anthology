/*

Binary search can be generally used to find the input value corresponding to any output value of a
monotonic (strictly non-increasing or strictly non-decreasing) function in O(log n) time with
respect to the domain size. This is a special case of finding the exact point at which any given
monotonic Boolean function changes from true to false or vice versa. Each step evaluates the
midpoint of the current interval and keeps the half that still contains the transition point,
halving the search space until it closes on the answer. Unlike searching through an array, discrete
binary search is not restricted by available memory, making it useful for handling infinitely large
search spaces such as real number intervals.

- `binary_search_first_true(lo, hi, pred)` takes integer boundaries for the search space
  [`lo`, `hi`) (i.e. including `lo`, but excluding `hi`) and returns the smallest integer `k` in
  [`lo`, `hi`) for which the predicate `pred(k)` tests true. If `pred(k)` tests false for the entire
  input range, then `hi` is returned. The caller must ensure `pred` is monotonic on the input range,
  i.e. returning all false for some (possibly empty) prefix, followed by all true in some (possibly
  empty) suffix. E.g., patterns `"01"`, `"00"`, and `"11"` are allowed, but `"10"` is disallowed.
- `binary_search_last_true(lo, hi, pred)` takes integer boundaries for the search space [`lo`, `hi`)
  (i.e. including `lo`, but excluding `hi`) and returns the largest integer `k` in [`lo`, `hi`) for
  which the predicate `pred(k)` tests true. If `pred(k)` tests false for the entire input range,
  then `hi` is returned. The caller must ensure `pred` is monotonic on the input range, i.e.
  returning all true for some (possibly empty) prefix, followed by all false in some (possibly
  empty) suffix. E.g., patterns `"10"`, `"00"`, and `"11"` are allowed, but `"01"` is disallowed.
- `fbinary_search(lo, hi, pred)` is the equivalent of `binary_search_first_true()` on floating point
  predicates. Since any interval of real numbers is dense, the exact target cannot be found due to
  floating point error. Instead, a value that is very close to the border between false and true is
  returned.
  The precision of the answer depends on the number of repetitions the function performs. Since each
  repetition bisects the search space, the absolute error of the answer is $1/(2^n)$ times the
  distance between `lo` and `hi` after $n$ repetitions. Although it is possible to control the error
  by looping until the distance shrinks to an arbitrary epsilon, it is simpler to let the loop run
  for a desired number of iterations until floating point arithmetic breaks down. 100 iterations
  is usually sufficient, since the search space will be reduced to $2^{-100}$ (roughly $10^{-30}$)
  times its original size. This implementation can be modified to find the "last true" point in the
  range by simply interchanging the assignments of `lo` and `hi` in the if-else statements.

Time Complexity:
- O(log n) calls will be made to `pred()` in `binary_search_first_true()` and
  `binary_search_last_true()`, where $n$ is the distance between `lo` and `hi`.
- O(n) calls will be made to `pred()` in `fbinary_search()`, where $n$ is the number of iterations
  performed.

Space Complexity:
- O(1) auxiliary for all operations.

*/

template<class Int, class Pred>
Int binary_search_first_true(Int lo, Int hi, Pred pred) {  // 000[1]11
  while (lo < hi) {
    Int mid = lo + (hi - lo) / 2;
    if (pred(mid)) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }
  return lo;
}

template<class Int, class Pred>
Int binary_search_last_true(Int lo, Int hi, Pred pred) {  // 11[1]000
  Int _hi = hi;
  if (lo == hi) {
    return _hi;
  }
  hi--;
  while (lo < hi) {
    Int mid = lo + (hi - lo + 1) / 2;
    if (pred(mid)) {
      lo = mid;
    } else {
      hi = mid - 1;
    }
  }
  if (!pred(lo)) {
    return _hi;  // All false.
  }
  return lo;
}

template<class Pred>
double fbinary_search(double lo, double hi, Pred pred) {  // 000[1]11
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

int main() {
  assert(binary_search_first_true(0, 7, [](int x) { return x >= 3; }) == 3);
  assert(binary_search_first_true(0, 7, [](int x) { return false; }) == 7);

  assert(binary_search_last_true(0, 7, [](int x) { return x <= 5; }) == 5);
  assert(binary_search_last_true(0, 7, [](int x) { return true; }) == 6);

  double res = fbinary_search(-10.0, 10.0, [](double x) { return x >= 1.2345; });
  assert(fabs(res - 1.2345) < 1e-15);
  return 0;
}
