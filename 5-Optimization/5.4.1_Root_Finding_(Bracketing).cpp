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
  continuous function `f` where either endpoint is a root or the endpoint values have opposite
  signs, using the bisection method.
- `falsi_illinois_root(f, a, b, iterations = 100)` returns a root in an interval $[`a`, `b`]$ for a
  continuous function `f` under the same endpoint conditions, using the Illinois algorithm variant
  of the false position (a.k.a. regula falsi) method.
- `brent_root(f, a, b, eps = 1e-15, iterations = 100)` returns a root under the same endpoint
  conditions using Brent's method, stopping early once the bracket is narrower than `eps`.

Brent's method is the one to reach for when the number of evaluations matters. It proposes the root
of the inverse quadratic through its three most recent points, which converges superlinearly on
smooth functions, and it falls back to secant steps when two of those values coincide. Any proposal
that leaves the bracket, or that fails to at least halve the step taken two iterations earlier, is
discarded in favor of a bisection step. That guard is what keeps the guaranteed bracketing of
bisection while retaining the speed of interpolation, so Brent's method is the default root finder
in most numerical libraries.

Time Complexity:
- O(n) calls to `f()` per call to `bisection_root()`, `falsi_illinois_root()`, and `brent_root()`,
  where $n$ is the number of iterations. Brent's method usually returns after far fewer than the
  iteration limit, since it stops once the bracket is narrower than `eps`.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <cmath>

template<typename Fn>
double bisection_root(Fn f, double a, double b, int iterations = 100) {
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
double falsi_illinois_root(Fn f, double a, double b, int iterations = 100) {
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

template<typename Fn>
double brent_root(Fn f, double a, double b, double eps = 1e-15, int iterations = 100) {
  double fa = f(a), fb = f(b);
  assert(a <= b && ((fa <= 0 && fb >= 0) || (fa >= 0 && fb <= 0)));
  if (fa == 0) {
    return a;
  }
  if (fb == 0) {
    return b;
  }
  if (std::fabs(fa) < std::fabs(fb)) {  // Keep b as the better of the two endpoints.
    std::swap(a, b);
    std::swap(fa, fb);
  }
  double c = a, fc = fa, d = c;
  bool bisected = true;
  for (int i = 0; i < iterations && fb != 0 && std::fabs(b - a) > eps; i++) {
    double s;
    if (fa != fc && fb != fc) {
      s = a * fb * fc / ((fa - fb) * (fa - fc)) + b * fa * fc / ((fb - fa) * (fb - fc)) +
          c * fa * fb / ((fc - fa) * (fc - fb));
    } else {
      s = b - fb * (b - a) / (fb - fa);
    }
    // Reject an interpolated point that leaves the bracket or converges too slowly, so that the
    // step count stays within a constant factor of plain bisection.
    double lo = (3 * a + b) / 4, hi = b;
    if (lo > hi) {
      std::swap(lo, hi);
    }
    double last = bisected ? std::fabs(b - c) : std::fabs(c - d);
    if (!(lo < s && s < hi) || std::fabs(s - b) >= last / 2 || last < eps) {
      s = a + (b - a) / 2;
      bisected = true;
    } else {
      bisected = false;
    }
    double fs = f(s);
    d = c;
    c = b;
    fc = fb;
    if ((fa < 0) == (fs < 0)) {
      a = s;
      fa = fs;
    } else {
      b = s;
      fb = fs;
    }
    if (std::fabs(fa) < std::fabs(fb)) {
      std::swap(a, b);
      std::swap(fa, fb);
    }
  }
  return b;
}

/*** Example Usage ***/

#include <cmath>

double f(double x) {
  return x * x - 4 * sin(x);
}

int main() {
  assert(fabs(f(bisection_root(f, 1, 3))) < 1e-10);
  assert(fabs(f(falsi_illinois_root(f, 1, 3))) < 1e-10);
  assert(fabs(f(brent_root(f, 1, 3))) < 1e-10);
  assert(bisection_root(f, 0, 1) == 0);
  assert(falsi_illinois_root(f, 0, 1) == 0);
  assert(brent_root(f, 0, 1) == 0);

  // A function whose flat tail defeats plain false position but not the bisection fallback.
  auto flat = [](double x) { return x * x * x - 2 * x - 5; };
  assert(fabs(flat(brent_root(flat, 2, 100))) < 1e-9);
  return 0;
}
