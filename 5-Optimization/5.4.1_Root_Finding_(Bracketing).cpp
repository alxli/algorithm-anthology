/*

Finds an $x$ in an interval $[a, b]$ for a continuous function $f$ such that $f(x) = 0$. By the
intermediate value theorem, a root must exist in $[a, b]$ if either endpoint is a root or the signs
of $f(a)$ and $f(b)$ differ. Each bisection step evaluates the midpoint of the interval and keeps
the half whose endpoints still differ in sign, so a root always remains bracketed. After $n$
iterations, the interval width is $(b - a) / 2^n$. Although it is possible to control the error by
looping while $b - a$ is greater than an arbitrary epsilon, it is simpler to let the loop run for a
desired number of iterations until floating point arithmetic breaks down. 100 iterations is usually
sufficient, since the search space will be reduced to $2^{-100}$ (roughly $10^{-30}$) times its
original size.

- `bisection_root(f, a, b, iterations = 100)` returns a root in an interval $[`a`, `b`]$ for a
  continuous function $f$ where either endpoint is a root or the endpoint values have opposite
  signs, using the bisection method.
- `falsi_illinois_root(f, a, b, iterations = 100)` returns a root in an interval $[`a`, `b`]$ for a
  continuous function $f$ under the same endpoint conditions, using the Illinois algorithm variant
  of the false position (a.k.a. regula falsi) method.

Time Complexity:
- O(n) calls will be made to `f()` in `bisection_root()` and `falsi_illinois_root()`, where $n$ is
  the number of iterations performed.

Space Complexity:
- O(1) auxiliary for both operations.

*/

#include <cassert>

template<typename Fn>
double bisection_root(Fn f, double a, double b, const int iterations = 100) {
  double fa = f(a), fb = f(b);
  assert(a <= b && ((fa <= 0 && fb >= 0) || (fa >= 0 && fb <= 0)));
  if (fa == 0) {
    return a;
  }
  if (fb == 0) {
    return b;
  }
  double m = a;
  for (int i = 0; i < iterations; i++) {
    m = a + (b - a) / 2;
    double fm = f(m);
    if (fm == 0) {
      return m;
    }
    if ((fa < 0) == (fm < 0)) {
      a = m;
      fa = fm;
    } else {
      b = m;
    }
  }
  return m;
}

template<typename Fn>
double falsi_illinois_root(Fn f, double a, double b, const int iterations = 100) {
  double fa = f(a), fb = f(b);
  assert(a <= b && ((fa <= 0 && fb >= 0) || (fa >= 0 && fb <= 0)));
  if (fa == 0) {
    return a;
  }
  if (fb == 0) {
    return b;
  }
  double m = a;
  int side = 0;
  for (int i = 0; i < iterations; i++) {
    m = (fa * b - fb * a) / (fa - fb);
    double fm = f(m);
    if (fm == 0) {
      break;
    }
    if ((fb < 0) == (fm < 0)) {
      b = m;
      fb = fm;
      if (side < 0) {
        fa /= 2;
      }
      side = -1;
    } else {
      a = m;
      fa = fm;
      if (side > 0) {
        fb /= 2;
      }
      side = 1;
    }
  }
  return m;
}

/*** Example Usage ***/

#include <cmath>

double f(double x) {
  return x * x - 4 * sin(x);
}

int main() {
  assert(fabs(f(bisection_root(f, 1, 3))) < 1e-10);
  assert(fabs(f(falsi_illinois_root(f, 1, 3))) < 1e-10);
  assert(bisection_root(f, 0, 1) == 0);
  assert(falsi_illinois_root(f, 0, 1) == 0);
  return 0;
}
