/*

1.2.2 - Ternary Search

Given a unimodal function f(x), find its maximum or minimum.

Assume we are looking for a maximum of f(x) and that we know
the maximum lies somewhere between A and B. For the algorithm
to be applicable, there must be some value x such that:
>>> for all a,b with A <= a < b <= x, we have f(a) < f(b); and
>>> for all a,b with x <= a < b <= B, we have f(a) > f(b).

In other words, for the graph of f(x) must look like a "valley"
with a single point at its trough (lowest point) and no flat
regions on the left side or right side for a ternary search to
be able to find its minimum point. Similarly the graph of f(x)
must look like a "hill" with a single peak for a ternary search
to able able to find its maximum point.

The answer for both functions will be really close to the peak
or trough by no further than +/-epsilon (0.00000001 by default).

*/

template<class UnimodalFunction>
double ternary_search_min(double lo, double hi,
                          UnimodalFunction f, double eps = 1E-8) {
  double lthird, hthird;
  while (hi - lo > eps) {
    lthird = lo + (hi - lo)/3;
    hthird = hi - (hi - lo)/3;
    if (f(lthird) < f(hthird)) hi = hthird;
    else lo = lthird;
  }
  return lo;
}

template<class UnimodalFunction>
double ternary_search_max(double lo, double hi,
                          UnimodalFunction f, double eps = 1E-8) {
  double lthird, hthird;
  while (hi - lo > eps) {
    lthird = lo + (hi - lo)/3;
    hthird = hi - (hi - lo)/3;
    if (f(lthird) < f(hthird)) lo = lthird;
    else hi = hthird;
  }
  return lo;
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

//parabola opening up with vertex at (-2, -24)
double f1(double x) { return 3*x*x + 12*x - 12; }

//parabola opening down with vertex at (0.105, 88.1)
double f2(double x) { return -5.7*x*x + 1.2*x + 88; }

int main() {
  cout << ternary_search_min(-99, 99, f1) << endl; //-2
  cout << ternary_search_max(-99, 99, f2) << endl; //0.105263
  return 0;
}
