/*

Computes the definite integral from a to b for a continuous function f using
Simpson's approximation: integral ~ [f(a) + 4*f((a + b)/2) + f(b)]*(b - a)/6.

- simpsons(f, a, b) returns the definite integral for a function f from a to b,
  to a tolerance of EPS in absolute error.

Time Complexity:
- O(p) per call to integrate(), where p = -log10(EPS) is the number of digits
  of absolute precision that is desired.

Space Complexity:
- O(p) auxiliary stack and O(1) auxiliary heap space, where p = -log10(EPS)
  is the number of digits of absolute precision that is desired.

*/

#include <cmath>

template<class ContinuousFunction>
double simpsons(ContinuousFunction f, double a, double b) {
  return (f(a) + 4*f((a + b)/2) + f(b))*(b - a)/6;
}

template<class ContinuousFunction>
double integrate(ContinuousFunction f, double a, double b,
                 const double EPS = 1e-15) {
  double m = (a + b) / 2;
  double am = simpsons(f, a, m);
  double mb = simpsons(f, m, b);
  double ab = simpsons(f, a, b);
  if (fabs(am + mb - ab) < EPS) {
    return ab;
  }
  return integrate(f, a, m) + integrate(f, m, b);
}

/*** Example Usage ***/

#include <cstdio>
#include <cassert>
using namespace std;

double f(double x) {
  return sin(x);
}

int main () {
  double PI = acos(-1.0);
  assert(fabs(integrate(f, 0.0, PI/2) - 1) < 1e-10);
  return 0;
}
