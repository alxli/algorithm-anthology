/*

Finds every complex root $x$ for a polynomial $p$ with complex coefficients such that $p(x) = 0$.
Laguerre's method is a root-finding iteration with reliable, near-cubic convergence from almost any
starting point. Once a root is found it is removed by polynomial deflation, and the iteration
repeats on the lower-degree quotient until every root has been extracted.

- `horner_eval(p, x)` evaluates a complex polynomial $p$ of degree $d$ (represented as a vector of
  size $d + 1$ where `p[i]` stores the degree-`i` coefficient) at `x`, using Horner's method,
  returning a pair where the first value is the final result $p(x)$ and the second value is the
  quotient polynomial $q$ from the identity $p(t) = (t - x)q(t) + p(x)$. The coefficient vector must
  be nonempty.
- `find_one_root(p, x0, eps = 1e-15, iterations = 10000)` returns a complex root $x$ for polynomial
  `p` (represented as a vector of size $d + 1$ where `p[i]` stores the degree-`i` coefficient) using
  an initial guess `x0` which should be relatively close to $x$. The root is found to a tolerance of
  `eps` in absolute or relative error (whichever is reached first). The polynomial must have degree
  at least one and omit trailing zero coefficients; its coefficients are scaled by their largest
  magnitude so a common factor does not affect convergence.
- `find_all_roots(p, eps = 1e-15, iterations = 10000)` returns a vector of all complex roots for a
  complex polynomial `p`. The roots are found to a tolerance of `eps` in absolute or relative error
  (whichever is reached first). The coefficient vector must be nonempty and omit trailing zero
  coefficients.

Time Complexity:
- O(n) per call to `horner_eval()`, where $n$ is the degree of the polynomial.
- O(nt) per call to `find_one_root()`, where $n$ is the degree of the polynomial and $t$ is the
  number of iterations performed.
- O(n^2 t) per call to `find_all_roots()`, where $n$ is the degree of the polynomial and $t$ is the
  number of iterations performed per root.

Space Complexity:
- O(n) auxiliary for `horner_eval()` and `find_one_root()`, where $n$ is the degree of the
  polynomial.
- O(n) auxiliary for `find_all_roots()`, where $n$ is the degree of the polynomial.

*/

#include <algorithm>
#include <chrono>
#include <cmath>
#include <complex>
#include <random>
#include <utility>
#include <vector>

using dbl = long double;
using cdbl = std::complex<dbl>;
using cpoly = std::vector<cdbl>;

std::pair<cdbl, cpoly> horner_eval(const cpoly &p, const cdbl &x) {
  int n = static_cast<int>(p.size());
  cpoly b(std::max(1, n - 1));
  for (int i = n - 1; i > 0; i--) {
    b[i - 1] = p[i] + (i < n - 1 ? b[i] * x : 0);
  }
  return {p[0] + b[0] * x, b};
}

cpoly derivative(const cpoly &p) {
  int n = static_cast<int>(p.size());
  cpoly res(std::max(1, n - 1));
  for (int i = 1; i < n; i++) {
    res[i - 1] = p[i] * cdbl(i);
  }
  return res;
}

cdbl find_one_root(cpoly p, const cdbl &x0, dbl eps = 1e-15L, int iterations = 10000) {
  dbl scale = 0;
  for (const cdbl &c : p) {
    scale = std::max(scale, std::abs(c));
  }
  for (cdbl &c : p) {
    c /= scale;
  }
  cdbl x = x0;
  int n = static_cast<int>(p.size()) - 1;
  cpoly p1 = derivative(p), p2 = derivative(p1);
  for (int i = 0; i < iterations; i++) {
    cdbl y0 = horner_eval(p, x).first;
    if (std::abs(y0) <= eps) {
      break;
    }
    cdbl g = horner_eval(p1, x).first / y0;
    cdbl h = g * g - horner_eval(p2, x).first / y0;
    cdbl r = std::sqrt(cdbl(n - 1) * (h * cdbl(n) - g * g));
    cdbl d1 = g + r, d2 = g - r;
    cdbl a = cdbl(n) / (std::abs(d1) > std::abs(d2) ? d1 : d2);
    x -= a;
    if (std::abs(a) <= eps) {
      break;
    }
  }
  return x;
}

std::vector<cdbl> find_all_roots(const cpoly &p, dbl eps = 1e-15L, int iterations = 10000) {
  std::vector<cdbl> res;
  cpoly q = p;
  if (q.size() <= 1) {
    return res;
  }
  static std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::uniform_real_distribution<dbl> unit(0.0L, 1.0L);
  while (q.size() > 2) {
    cdbl z(unit(rng), unit(rng));
    z = find_one_root(p, find_one_root(q, z, eps, iterations), eps, iterations);
    q = horner_eval(q, z).second;
    res.push_back(z);
  }
  res.push_back(-q[0] / q[1]);
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

void assert_roots(const vector<cdbl> &actual, const vector<cdbl> &expected) {
  assert(actual.size() == expected.size());
  vector<char> matched(actual.size());
  for (const cdbl &root : expected) {
    int found = -1;
    for (int i = 0; i < static_cast<int>(actual.size()); i++) {
      if (!matched[i] && abs(actual[i] - root) < 1e-8L) {
        found = i;
        break;
      }
    }
    assert(found != -1);
    matched[found] = true;
  }
}

int main() {
  {  // 140 - 13x - 8x^2 + x^3 = (x + 4)(x - 5)(x - 7)
    cpoly p;
    p.push_back(140);
    p.push_back(-13);
    p.push_back(-8);
    p.push_back(1);
    assert_roots(find_all_roots(p), {-4, 5, 7});
    for (cdbl &c : p) {
      c *= 1e-30L;
    }
    assert_roots(find_all_roots(p), {-4, 5, 7});
  }
  {  // (-24+36i) + (-26+12i)x + (-30+40i)x^2 + (-26+12i)x^3 + (-6+4i)x^4
    // = ((2 + 3i)x + 6)(x + i)(2x + (6 + 4i))(xi + 1):
    cpoly p;
    p.emplace_back(-24, 36);
    p.emplace_back(-26, 12);
    p.emplace_back(-30, 40);
    p.emplace_back(-26, 12);
    p.emplace_back(-6, 4);
    assert_roots(find_all_roots(p), {{-3, -2}, {-12.0L / 13, 18.0L / 13}, {0, -1}, {0, 1}});
  }
  return 0;
}
