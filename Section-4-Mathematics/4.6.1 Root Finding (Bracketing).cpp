/*

Finds an x in an interval [a, b] for a continuous function f such that f(x) = 0.
By the intermediate value theorem, a root must exist in [a, b] if the signs of
f(a) and f(b) differ. The answer is found with an absolute error of roughly
1/(2^n), where n is the number of iterations. Although it is possible to control
the error by looping while b - a is greater than an arbitrary epsilon, it is
simpler to let the loop run for a desired number of iterations until floating
point arithmetic break down. 100 iterations is usually sufficient, since the
search space will be reduced to 2^-100 (roughly 10^-30) times its original size.

- bisection_root(f, a, b) returns a root in an interval [a, b] for a continuous
  function f where sgn(f(a)) != sgn(f(b)), using the bisection method.
- falsi_root(f, a, b) returns a root in an interval [a, b] for a continuous
  function f where sgn(f(a)) != sgn(f(b)), using the Illinois algorithm variant
  of the false position (a.k.a. regula falsi) method.

Time Complexity:
- O(n) calls will be made to f() in bisection_root() and falsi_illinois_root(),
  where n is the number of iterations performed.

Space Complexity:
- O(1) auxiliary space for both operations.

*/

#include <stdexcept>

template<class ContinuousFunction>
double bisection_root(ContinuousFunction f, double a, double b,
                      const int ITERATIONS = 100) {
  if (a > b || f(a)*f(b) > 0) {
    throw std::runtime_error("Must give [a, b] where sgn(f(a)) != sgn(f(b)).");
  }
  double m;
  for (int i = 0; i < ITERATIONS; i++) {
    m = a + (b - a)/2;
    if (f(a)*f(m) >= 0) {
      a = m;
    } else {
      b = m;
    }
  }
  return m;
}

template<class ContinuousFunction>
double falsi_illinois_root(ContinuousFunction f, double a, double b,
                           const int ITERATIONS = 100) {
  if (a > b || f(a)*f(b) > 0) {
    throw std::runtime_error("Must give [a, b] where sgn(f(a)) != sgn(f(b)).");
  }
  double m, fm, fa = f(a), fb = f(b);
  int side = 0;
  for (int i = 0; i < ITERATIONS; i++) {
    m = (fa*b - fb*a)/(fa - fb);
    fm = f(m);
    if (fb*fm > 0) {
      b = m;
      fb = fm;
      if (side < 0) {
        fa /= 2;
      }
      side = -1;
    } else if (fa*fm > 0) {
      a = m;
      fa = fm;
      if (side > 1) {
        fb /= 2;
      }
      side = 1;
    } else {
      break;
    }
  }
  return m;
}

/*** Example Usage ***/

#include <cassert>
#include <cmath>

double f(double x) {
  return x*x - 4*sin(x);
}

int main() {
  assert(fabs(f(bisection_root(f, 1, 3))) < 1e-10);
  assert(fabs(f(falsi_illinois_root(f, 1, 3))) < 1e-10);
  return 0;
}
