/*

Binary search can be generally used to find the input value corresponding to any
output value of a monotonic (strictly non-increasing or strictly non-decreasing)
function in O(log n) time with respect to the domain size. This is a special
case of finding the exact point at which any given monotonic Boolean function
changes from true to false or vice versa. Unlike searching through an array,
discrete binary search is not restricted by available memory, making it useful
for handling infinitely large search spaces such as real number intervals.

- binary_search_first_true() takes two integers lo and hi as boundaries for the
  search space [lo, hi) (i.e. including lo, but excluding hi) and returns the
  smallest integer k in [lo, hi) for which the predicate pred(k) tests true. If
  pred(k) tests false for every k in [lo, hi), then hi is returned. This
  function must be used on a range in which there exists a constant k such that
  pred(x) tests false for every x in [lo, k) and true for every x in [k, hi).
- binary_search_last_true() takes two integers lo and hi as boundaries for the
  search space [lo, hi) (i.e. including lo, but excluding hi) and returns the
  largest integer k in [lo, hi) for which the predicate pred(k) tests true. If
  pred(k) tests false for every k in [lo, hi), then hi is returned. This
  function must be used on a range in which there exists a constant k such that
  pred(x) tests true for every x in [lo, k] and false for every x in (k, hi).
- fbinary_search() is the equivalent of binary_search_first_true() on floating
  point predicates. Since any interval of real numbers is dense, the exact
  target cannot be found due to floating point error. Instead, a value that is
  very close to the border between false and true is returned. The precision of
  the answer depends on the number of repetitions the function performs. Since
  each repetition bisects the search space, the absolute error of the answer is
  1/(2^n) times the distance between lo and hi after n repetitions. Although it
  is possible to control the error by looping while hi - lo is greater than an
  arbitrary epsilon, it is simpler to let the loop run for a desired number of
  iterations until floating point arithmetic break down. 100 iterations is
  usually sufficient, since the search space will be reduced to 2^-100 (roughly
  10^-30) times its original size. This implementation can be modified to find
  the "last true" point in the range by simply interchanging the assignments of
  lo and hi in the if-else statements.

Time Complexity:
- O(log n) calls will be made to pred() in binary_search_first_true() and
  binary_search_last_true(), where n is the distance between lo and hi.
- O(n) calls will be made to pred() in fbinary_search(), where n is the number
  of iterations performed.

Space Complexity:
- O(1) auxiliary for all operations.

*/

template<class Int, class IntPredicate>
Int binary_search_first_true(Int lo, Int hi, IntPredicate pred) {  // 000[1]11
  Int mid, _hi = hi;
  while (lo < hi) {
    mid = lo + (hi - lo)/2;
    if (pred(mid)) {
      hi = mid;
    } else {
      lo = mid + 1;
    }
  }
  if (!pred(lo)) {
    return _hi;  // All false.
  }
  return lo;
}

template<class Int, class IntPredicate>
Int binary_search_last_true(Int lo, Int hi, IntPredicate pred) {  // 11[1]000
  Int mid, _hi = hi--;
  while (lo < hi) {
    mid = lo + (hi - lo + 1)/2;
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

template<class DoublePredicate>
double fbinary_search(double lo, double hi, DoublePredicate pred) {  // 000[1]11
  double mid;
  for (int i = 0; i < 100; i++) {
    mid = (lo + hi)/2.0;
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

// Simple predicate examples.
bool pred1(int x) { return x >= 3; }
bool pred2(int x) { return false; }
bool pred3(int x) { return x <= 5; }
bool pred4(int x) { return true; }
bool pred5(double x) { return x >= 1.2345; }

int main() {
  assert(binary_search_first_true(0, 7, pred1) == 3);
  assert(binary_search_first_true(0, 7, pred2) == 7);
  assert(binary_search_last_true(0, 7, pred3)  == 5);
  assert(binary_search_last_true(0, 7, pred4)  == 6);
  assert(fabs(fbinary_search(-10.0, 10.0, pred5) - 1.2345) < 1e-15);
  return 0;
}
