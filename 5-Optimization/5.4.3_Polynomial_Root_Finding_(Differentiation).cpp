/*

Finds every real root $x$ of a polynomial $p$ satisfying $p(x) = 0$ by recursively finding the roots
of its derivative. Each adjacent pair of local extrema is searched using the bisection method.

The key observation is that derivative roots split the real line into intervals where the polynomial
is monotone. A monotone interval contains at most one root, and if an endpoint is a root or the
endpoint values have opposite signs, bisection isolates that root. Recursively solving the
derivative therefore provides all interval boundaries needed to find every real root in the
requested range, including roots of even multiplicity.

- `horner_eval(p, x)` evaluates the polynomial `p` of degree $d$ (represented as a vector of size
  $d + 1$ where `p[i]` stores the coefficient for the $x^i$ term) at `x`, using Horner's method.
- `find_one_root(p, a, b, eps = 1e-15)` returns a root in the interval $[`a`, `b`]$ for a polynomial
  `p` where either endpoint is a root or the endpoint values have opposite signs, using the
  bisection method. If this precondition is not satisfied, then `NaN` is returned. The root is found
  to a tolerance of `eps` in absolute or relative error (whichever is reached first).
- `find_all_roots(p, a = -1e20, b = 1e20, eps = 1e-15)` returns a vector of all roots in the
  interval $[`a`, `b`]$ for a polynomial `p` using the bisection method. The roots are found to a
  tolerance of `eps` in absolute or relative error (whichever is reached first).

Time Complexity:
- O(n) per call to `horner_eval()`, where $n$ is the degree of the polynomial.
- O(nt) per call to `find_one_root()`, where $n$ is the degree of the polynomial and $t$ is the
  number of bisection iterations required to reach the requested tolerance.
- O(n^3 t) per call to `find_all_roots()`, where $n$ is the degree of the polynomial and $t$ is the
  number of bisection iterations required to reach the requested tolerance.

Space Complexity:
- O(1) auxiliary for `horner_eval()` and `find_one_root()`.
- O(n) auxiliary stack space and O(n^2) auxiliary heap space for `find_all_roots()`, where $n$ is
  the degree of the polynomial.

*/

#include <cmath>
#include <limits>
#include <vector>

double horner_eval(const std::vector<double> &p, double x) {
  double res = p.back();
  for (int i = static_cast<int>(p.size()) - 2; i >= 0; i--) {
    res = res * x + p[i];
  }
  return res;
}

double find_one_root(const std::vector<double> &p, double a, double b, const double eps = 1e-15) {
  double pa = horner_eval(p, a), pb = horner_eval(p, b);
  if (fabs(pa) <= eps) {
    return a;
  }
  if (fabs(pb) <= eps) {
    return b;
  }
  bool paneg = pa < 0, pbneg = pb < 0;
  if (paneg == pbneg) {
    return std::numeric_limits<double>::quiet_NaN();
  }
  while (b - a > eps && a * (1 + eps) < b && a < b * (1 + eps)) {
    double m = a + (b - a) / 2;
    if ((horner_eval(p, m) < 0) == paneg) {
      a = m;
    } else {
      b = m;
    }
  }
  return a;
}

std::vector<double> find_all_roots(
    const std::vector<double> &p, double a = -1e20, double b = 1e20, const double eps = 1e-15
) {
  std::vector<double> pprime;
  pprime.reserve(p.size() > 0 ? p.size() - 1 : 0);
  for (int i = 1; i < static_cast<int>(p.size()); i++) {
    pprime.push_back(p[i] * i);
  }
  if (pprime.empty()) {
    return {};
  }
  std::vector<double> res, r = find_all_roots(pprime, a, b, eps);
  r.push_back(b);
  for (int i = 0; i < static_cast<int>(r.size()); i++) {
    double root = find_one_root(p, i == 0 ? a : r[i - 1], r[i], eps);
    if (!std::isnan(root) && (res.empty() || root != res.back())) {
      res.push_back(root);
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  {  // -1 + 2x - 6x^2 + 2x^3
    vector<double> p{-1.0, 2.0, -6.0, 2.0};
    vector<double> roots = find_all_roots(p);
    assert(roots.size() == 1 && fabs(horner_eval(p, roots[0])) < 1e-10);
  }
  {  // -20 + 4x + 3x^2
    vector<double> p{-20.0, 4.0, 3.0};
    vector<double> roots = find_all_roots(p);
    assert(roots.size() == 2);
    assert(fabs(horner_eval(p, roots[0])) < 1e-10);
    assert(fabs(horner_eval(p, roots[1])) < 1e-10);
  }
  {  // (x - 1)^2
    vector<double> p{1.0, -2.0, 1.0};
    vector<double> roots = find_all_roots(p);
    assert(roots.size() == 1 && fabs(roots[0] - 1) < 1e-10);
  }
  return 0;
}
