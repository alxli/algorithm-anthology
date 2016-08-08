/*

Not only can binary search be used to find the position of a given element in a
sorted array, it can also be used to find the input value corresponding to any
output value of a monotonic (strictly non-increasing or strictly non-decreasing)
function in O(log n) running time with respect to the domain size. This is a
special case of finding the exact point at which any given monotonic Boolean
function changes from true to false (or vice versa). Unlike searching through an
array, discrete binary search is not restricted by available memory, making it
useful for handling infinitely large search spaces such as the real numbers.

binary_search_first_true() takes two integers lo and hi as boundaries for the
search space [lo, hi) (i.e. including lo, but excluding hi) and returns the
smallest integer k in [lo, hi) for which the predicate pred(k) tests true. If
pred(k) tests false for every k in [lo, hi), then hi is returned. This function
must be used on a range where there exists a constant k such that pred(x) is
false for every x in [lo, k) and true for every x in [k, hi).

binary_search_last_true() takes two integers lo and hi as boundaries for the
search space [lo, hi) (i.e. including lo, but excluding hi) and returns the
largest integer k in [lo, hi) for which the predicate pred(k) tests true. If
pred(k) tests false for every k in [lo, hi), then hi is returned. This function
must be used on a range where there exists a constant k such that pred(x) is
true for every x in [lo, k] and false for every x in (k, hi).

Time Complexity: At most O(log n) calls will be made to pred(), where n is the
distance between lo and hi.

Space Complexity: O(1) auxiliary.

*/

// 000[1]11
template<class Int, class IntPredicate>
Int binary_search_first_true(Int lo, Int hi, IntPredicate pred) {
  Int mid, _hi = hi;
  while (lo < hi) {
    mid = lo + (hi - lo)/2;
    if (pred(mid))
      hi = mid;
    else
      lo = mid + 1;
  }
  if (!pred(lo))
    return _hi;  // All false.
  return lo;
}

// 11[1]000
template<class Int, class IntPredicate>
Int binary_search_last_true(Int lo, Int hi, IntPredicate pred) {
  Int mid, _hi = hi;
  --hi;
  while (lo < hi) {
    mid = lo + (hi - lo + 1)/2;
    if (pred(mid))
      lo = mid;
    else
      hi = mid - 1;
  }
  if (!pred(lo))
    return _hi;  // All false.
  return lo;
}

/*

fbinary_search() is the equivalent of binary_search_first_true() on floating
point predicates. Since any given interval of reals numbers is dense, it is
clear that the exact target cannot be found. Instead, the function returns a
value that is very close to the border between false and true. The precision of
the answer depends on the number of repetitions the function performs. As each
repetition divides the search space by half, the absolute error of the answer
is therefore 1/(2^r) times the distance between lo and hi if r repetitions are
performed. Although it is possible to control the error by looping while hi - lo
is greater than an arbitrary epsilon, it is simpler to let the loop run for a
desired number of iterations until floating point arithmetic break down. It is
typically sufficient to have 100 iterations, which reduces the search space to
2^-100 ~ 10^-30 times its original size.

The function can be modified to compute the "last true" point in the range by
simply interchanging the assignments of lo and hi in the if-else statements.

Time Complexity: At most O(log n) calls will be made to pred(), where n is the
distance between lo and hi divided by the desired absolute error.

Space Complexity: O(1) auxiliary.

*/

// 000[1]11
template<class DoublePredicate>
double fbinary_search(double lo, double hi, DoublePredicate pred) {
  double mid;
  for (int reps = 0; reps < 100; reps++) {
    mid = (lo + hi)/2.0;
    if (pred(mid))
      hi = mid;
    else
      lo = mid;
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
