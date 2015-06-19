/*

4.4.3 - Fast Fourier Transform and Multiplication

A discrete Fourier transform (DFT) converts a list of equally
spaced samples of a function into the list of coefficients of
a finite combination of complex sinusoids, ordered by their
frequencies, that has those same sample values. A Fast Fourier
Transform (FFT) rapidly computes the DFT by factorizing the
DFT matrix into a product of sparse (mostly zero) factors.
The FFT can be used to solve problems such as efficiently
multiplying big integers or polynomials

The fft() function below is a generic function that will
work well in many applications beyond just multiplying
big integers. While Karatsuba multiplication is ~ O(n^1.58),
the complexity of the fft multiplication is only O(n log n).

Note that mul(string, string) in the following implementation
only works for strings of strictly digits from '0' to '9'.
It is also easy to adapt this for the bigint class in the
previous section. Simply replace the old bigint operator *
definition with the following modified version of mul():

  bigint operator * (const bigint & v) const {
    static const int _base = 10000, _base_digits = 4;
    vint _a = convert_base(this->a, base_digits, _base_digits);
    vint _b = convert_base(v.a, base_digits, _base_digits);
    int len = 32 - __builtin_clz(std::max(_a.size(), _b.size()) - 1);
    len = 1 << (len + 1);
    vcd a(len), b(len);
    for (int i = 0; i < _a.size(); i++) a[i] = cd(_a[i], 0);
    for (int i = 0; i < _b.size(); i++) b[i] = cd(_b[i], 0);
    a = fft(a);
    b = fft(b);
    for (int i = 0; i < len; i++) {
      double real = a[i].real() * b[i].real() - a[i].imag() * b[i].imag();
      a[i].imag() = a[i].imag() * b[i].real() + b[i].imag() * a[i].real();
      a[i].real() = real;
    }
    a = fft(a, true);
    vll c(len);
    for (int i = 0; i < len; i++) c[i] = (long long)(a[i].real() + 0.5);
    bigint res;
    res.sign = sign * v.sign;
    for (int i = 0, carry = 0; i < c.size(); i++) {
      long long cur = c[i] + carry;
      res.a.push_back((int)(cur % _base));
      carry = (int)(cur / _base);
    }
    res.a = convert_base(res.a, _base_digits, base_digits);
    res.trim();
    return res;
  }

*/

#include <algorithm> /* std::max(), std::reverse() */
#include <cmath>     /* M_PI, cos(), sin() */
#include <complex>
#include <iomanip>   /* std::setw(), std::setfill() */
#include <sstream>
#include <string>
#include <vector>

typedef std::complex<double> cd;
typedef std::vector<cd> vcd;

vcd fft(const vcd & v, bool inverse = false) {
  static const double PI = acos(-1.0);
  int n = v.size(), k = 0, high1 = -1;
  while ((1 << k) < n) k++;
  std::vector<int> rev(n);
  rev[0] = 0;
  for (int i = 1; i < n; i++) {
    if ((i & (i - 1)) == 0) high1++;
    rev[i] = rev[i ^ (1 << high1)];
    rev[i] |= (1 << (k - high1 - 1));
  }
  vcd roots(n), res(n);
  for (int i = 0; i < n; i++) {
    double alpha = 2 * PI * i / n;
    roots[i] = cd(cos(alpha), sin(alpha));
  }
  for (int i = 0; i < n; i++) res[i] = v[rev[i]];
  for (int len = 1; len < n; len <<= 1) {
    vcd tmp(n);
    int rstep = roots.size() / (len * 2);
    for (int pdest = 0; pdest < n;) {
      int p1 = pdest;
      for (int i = 0; i < len; i++) {
        cd val = roots[i * rstep] * res[p1 + len];
        tmp[pdest] = res[p1] + val;
        tmp[pdest + len] = res[p1] - val;
        pdest++, p1++;
      }
      pdest += len;
    }
    res.swap(tmp);
  }
  if (inverse) {
    for (int i = 0; i < res.size(); i++) res[i] /= v.size();
    std::reverse(res.begin() + 1, res.end());
  }
  return res;
}

typedef std::vector<long long> vll;

vll mul(const vll & va, const vll & vb) {
  int len = 32 - __builtin_clz(std::max(va.size(), vb.size()) - 1);
  len = 1 << (len + 1);
  vcd a(len), b(len);
  for (int i = 0; i < va.size(); i++) a[i] = cd(va[i], 0);
  for (int i = 0; i < vb.size(); i++) b[i] = cd(vb[i], 0);
  a = fft(a);
  b = fft(b);
  for (int i = 0; i < len; i++) {
    double real = a[i].real() * b[i].real() - a[i].imag() * b[i].imag();
    a[i].imag() = a[i].imag() * b[i].real() + b[i].imag() * a[i].real();
    a[i].real() = real;
  }
  a = fft(a, true);
  vll res(len);
  for (int i = 0; i < len; i++) res[i] = (long long)(a[i].real() + 0.5);
  return res;
}

const int base = 10000, base_digits = 4;

std::string mul(const std::string & as, const std::string & bs) {
  vll a, b;
  for (int i = as.size() - 1; i >= 0; i -= base_digits) {
    int x = 0;
    for (int j = std::max(0, i - base_digits + 1); j <= i; j++)
      x = x * 10 + as[j] - '0';
    a.push_back(x);
  }
  for (int i = bs.size() - 1; i >= 0; i -= base_digits) {
    int x = 0;
    for (int j = std::max(0, i - base_digits + 1); j <= i; j++)
      x = x * 10 + bs[j] - '0';
    b.push_back(x);
  }
  vll c = mul(a, b);
  long long carry = 0;
  for (int i = 0; i < c.size(); i++) {
    c[i] += carry;
    carry = c[i] / base;
    c[i] %= base;
  }
  while (c.back() == 0) c.pop_back();
  if (c.empty()) c.push_back(0);
  std::ostringstream oss;
  oss << (c.empty() ? 0 : c.back());
  for (int i = c.size() - 2; i >= 0; i--)
    oss << std::setw(base_digits) << std::setfill('0') << c[i];
  return oss.str();
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(mul("98904189", "244212") == "24153589804068");
  return 0;
}
