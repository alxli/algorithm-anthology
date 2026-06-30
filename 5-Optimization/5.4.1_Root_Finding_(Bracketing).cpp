/*

Finds an $x$ in an interval $[a, b]$ for a continuous function $f$ such that $f(x) = 0$. By the
intermediate value theorem, a root must exist in $[a, b]$ if the signs of $f(a)$ and $f(b)$ differ.
Each bisection step evaluates the midpoint of the interval and keeps the half whose endpoints still
differ in sign, so a root always remains bracketed. The answer is found with an absolute error of
roughly $1 / 2^n$, where $n$ is the number of iterations. Although it is possible to control the
error by looping while $b - a$ is greater than an arbitrary epsilon, it is simpler to let the loop
run for a desired number of iterations until floating point arithmetic breaks down. 100 iterations
is usually sufficient, since the search space will be reduced to $2^{-100}$ (roughly $10^{-30}$)
times its original size.

- `bisection_root(f, a, b, iterations = 100)` returns a root in an interval $[`a`, `b`]$ for a
  continuous function $f$ where $\operatorname{sgn}(f(a)) \neq \operatorname{sgn}(f(b))$, using the
  bisection method.
- `falsi_illinois_root(f, a, b, iterations = 100)` returns a root in an interval $[`a`, `b`]$ for a
  continuous function $f$ where $\operatorname{sgn}(f(a)) \neq \operatorname{sgn}(f(b))$, using the
  Illinois algorithm variant of the false position (a.k.a. regula falsi) method.

Time Complexity:
- O(n) calls will be made to `f()` in `bisection_root()` and `falsi_illinois_root()`, where $n$ is
  the number of iterations performed.

Space Complexity:
- O(1) auxiliary for both operations.

*/

#include <cassert>

template<typename Fn>
double bisection_root(Fn f, double a, double b, const int iterations = 100) {
  assert(a <= b && f(a) * f(b) <= 0);
  double m, fa = f(a);
  for (int i = 0; i < iterations; i++) {
    m = a + (b - a) / 2;
    double fm = f(m);
    if (fa * fm >= 0) {
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
  assert(a <= b && f(a) * f(b) <= 0);
  double m, fm, fa = f(a), fb = f(b);
  int side = 0;
  for (int i = 0; i < iterations; i++) {
    m = (fa * b - fb * a) / (fa - fb);
    fm = f(m);
    if (fb * fm > 0) {
      b = m;
      fb = fm;
      if (side < 0) {
        fa /= 2;
      }
      side = -1;
    } else if (fa * fm > 0) {
      a = m;
      fa = fm;
      if (side > 0) {
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

#include <cmath>

double f(double x) {
  return x * x - 4 * sin(x);
}

int main() {
  assert(fabs(f(bisection_root(f, 1, 3))) < 1e-10);
  assert(fabs(f(falsi_illinois_root(f, 1, 3))) < 1e-10);
  return 0;
}
