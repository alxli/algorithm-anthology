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

Time Complexity:
- O(n) per call to `eval_with_derivative(p, x)`, where $n$ is the degree of the polynomial.
- O(n^2 t) per call to `find_all_roots(p, eps, iterations)`, where $n$ is the degree of the
  polynomial and $t$ is the number of iterations required to reach the desired precision.

Space Complexity:
- O(1) auxiliary space for `eval_with_derivative(p, x)`.
- O(n) auxiliary heap space for `find_all_roots(p, eps, iterations)`, where $n$ is the degree of the
  polynomial.

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

bool is_zero(const cdbl &z, const dbl eps = ZERO_EPS) {
  return std::abs(z) <= eps;
}

bool is_finite(const cdbl &z) {
  return std::isfinite(static_cast<double>(z.real())) &&
         std::isfinite(static_cast<double>(z.imag()));
}

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
  return 2 * std::max((dbl)1, res);
}

bool root_less(const cdbl &a, const cdbl &b) {
  if (a.real() != b.real()) {
    return a.real() < b.real();
  }
  return a.imag() < b.imag();
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
        continue;
      }
      cdbl step = fx / denom;
      if (!is_finite(step) && !is_zero(dfx)) {
        step = fx / dfx;
      }
      if (!is_finite(step)) {
        continue;
      }
      dbl limit = 2 * max_radius;
      if (std::abs(step) > limit) {
        step *= limit / std::abs(step);
      }
      next[i] = z[i] - step;
      if (!is_finite(next[i])) {
        next[i] = z[i];
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
  std::sort(roots.begin(), roots.end(), root_less);
  return roots;
}

std::vector<cdbl> find_all_roots(
    const std::vector<dbl> &p, const dbl eps = ROOT_EPS, const int iterations = 2000
) {
  cpoly q(p.begin(), p.end());
  return find_all_roots(q, eps, iterations);
}

/*** Example Usage and Output:

Roots of -1 + 2x - 6x^2 + 2x^3:
(0.15098, -0.40314)
(0.15098, 0.40314)
(2.69805, 0.00000)
Roots of ((2 + 3i)x + 6)(x + i)(2x + (6 + 4i))(xi + 1):
(-3.00000, -2.00000)
(-0.92308, 1.38462)
(0.00000, -1.00000)
(0.00000, 1.00000)

***/

#include <cassert>
#include <cstdio>
using namespace std;

cdbl eval(const cpoly &p, const cdbl &x) {
  return eval_with_derivative(p, x).first;
}

void print_roots(vector<cdbl> x) {
  sort(x.begin(), x.end(), [](const cdbl &a, const cdbl &b) {
    return abs(a.real() - b.real()) > 0.5e-5 ? a.real() < b.real() : a.imag() < b.imag();
  });
  for (const auto &z : x) {
    dbl real = abs(z.real()) < 0.5e-5 ? 0 : z.real();
    dbl imag = abs(z.imag()) < 0.5e-5 ? 0 : z.imag();
    printf("(%.5Lf, %.5Lf)\n", real, imag);
  }
}

int main() {
  {  // -1 + 2x - 6x^2 + 2x^3
    printf("Roots of -1 + 2x - 6x^2 + 2x^3:\n");
    vector<dbl> p{-1.0, 2.0, -6.0, 2.0};
    vector<cdbl> roots = find_all_roots(p);
    assert(roots.size() == 3);
    for (const auto &root : roots) {
      assert(abs(eval(cpoly(p.begin(), p.end()), root)) < CHECK_EPS);
    }
    print_roots(roots);
  }
  {  // (-24+36i) + (-26+12i)x + (-30+40i)x^2 + (-26+12i)x^3 + (-6+4i)x^4
    // = ((2 + 3i)x + 6)(x + i)(2x + (6 + 4i))(xi + 1):
    printf("Roots of ((2 + 3i)x + 6)(x + i)(2x + (6 + 4i))(xi + 1):\n");
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
    print_roots(roots);
  }
  return 0;
}
