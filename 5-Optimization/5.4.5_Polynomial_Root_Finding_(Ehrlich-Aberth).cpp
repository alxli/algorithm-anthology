/*

Finds every complex root $x$ for a polynomial $p$ such that $p(x) = 0$. The Ehrlich-Aberth method is
a simultaneous iteration: every root estimate is updated using both the Newton correction and a
repulsion term from the other estimates.

This routine is intended for well-scaled polynomials when all complex roots are wanted. If only real
roots are needed, a real-root isolator such as derivative recursion with bisection is usually more
reliable. Multiple or tightly clustered roots may converge slowly, and very large or very small root
scales may require rescaling the input or using multiprecision arithmetic.

- `eval_with_derivative(p, x)` returns a pair $(p(x), p'(x))$ for a polynomial $p$ given as a vector
  `p` where `p[i]` stores the coefficient for the $x^i$ term.
- `find_all_roots(p, eps = ROOT_EPS, iterations = 2000)` returns a vector of all complex roots for a
  complex polynomial given by the vector of coefficients `p`. A `vector<dbl>` overload is provided
  for polynomials with real coefficients. The roots are found to a tolerance of `eps` in absolute or
  relative error (whichever is reached first), and zero roots are removed exactly before the
  simultaneous iteration starts.

Root-finding inputs must contain at least one nonzero coefficient; trailing near-zero coefficients
are discarded. `eval_with_derivative()` requires a nonempty coefficient vector.

Time Complexity:
- O(n) per call to `eval_with_derivative()`, where $n$ is the degree of the polynomial.
- O(n^2 t) per call to `find_all_roots()`, where $n$ is the degree of the polynomial and $t$ is the
  number of iterations required to reach the desired precision.

Space Complexity:
- O(1) auxiliary for `eval_with_derivative()`.
- O(n) auxiliary for `find_all_roots()`, where $n$ is the degree of the polynomial.

*/

#include <algorithm>
#include <cmath>
#include <complex>
#include <utility>
#include <vector>

using dbl = long double;
using cdbl = std::complex<dbl>;
using cpoly = std::vector<cdbl>;

const dbl PI = acosl(-1.0L);
const dbl ZERO_EPS = 1e-30L;   // Treat coefficients and denominators this small as zero.
const dbl ROOT_EPS = 1e-18L;   // Stop once root updates are this small relative to the root.
const dbl CHECK_EPS = 1e-12L;  // Residual tolerance used by the example assertions.

bool is_zero(const cdbl &z) { return std::abs(z) <= ZERO_EPS; }
bool is_finite(const cdbl &z) { return std::isfinite(z.real()) && std::isfinite(z.imag()); }

std::pair<cdbl, cdbl> eval_with_derivative(const cpoly &p, const cdbl &x) {
  cdbl value = p.back(), derivative = 0;
  for (int i = static_cast<int>(p.size()) - 2; i >= 0; i--) {
    derivative = derivative * x + value;
    value = value * x + p[i];
  }
  return {value, derivative};
}

dbl root_bound(const cpoly &p) {
  dbl res = 0;
  int n = static_cast<int>(p.size()) - 1;
  for (int i = 0; i < n; i++) {
    dbl ratio = std::abs(p[i] / p.back());
    if (ratio > 0) {
      res = std::max(res, powl(ratio, 1.0L / (n - i)));
    }
  }
  return 2 * std::max(static_cast<dbl>(1), res);
}

