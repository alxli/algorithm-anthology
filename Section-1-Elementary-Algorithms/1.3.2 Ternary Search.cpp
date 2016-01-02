/*

1.3.2 - Ternary Search

Given a unimodal function f(x), find its maximum or minimum point to a
an arbitrarily specified absolute error.

ternary_search_min() takes the domain [lo, hi] of a continuous function
f(x) and returns a number x such that f is strictly decreasing on the
interval [lo, x] and strictly increasing on the interval [x, hi]. For
ternary search to work, this x must exist and be unique.

ternary_search_max() takes the domain [lo, hi] of a continuous function
f(x) and returns a number x such that f is strictly increasing on the
interval [lo, x] and strictly decreasing on the interval [x, hi]. For
ternary search to work, this x must exist and be unique.

Time Complexity: At most O(log n) calls to f, where n is the distance
between lo and hi divided by the desired absolute error (epsilon).

Space Complexity: O(1) auxiliary.

*/

template<class UnimodalFunction>
double ternary_search_min(double lo, double hi, UnimodalFunction f) {
  static const double EPS = 1e-9;
  double lthird, hthird;
  while (hi - lo > EPS) {
    lthird = lo + (hi - lo) / 3;
    hthird = hi - (hi - lo) / 3;
    if (f(lthird) < f(hthird))
      hi = hthird;
    else
      lo = lthird;
  }
  return lo;
}

template<class UnimodalFunction>
double ternary_search_max(double lo, double hi, UnimodalFunction f) {
  static const double EPS = 1e-9;
  double lthird, hthird;
  while (hi - lo > EPS) {
    lthird = lo + (hi - lo) / 3;
    hthird = hi - (hi - lo) / 3;
    if (f(lthird) < f(hthird))
      lo = lthird;
    else
      hi = hthird;
  }
  return hi;
}

/*** Example Usage ***/

#include <cmath>
#include <cassert>

bool eq(double a, double b) {
  return fabs(a - b) < 1e-9;
}

//parabola opening up with vertex at (-2, -24)
double f1(double x) {
  return 3*x*x + 12*x - 12;
}

//parabola opening down with vertex at (2/19, 8366/95)
double f2(double x) {
  return -5.7*x*x + 1.2*x + 88;
}

//absolute value function shifted to the right by 30 units
double f3(double x) {
  return fabs(x - 30);
}

int main() {
  assert(eq(ternary_search_min(-1000, 1000, f1), -2));
  assert(eq(ternary_search_max(-1000, 1000, f2), 2.0/19));
  assert(eq(ternary_search_min(-1000, 1000, f3), 30));
  return 0;
}
