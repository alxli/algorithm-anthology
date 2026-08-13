/*

Finds every real root $x$ of a polynomial $p$ satisfying $p(x) = 0$ by recursively finding the roots
of its derivative. Each adjacent pair of local extrema is searched using the bisection method.

The key observation is that derivative roots split the real line into intervals where the polynomial
is monotone. A monotone interval contains at most one root, and if an endpoint is a root or the
endpoint values have opposite signs, bisection isolates that root. Recursively solving the
derivative therefore provides all interval boundaries needed to find every real root in the
requested range, including roots of even multiplicity. Each recursive call first scales the
coefficients by their largest magnitude so that root detection is unaffected by a common factor.

- `horner_eval(p, x)` evaluates the polynomial `p` of degree $d$ (represented as a vector of size
  $d + 1$ where `p[i]` stores the degree-`i` coefficient) at `x`, using Horner's method.
- `find_one_root(p, a, b, eps = 1e-15)` returns a root in the interval $[`a`, `b`]$ for a polynomial
  `p` where either endpoint is a root or the endpoint values have opposite signs, using the
  bisection method. If this precondition is not satisfied, then `NaN` is returned. The root is found
  to a tolerance of `eps` in absolute or relative error (whichever is reached first).
- `find_all_roots(p, a = -1e20, b = 1e20, eps = 1e-15)` returns a vector of all roots in the
  interval $[`a`, `b`]$ for a polynomial `p` using the bisection method. The roots are found to a
  tolerance of `eps` in absolute or relative error (whichever is reached first).

The coefficient vector must be nonempty and contain at least one nonzero coefficient.

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

#include <algorithm>
#include <cmath>
#include <limits>
#include <vector>

using dbl = long double;

dbl horner_eval(const std::vector<dbl> &p, dbl x) {
  dbl res = p.back();
  for (int i = static_cast<int>(p.size()) - 2; i >= 0; i--) {
    res = res * x + p[i];
  }
  return res;
}

dbl find_one_root(const std::vector<dbl> &p, dbl a, dbl b, dbl eps = 1e-15L) {
  dbl scale = 0;
  for (dbl c : p) {
    scale = std::max(scale, std::fabs(c));
  }
  dbl pa = horner_eval(p, a), pb = horner_eval(p, b);
  if (std::fabs(pa) <= eps * scale) {
    return a;
  }
  if (std::fabs(pb) <= eps * scale) {
    return b;
  }
  bool paneg = pa < 0, pbneg = pb < 0;
  if (paneg == pbneg) {
    return std::numeric_limits<dbl>::quiet_NaN();
  }
  while (b - a > eps && a * (1 + eps) < b && a < b * (1 + eps)) {
    dbl m = a + (b - a) / 2;
    if ((horner_eval(p, m) < 0) == paneg) {
      a = m;
    } else {
      b = m;
    }
  }
  return a;
}

std::vector<dbl> find_all_roots(
    std::vector<dbl> p, dbl a = -1e20L, dbl b = 1e20L, dbl eps = 1e-15L
) {
  while (p.size() > 1 && p.back() == 0) {
    p.pop_back();
  }
  if (p.size() <= 1) {
    return {};
  }
  dbl scale = 0;
  for (dbl c : p) {
    scale = std::max(scale, std::fabs(c));
  }
  for (dbl &c : p) {
    c /= scale;
  }
  std::vector<dbl> pprime;
  pprime.reserve(p.size() > 0 ? p.size() - 1 : 0);
  for (int i = 1; i < static_cast<int>(p.size()); i++) {
    pprime.push_back(p[i] * i);
  }
  std::vector<dbl> res, r = find_all_roots(pprime, a, b, eps);
  r.push_back(b);
  for (int i = 0; i < static_cast<int>(r.size()); i++) {
    dbl root = find_one_root(p, i == 0 ? a : r[i - 1], r[i], eps);
    dbl dedup_eps = eps * std::max(dbl{1}, std::fabs(root));
    if (!std::isnan(root) && (res.empty() || std::fabs(root - res.back()) > dedup_eps)) {
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
    vector<dbl> p{-1, 2, -6, 2};
    vector<dbl> roots = find_all_roots(p);
    assert(roots.size() == 1 && fabs(horner_eval(p, roots[0])) < 1e-10L);
  }
  {  // -20 + 4x + 3x^2
    vector<dbl> p{-20, 4, 3};
    vector<dbl> roots = find_all_roots(p);
    assert(roots.size() == 2);
    assert(fabs(horner_eval(p, roots[0])) < 1e-10L);
    assert(fabs(horner_eval(p, roots[1])) < 1e-10L);
  }
  {  // (x - 1)^2
    vector<dbl> p{1, -2, 1};
    vector<dbl> roots = find_all_roots(p);
    assert(roots.size() == 1 && fabs(roots[0] - 1) < 1e-10L);
  }
  {  // Scaling the coefficients does not duplicate or hide the repeated root (x - sqrt(2))^2.
    dbl r = sqrt(2.0L), scale = 1e24L;
    vector<dbl> p{2 * scale, -2 * r * scale, scale};
    vector<dbl> roots = find_all_roots(p, -10, 10);
    assert(roots.size() == 1 && fabs(roots[0] - r) < 1e-10L);
    assert(fabs(find_one_root(p, r, 10) - r) < 1e-10L);
  }
  {
    vector<dbl> roots = find_all_roots(vector<dbl>{-1, 1, 0}, -10, 10);
    assert(roots.size() == 1 && fabs(roots[0] - 1) < 1e-10L);
  }
  return 0;
}
