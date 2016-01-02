/*

1.3.1 - Discrete Binary Search

Not only can binary search be used to find the position of a given
element in a sorted array, it can also be used to find the input value
corresponding to any output value of a monotonic (either strictly
non-increasing or strictly non-decreasing) function in O(log n) running
time with respect to the domain. This is a special case of finding
the exact point at which any given monotonic Boolean function changes
from true to false (or vice versa). Unlike searching through an array,
discrete binary search is not restricted by available memory, which is
especially important while handling infinitely large search spaces such
as the real numbers.

binary_search_first_true() takes two integers lo and hi as boundaries
for the search space [lo, hi) (i.e. including lo, but excluding hi),
and returns the least integer k (lo <= k < hi) for which the Boolean
predicate pred(k) tests true. This function is correct if and only if
there exists a constant k where the return value of pred(x) is false
for all x < k and true for all x >= k.

binary_search_last_true() takes two integers lo and hi as boundaries
for the search space [lo, hi) (i.e. including lo, but excluding hi),
and returns the greatest integer k (lo <= k < hi) for which the Boolean
predicate pred(k) tests true. This function is correct if and only if
there exists a constant k where the return value of pred(x) is true
for all x <= k and false for all x > k.

Time Complexity: At most O(log n) calls to pred(), where n is the
distance between lo and hi.

Space Complexity: O(1) auxiliary.

*/

//000[1]11
template<class Int, class IntPredicate>
Int binary_search_first_true(Int lo, Int hi, IntPredicate pred) {
  Int mid, _hi = hi;
  while (lo < hi) {
    mid = lo + (hi - lo) / 2;
    if (pred(mid))
      hi = mid;
    else
      lo = mid + 1;
  }
  if (!pred(lo)) return _hi; //all false
  return lo;
}

//11[1]000
template<class Int, class IntPredicate>
Int binary_search_last_true(Int lo, Int hi, IntPredicate pred) {
  Int mid, _hi = hi;
  while (lo < hi) {
    mid = lo + (hi - lo + 1) / 2;
    if (pred(mid))
      lo = mid;
    else
      hi = mid - 1;
  }
  if (!pred(lo)) return _hi; //all true
  return lo;
}

/*

fbinary_search() is the equivalent of binary_search_first_true() on
floating point predicates. Since any given range of reals numbers is
dense, it is clear that the exact target cannot be found. Instead, the
function will return a value that is very close to the border between
false and true. The precision of the answer depends on the number of
repetitions the function uses. Since each repetition bisects the search
space, for r repetitions, the absolute error of the answer will be
1/(2^r) times the distance between lo and hi. Although it's possible to
control the error by looping while hi - lo is greater than an arbitrary
epsilon, it is much simpler to let the loop run for a sizable number of
iterations until floating point arithmetic breaks down. 100 iterations
is typically sufficient, reducing the search space to 2^-100 ~ 10^-30
times its original size.

Note that the function can be modified to find the "last true" point
in the range by interchanging lo and hi in the if-else statement.

Time Complexity: At most O(log n) calls to pred(), where n is the
distance between lo and hi divided by the desired absolute error.

Space Complexity: O(1) auxiliary.

*/

//000[1]11
template<class DoublePredicate>
double fbinary_search(double lo, double hi, DoublePredicate pred) {
  double mid;
  for (int reps = 0; reps < 100; reps++) {
    mid = (lo + hi) / 2.0;
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

//Simple predicate examples:
bool pred1(int x) { return x >= 3; }
bool pred2(int x) { return false; }
bool pred3(int x) { return x <= 5; }
bool pred4(int x) { return true; }
bool pred5(double x) { return x >= 1.2345; }

int main() {
  assert(binary_search_first_true(0, 7, pred1) == 3);
  assert(binary_search_first_true(0, 7, pred2) == 7);
  assert(binary_search_last_true(0, 7, pred3)  == 5);
  assert(binary_search_last_true(0, 7, pred4)  == 7);
  assert(fabs(fbinary_search(-10.0, 10.0, pred5) - 1.2345) < 1e-15);
  return 0;
}
