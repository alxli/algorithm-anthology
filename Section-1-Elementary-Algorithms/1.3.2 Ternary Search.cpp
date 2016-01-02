/*

1.3.2 - Ternary Search

Given a unimodal function f(x), find its maximum or minimum point to a
an arbitrarily specified absolute error. A maximum unimodal function
f(x) with domain [lo, hi] means that there must exist a number x such
that for all pairs a, b in [lo, x] with a < b, f(a) < f(b) and that
for all pairs c, d in [x, hi] with c < d, f(c) > f(d). For a minimum
unimodal function, f(a) > f(b) and f(c) < f(d) will instead hold.

In other words, for ternary_search_min() to work on a function f, the
graph of f must resemble a "valley" with a single point as its minimum
and no flat regions on its left side nor right side. Conversely, for
ternary_search_max() to work, the graph of f must look like a "hill"
with a single point as its maximum.

Time Complexity: At most O(log n) calls to f, where n is the distance
between lo and hi divided by the desired absolute error (epsilon).

Space Complexity: O(1) auxiliary.

*/

template<class UnimodalFunction>
double ternary_search_min(double lo, double hi, UnimodalFunction f,
                          double eps = 1e-9) {
  double lthird, hthird;
  while (hi - lo > eps) {
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
double ternary_search_max(double lo, double hi, UnimodalFunction f,
                          double eps = 1e-9) {
  double lthird, hthird;
  while (hi - lo > eps) {
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

//parabola opening up with vertex at (-2, -24)
double f1(double x) {
  return 3*x*x + 12*x - 12;
}

//parabola opening down with vertex at (2/19, 8366/95)
double f2(double x) {
  return -5.7*x*x + 1.2*x + 88;
}

bool eq(double a, double b) {
  return fabs(a - b) < 1e-9;
}

int main() {
  assert(eq(ternary_search_min(-1000, 1000, f1), -2));
  assert(eq(ternary_search_max(-1000, 1000, f2), 2.0/19));
  return 0;
}
