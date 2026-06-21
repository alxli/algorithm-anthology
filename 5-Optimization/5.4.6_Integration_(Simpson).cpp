/*

Computes the definite integral from $a$ to $b$ for a continuous function $f$ using the composite
Simpson's rule: the interval is split into `n` equal subintervals and each consecutive pair is
approximated by a parabola, giving the approximation $\int_{a}^{b} f(x) \,dx \approx$
$h/3 \cdot [f(x_0) + 4f(x_1) + 2f(x_2) + \ldots + 4f(x_{n-1}) + f(x_n)]$ with step $h = (b - a)/n$.

- `integrate(f, a, b, n)` returns the definite integral of a function `f` from `a` to `b` using `n`
  subintervals, where `n` must be even. Larger `n` trades runtime for accuracy: the error is
  O((b - a) * h^4), so for a smooth `f` a value of `n` on the order of $10^6$ reaches near double
  precision.

Unlike adaptive quadrature, the step size here is uniform, so a feature narrower than `h` may be
under-resolved. The benefit is a simple, non-recursive routine with predictable cost that is not
fooled by the premature-termination heuristic that adaptive Simpson's rule can suffer on functions
whose coarse sample points happen to fit a parabola.

Time Complexity:
- O(n) per call, requiring $n + 1$ evaluations of $f$.

Space Complexity:
- O(1) auxiliary.

*/

#include <cassert>

template<typename Fn>
double integrate(Fn f, double a, double b, int n = 1000000) {
  assert(n > 0 && n % 2 == 0);
  double h = (b - a) / n, s = f(a) + f(b);
  for (int i = 1; i < n; i++) {
    s += f(a + h * i) * (i & 1 ? 4 : 2);
  }
  return s * h / 3;
}

/*** Example Usage ***/

#include <cmath>
#include <cstdio>
using namespace std;

double f(double x) {
  return sin(x);
}

int main() {
  const double PI = acos(-1.0);
  assert(fabs(integrate(f, 0.0, PI / 2) - 1) < 1e-10);
  return 0;
}
