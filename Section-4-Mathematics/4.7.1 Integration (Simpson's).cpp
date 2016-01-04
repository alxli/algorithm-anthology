/*

Simpson's rule is a method for numerical integration, the
numerical approximation of definite integrals. The rule is:

Integral of f(x) dx from a to b ~=
  [f(a) + 4*f((a + b)/2) + f(b)] * (b - a)/6

*/

#include <cmath> /* fabs() */

template<class DoubleFunction>
double simpsons(DoubleFunction f, double a, double b) {
  return (f(a) + 4 * f((a + b)/2) + f(b)) * (b - a)/6;
}

template<class DoubleFunction>
double integrate(DoubleFunction f, double a, double b) {
  static const double eps = 1e-10;
  double m = (a + b) / 2;
  double am = simpsons(f, a, m);
  double mb = simpsons(f, m, b);
  double ab = simpsons(f, a, b);
  if (fabs(am + mb - ab) < eps) return ab;
  return integrate(f, a, m) + integrate(f, m, b);
}

/*** Example Usage ***/

#include <iostream>
using namespace std;

double f(double x) { return sin(x); }

int main () {
  double PI = acos(-1.0);
  cout << integrate(f, 0.0, PI/2) << "\n"; //1
  return 0;
}
