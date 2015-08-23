/*

4.6.2 - Complex Root Finding (Laguerre's Method)

Laguerre's method can be used to not only find complex roots of
a polynomial, the polynomial may also have complex coefficients.
From extensive empirical study, Laguerre's method is observed to
be very close to being a "sure-fire" method, as it is almost
guaranteed to always converge to some root of the polynomial
regardless of what initial guess is chosen.

*/

#include <complex>
#include <cstdlib> /* rand(), RAND_MAX */
#include <vector>

typedef long double Double;
typedef std::complex<Double> cdouble;
typedef std::vector<cdouble> poly;

const Double eps = 1e-12;

std::pair<poly, cdouble> horner(const poly & a, const cdouble & x) {
  int n = a.size();
  poly b = poly(std::max(1, n - 1));
  for (int i = n - 1; i > 0; i--)
    b[i - 1] = a[i] + (i < n - 1 ? b[i] * x : 0);
  return std::make_pair(b, a[0] + b[0] * x);
}

cdouble eval(const poly & p, const cdouble & x) {
  return horner(p, x).second;
}

poly derivative(const poly & p) {
  int n = p.size();
  poly r(std::max(1, n - 1));
  for(int i = 1; i < n; i++)
    r[i - 1] = p[i] * cdouble(i);
  return r;
}

int comp(const cdouble & x, const cdouble & y) {
  Double diff = std::abs(x) - std::abs(y);
  return diff < -eps ? -1 : (diff > eps ? 1 : 0);
}

cdouble find_one_root(const poly & p, cdouble x) {
  int n = p.size() - 1;
  poly p1 = derivative(p), p2 = derivative(p1);
  for (int step = 0; step < 10000; step++) {
    cdouble y0 = eval(p, x);
    if (comp(y0, 0) == 0) break;
    cdouble G = eval(p1, x) / y0;
    cdouble H = G * G - eval(p2, x) / y0;
    cdouble R = std::sqrt(cdouble(n - 1) * (H * cdouble(n) - G * G));
    cdouble D1 = G + R, D2 = G - R;
    cdouble a = cdouble(n) / (comp(D1, D2) > 0 ? D1 : D2);
    x -= a;
    if (comp(a, 0) == 0) break;
  }
  return x;
}

std::vector<cdouble> find_all_roots(const poly & p) {
  std::vector<cdouble> res;
  poly q = p;
  while (q.size() > 2) {
    cdouble z(rand()/Double(RAND_MAX), rand()/Double(RAND_MAX));
    z = find_one_root(q, z);
    z = find_one_root(p, z);
    q = horner(q, z).first;
    res.push_back(z);
  }
  res.push_back(-q[0] / q[1]);
  return res;
}

/*** Example Usage ***/

#include <cstdio>
#include <iostream>
using namespace std;

void print_roots(vector<cdouble> roots) {
  for (int i = 0; i < (int)roots.size(); i++) {
    printf("(%9.5f, ", (double)roots[i].real());
    printf("%9.5f)\n", (double)roots[i].imag());
  }
}

int main() {
  { // x^3 - 8x^2 - 13x + 140 = (x + 4)(x - 5)(x - 7)
    printf("Roots of x^3 - 8x^2 - 13x + 140:\n");
    poly p;
    p.push_back(140);
    p.push_back(-13);
    p.push_back(-8);
    p.push_back(1);
    vector<cdouble> roots = find_all_roots(p);
    print_roots(roots);
  }

  { //(-6+4i)x^4 + (-26+12i)x^3 + (-30+40i)x^2 + (-26+12i)x + (-24+36i)
    // = ((2+3i)x + 6)*(x + i)*(2x + (6+4i))*(x*i + 1)
    printf("Roots of ((2+3i)x + 6)(x + i)(2x + (6+4i))(x*i + 1):\n");
    poly p;
    p.push_back(cdouble(-24, 36));
    p.push_back(cdouble(-26, 12));
    p.push_back(cdouble(-30, 40));
    p.push_back(cdouble(-26, 12));
    p.push_back(cdouble(-6, 4));
    vector<cdouble> roots = find_all_roots(p);
    print_roots(roots);
  }
  return 0;
}
