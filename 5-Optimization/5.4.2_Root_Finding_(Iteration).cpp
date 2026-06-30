/*

Finds an $x$ for a continuous function $f$ such that $f(x) = 0$ using iterative approximation by an
initial guess that is close to the answer. Each step follows the tangent line at the current guess
down to its $x$-intercept, that is, $x \leftarrow x - f(x)/f'(x)$. Newton's method requires an
explicit definition of the function's derivative while the secant method starts with two initial
guesses and approximates the derivative using the secant slope from the previous iteration. For
$n$ iterations and a good initial guess, the methods below compute approximately $2^n$ digits of
precision, with the secant method converging approximately $1.6$ times slower than Newton's.

- `newton_root(f, fprime, x0, eps = 1e-15, iterations = 100)` returns a root $x$ for a function `f`
  with derivative `fprime` using an initial guess `x0` which should be relatively close to $x$.
- `secant_root(f, x0, x1, eps = 1e-15, iterations = 100)` returns a root $x$ for a function `f`
  using two initial guesses `x0` and `x1` which should be relatively close to $x$.

Time Complexity:
- O(n) calls will be made to `f()` in `newton_root()` and `secant_root()`, where $n$ is the number
  of iterations performed.

Space Complexity:
- O(1) auxiliary for both operations.

*/

#include <cmath>
#include <stdexcept>

template<typename Fn>
double newton_root(
    Fn f, Fn fprime, double x0, const double eps = 1e-15, const int iterations = 100
) {
  double x = x0, error = eps + 1;
  for (int i = 0; error > eps && i < iterations; i++) {
    double xnew = x - f(x) / fprime(x);
    error = fabs(xnew - x);
    x = xnew;
  }
  if (error > eps) {
    throw std::runtime_error("Newton's method failed to converge.");
  }
  return x;
}

template<typename Fn>
double secant_root(
    Fn f, double x0, double x1, const double eps = 1e-15, const int iterations = 100
) {
  double xold = x0, fxold = f(x0), x = x1, error = eps + 1;
  for (int i = 0; error > eps && i < iterations; i++) {
    double fx = f(x);
    double xnew = x - fx * ((x - xold) / (fx - fxold));
    xold = x;
    fxold = fx;
    error = fabs(xnew - x);
    x = xnew;
  }
  if (error > eps) {
    throw std::runtime_error("Secant method failed to converge.");
  }
  return x;
}

/*** Example Usage ***/

#include <cassert>

double f(double x) {
  return x * x - 4 * sin(x);
}

double fprime(double x) {
  return 2 * x - 4 * cos(x);
}

int main() {
  assert(fabs(f(newton_root(f, fprime, 3))) < 1e-10);
  assert(fabs(f(secant_root(f, 3, 2))) < 1e-10);
  return 0;
}