cpoly find_all_roots(cpoly p, const dbl eps = ROOT_EPS, const int iterations = 2000) {
  while (!p.empty() && is_zero(p.back())) {
    p.pop_back();
  }
  cpoly roots;
  while (p.size() > 1 && is_zero(p[0])) {
    roots.push_back(0);
    p.erase(p.begin());
  }
  if (p.size() <= 1) {
    return roots;
  }
  dbl scale = 0;
  for (int i = 0; i < static_cast<int>(p.size()); i++) {
    scale = std::max(scale, std::abs(p[i]));
  }
  for (int i = 0; i < static_cast<int>(p.size()); i++) {
    p[i] /= scale;
  }
  int n = static_cast<int>(p.size()) - 1;
  if (n == 1) {
    roots.push_back(-p[0] / p[1]);
    return roots;
  }
  cpoly z(n);
  dbl radius = root_bound(p), offset = PI / (2 * n);
  dbl max_radius = 2 * radius;
  for (int i = 0; i < n; i++) {
    dbl angle = offset + 2 * PI * i / n;
    z[i] = radius * cdbl(cosl(angle), sinl(angle));
  }
  for (int it = 0; it < iterations; it++) {
    bool done = true;
    cpoly next = z;
    for (int i = 0; i < n; i++) {
      auto [fx, dfx] = eval_with_derivative(p, z[i]);
      if (std::abs(fx) <= eps) {
        continue;
      }
      cdbl repulsion = 0;
      for (int j = 0; j < n; j++) {
        if (i != j) {
          cdbl diff = z[i] - z[j];
          if (std::abs(diff) <= eps * eps) {
            dbl angle = 2 * PI * (i + 1) / (n + 1);
            diff = eps * (1 + std::abs(z[i])) * cdbl(cosl(angle), sinl(angle));
          }
          repulsion += cdbl(1) / diff;
        }
      }
      cdbl denom = dfx - fx * repulsion;
      if (is_zero(denom)) {
        done = false;
        continue;
      }
      cdbl step = fx / denom;
      if (!is_finite(step) && !is_zero(dfx)) {
        step = fx / dfx;
      }
      if (!is_finite(step)) {
        done = false;
        continue;
      }
      dbl limit = 2 * max_radius;
      if (std::abs(step) > limit) {
        step *= limit / std::abs(step);
      }
      next[i] = z[i] - step;
      if (!is_finite(next[i])) {
        next[i] = z[i];
        done = false;
        continue;
      }
      if (std::abs(next[i]) > max_radius) {
        next[i] *= max_radius / std::abs(next[i]);
      }
      if (std::abs(step) > eps * (1 + std::abs(next[i]))) {
        done = false;
      }
    }
    z = next;
    if (done) {
      break;
    }
  }
  for (int i = 0; i < n; i++) {
    roots.emplace_back(z[i]);
  }
  std::sort(roots.begin(), roots.end(), [](const cdbl &a, const cdbl &b) {
    return a.real() != b.real() ? a.real() < b.real() : a.imag() < b.imag();
  });
  return roots;
}

std::vector<cdbl> find_all_roots(
    const std::vector<dbl> &p, const dbl eps = ROOT_EPS, const int iterations = 2000
) {
  cpoly q(p.begin(), p.end());
  return find_all_roots(q, eps, iterations);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

cdbl eval(const cpoly &p, const cdbl &x) {
  return eval_with_derivative(p, x).first;
}

void assert_roots(const vector<cdbl> &actual, const vector<cdbl> &expected) {
  assert(actual.size() == expected.size());
  vector<char> matched(actual.size());
  for (const cdbl &root : expected) {
    int found = -1;
    for (int i = 0; i < static_cast<int>(actual.size()); i++) {
      if (!matched[i] && abs(actual[i] - root) < CHECK_EPS) {
        found = i;
        break;
      }
    }
    assert(found != -1);
    matched[found] = true;
  }
}

int main() {
  {  // -1 + 2x - 6x^2 + 2x^3
    vector<dbl> p{-1.0, 2.0, -6.0, 2.0};
    vector<cdbl> roots = find_all_roots(p);
    assert(roots.size() == 3);
    for (const auto &root : roots) {
      assert(abs(eval(cpoly(p.begin(), p.end()), root)) < CHECK_EPS);
    }
  }
  {  // (-24+36i) + (-26+12i)x + (-30+40i)x^2 + (-26+12i)x^3 + (-6+4i)x^4
    // = ((2 + 3i)x + 6)(x + i)(2x + (6 + 4i))(xi + 1):
    cpoly p;
    p.emplace_back(-24, 36);
    p.emplace_back(-26, 12);
    p.emplace_back(-30, 40);
    p.emplace_back(-26, 12);
    p.emplace_back(-6, 4);
    vector<cdbl> roots = find_all_roots(p);
    assert(roots.size() == 4);
    for (const auto &root : roots) {
      assert(abs(eval(p, root)) < CHECK_EPS);
    }
    assert_roots(roots, {{-3, -2}, {-12.0L / 13, 18.0L / 13}, {0, -1}, {0, 1}});
  }
  return 0;
}
