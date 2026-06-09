/*

Finds every complex root $x$ for a polynomial $p$ with complex coefficients such that $p(x) = 0$
using Laguerre's method.

- `horner_eval(p, x)` evaluates the complex polynomial `p` of degree $d$ (represented as a vector of
  size $d + 1$ where `p[i]` stores the complex coefficient for the $x^i$ term) at `x`, using
  Horner's method, returning a pair where the first value is a vector of sub-evaluations and the
  second value is the final result $p(x)$.
- `find_one_root(p, x0)` returns a complex root $x$ for a polynomial `p` (represented as a vector of
  size $d + 1$ where `p[i]` stores the complex coefficient for the $x^i$ term) using an initial
  guess `x0` which should be relatively close to $x$. The root is found to a tolerance of `EPS` in
  absolute or relative error (whichever is reached first).
- `find_all_roots(p)` returns a vector of all complex roots for a complex polynomial `p`. The roots
  are found to a tolerance of `EPS` in absolute or relative error (whichever is reached first).

Time Complexity:
- O(n) per call to `horner_eval()`, where $n$ is the degree of the polynomial.
- O(n log p) per call to `find_one_root()`, where $n$ is the degree of the polynomial and $p =
  -\log_{10}(\text{EPS})$ is the number of digits of absolute or relative precision that is desired.
- O(n^2 log p) per call to `find_all_roots()`, where $n$ is the degree of the polynomial and $p =
  -\log_{10}(\text{EPS})$ is the number of digits of absolute or relative precision that is desired.

Space Complexity:
- O(n) auxiliary heap space and O(1) auxiliary stack space for `horner_eval()` and
  `find_one_root()`, where $n$ is the degree of the polynomial.
- O(n) auxiliary heap and O(1) auxiliary stack space per for `find_one_root()` and
  `find_all_roots()`, where $n$ is the degree of the polynomial.

*/

#include <complex>
#include <random>
#include <vector>

using cdouble = std::complex<double>;
using cpoly = std::vector<cdouble>;

std::pair<cdouble, cpoly> horner_eval(const cpoly &p, const cdouble &x) {
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
    res[i - 1] = p[i] * cdouble(i);
  }
  return res;
}

int comp(const cdouble &a, const cdouble &b, const double EPS = 1e-15) {
  double diff = std::abs(a) - std::abs(b);
  return (diff < -EPS) ? -1 : (diff > EPS ? 1 : 0);
}

cdouble find_one_root(
    const cpoly &p, const cdouble &x0, const double EPS = 1e-15, const int ITERATIONS = 10000
) {
  cdouble x = x0;
  int n = static_cast<int>(p.size()) - 1;
  cpoly p1 = derivative(p), p2 = derivative(p1);
  for (int i = 0; i < ITERATIONS; i++) {
    cdouble y0 = horner_eval(p, x).first;
    if (comp(y0, 0, EPS) == 0) {
      break;
    }
    cdouble g = horner_eval(p1, x).first / y0;
    cdouble h = g * g - horner_eval(p2, x).first / y0;
    cdouble r = std::sqrt(cdouble(n - 1) * (h * cdouble(n) - g * g));
    cdouble d1 = g + r, d2 = g - r;
    cdouble a = cdouble(n) / (comp(d1, d2, EPS) > 0 ? d1 : d2);
    x -= a;
    if (comp(a, 0, EPS) == 0) {
      break;
    }
  }
  return x;
}

std::vector<cdouble> find_all_roots(
    const cpoly &p, const double EPS = 1e-15, const int ITERATIONS = 10000
) {
  std::vector<cdouble> res;
  cpoly q = p;
  static std::mt19937 rng(std::random_device{}());
  std::uniform_real_distribution<double> unit(0.0, 1.0);
  while (q.size() > 2) {
    cdouble z(unit(rng), unit(rng));
    z = find_one_root(p, find_one_root(q, z, EPS, ITERATIONS), EPS, ITERATIONS);
    q = horner_eval(q, z).second;
    res.push_back(z);
  }
  res.push_back(-q[0] / q[1]);
  return res;
}

/*** Example Usage and Output:

Roots of 140 - 13x - 8x^2 + x^3:
(-4.00000, 0.00000)
(5.00000, 0.00000)
(7.00000, 0.00000)
Roots of ((2 + 3i)x + 6)(x + i)(2x + (6 + 4i))(xi + 1):
(-3.00000, -2.00000)
(-0.92308, 1.38462)
(0.00000, -1.00000)
(0.00000, 1.00000)

***/

#include <algorithm>
#include <cstdio>
#include <iostream>
using namespace std;

void print_roots(vector<cdouble> x) {
  sort(x.begin(), x.end(), [](const cdouble &a, const cdouble &b) {
    return abs(a.real() - b.real()) > 0.5e-5 ? a.real() < b.real() : a.imag() < b.imag();
  });
  for (const auto &z : x) {
    double real = abs(z.real()) < 0.5e-5 ? 0 : z.real();
    double imag = abs(z.imag()) < 0.5e-5 ? 0 : z.imag();
    printf("(%.5lf, %.5lf)\n", real, imag);
  }
}

int main() {
  {  // 140 - 13x - 8x^2 + x^3 = (x + 4)(x - 5)(x - 7)
    printf("Roots of 140 - 13x - 8x^2 + x^3:\n");
    cpoly p;
    p.push_back(140);
    p.push_back(-13);
    p.push_back(-8);
    p.push_back(1);
    print_roots(find_all_roots(p));
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
    print_roots(find_all_roots(p));
  }
  return 0;
}
