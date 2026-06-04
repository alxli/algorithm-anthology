/*

Finds every complex root $x$ for a polynomial $p$ such that $p(x) = 0$ using the Ehrlich-Aberth
method. This is a simultaneous iteration: every root estimate is updated using both the Newton
correction and a repulsion term from the other estimates.

This routine is intended for well-scaled polynomials when all complex roots are wanted. If only real
roots are needed, a real-root isolator such as derivative recursion with bisection is usually more
reliable. Multiple or tightly clustered roots may converge slowly, and very large or very small root
scales may require rescaling the input or using multiprecision arithmetic.

- `eval_with_derivative(p, x)` returns `p(x)` and `p'(x)` for a polynomial `p` represented as a
  vector where `p[i]` stores the coefficient for the $x^i$ term.
- `find_all_roots(p, EPS, ITERATIONS)` returns a vector of all complex roots for a complex
  polynomial `p`. A `vector<LD>` overload is provided for polynomials with real coefficients. The
  roots are found to a tolerance of `EPS` in absolute or relative error (whichever is reached
  first), and zero roots are removed exactly before the simultaneous iteration starts.

Time Complexity:
- O(n) per call to `eval_with_derivative(p, x)`, where $n$ is the degree of the polynomial.
- O(n^2 p) per call to `find_all_roots(p, EPS, ITERATIONS)`, where $n$ is the degree of the
  polynomial and $p$ is the number of iterations required to reach the desired precision.

Space Complexity:
- O(1) auxiliary space for `eval_with_derivative(p, x)`.
- O(n) auxiliary heap space for `find_all_roots(p, EPS, ITERATIONS)`, where $n$ is the degree of the
  polynomial.

*/

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

using LD = long double;
using cdouble = std::complex<LD>;
using cpoly = std::vector<cdouble>;

const LD PI = acosl(-1.0L);
const LD ZERO_EPS = 1e-30L;   // Treat coefficients and denominators this small as zero.
const LD ROOT_EPS = 1e-18L;   // Stop once root updates are this small relative to the root.
const LD CLEAN_EPS = 1e-12L;  // Round tiny real or imaginary output parts down to zero.
const LD CHECK_EPS = 1e-12L;  // Residual tolerance used by the example assertions.

bool is_zero(const cdouble &z, const LD EPS = ZERO_EPS) {
  return std::abs(z) <= EPS;
}

bool is_finite(const cdouble &z) {
  return std::isfinite(static_cast<double>(z.real())) &&
         std::isfinite(static_cast<double>(z.imag()));
}

std::pair<cdouble, cdouble> eval_with_derivative(const cpoly &p, const cdouble &x) {
  cdouble value = p.back(), derivative = 0;
  for (int i = static_cast<int>(p.size()) - 2; i >= 0; i--) {
    derivative = derivative * x + value;
    value = value * x + p[i];
  }
  return {value, derivative};
}

LD root_bound(const cpoly &p) {
  LD res = 0;
  int n = p.size() - 1;
  for (int i = 0; i + 1 < static_cast<int>(p.size()); i++) {
    LD ratio = std::abs(p[i] / p.back());
    if (ratio > 0) {
      res = std::max(res, powl(ratio, 1.0L / (n - i)));
    }
  }
  return 2 * std::max((LD)1, res);
}

bool root_less(const cdouble &a, const cdouble &b) {
  if (a.real() != b.real()) {
    return a.real() < b.real();
  }
  return a.imag() < b.imag();
}

cdouble clean_root(const cdouble &z) {
  LD real = fabsl(z.real()) < CLEAN_EPS ? 0 : z.real();
  LD imag = fabsl(z.imag()) < CLEAN_EPS ? 0 : z.imag();
  return cdouble(real, imag);
}

cpoly find_all_roots(cpoly p, const LD EPS = ROOT_EPS, const int ITERATIONS = 2000) {
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
  LD scale = 0;
  for (int i = 0; i < static_cast<int>(p.size()); i++) {
    scale = std::max(scale, std::abs(p[i]));
  }
  for (int i = 0; i < static_cast<int>(p.size()); i++) {
    p[i] /= scale;
  }
  int n = p.size() - 1;
  if (n == 1) {
    roots.push_back(-p[0] / p[1]);
    return roots;
  }
  cpoly z(n);
  LD radius = root_bound(p), offset = PI / (2 * n);
  LD max_radius = 2 * radius;
  for (int i = 0; i < n; i++) {
    LD angle = offset + 2 * PI * i / n;
    z[i] = radius * cdouble(cosl(angle), sinl(angle));
  }
  for (int it = 0; it < ITERATIONS; it++) {
    bool done = true;
    cpoly next = z;
    for (int i = 0; i < n; i++) {
      auto [fx, dfx] = eval_with_derivative(p, z[i]);
      if (std::abs(fx) <= EPS) {
        continue;
      }
      cdouble repulsion = 0;
      for (int j = 0; j < n; j++) {
        if (i != j) {
          cdouble diff = z[i] - z[j];
          if (std::abs(diff) <= EPS * EPS) {
            LD angle = 2 * PI * (i + 1) / (n + 1);
            diff = EPS * (1 + std::abs(z[i])) * cdouble(cosl(angle), sinl(angle));
          }
          repulsion += cdouble(1) / diff;
        }
      }
      cdouble denom = dfx - fx * repulsion;
      if (is_zero(denom)) {
        continue;
      }
      cdouble step = fx / denom;
      if (!is_finite(step) && !is_zero(dfx)) {
        step = fx / dfx;
      }
      if (!is_finite(step)) {
        continue;
      }
      LD limit = 2 * max_radius;
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
      if (std::abs(step) > EPS * (1 + std::abs(next[i]))) {
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

std::vector<cdouble> find_all_roots(
    const std::vector<LD> &p, const LD EPS = ROOT_EPS, const int ITERATIONS = 2000
) {
  cpoly q(p.begin(), p.end());
  return find_all_roots(q, EPS, ITERATIONS);
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

cdouble eval(const cpoly &p, const cdouble &x) {
  return eval_with_derivative(p, x).first;
}

void print_roots(const vector<cdouble> &x) {
  for (const auto &z : x) {
    printf("(%.5Lf, %.5Lf)\n", z.real(), z.imag());
  }
}

int main() {
  {  // -1 + 2x - 6x^2 + 2x^3
    printf("Roots of -1 + 2x - 6x^2 + 2x^3:\n");
    vector<LD> p{-1.0, 2.0, -6.0, 2.0};
    vector<cdouble> roots = find_all_roots(p);
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
    vector<cdouble> roots = find_all_roots(p);
    assert(roots.size() == 4);
    for (const auto &root : roots) {
      assert(abs(eval(p, root)) < CHECK_EPS);
    }
    print_roots(roots);
  }
  return 0;
}
