/*

The fast Fourier transform (FFT) multiplies two polynomials, or equivalently convolves two
sequences, in O(n log n) time by evaluating them at the complex roots of unity. Like the number
theoretic transform of the previous section, it replaces a coefficient vector with its evaluations
at the powers of a root of unity, multiplies two such evaluation vectors pointwise, and transforms
back. The difference is the arithmetic: the FFT works over the complex numbers with $e^{2\pi i / n}$
as the root of unity, so it convolves arbitrary real coefficients rather than residues modulo a
prime.

The trade-off is precision. Complex arithmetic in double precision accumulates rounding error, so
the result is recovered by rounding each output to the nearest integer; this is reliable as long as
the true coefficients stay well within the roughly 15 significant decimal digits a `double` can
hold. When exact results modulo a prime are required, or coefficients are large, prefer the number
theoretic transform; use the FFT for real-valued convolution or big-integer multiplication.

- `fft(a, invert)` transforms the complex vector `a` in place, whose length must be a power of two.
  The forward transform uses `invert = false`; the inverse uses `invert = true`.
- `convolve(a, b)` returns the convolution of two integer sequences `a` and `b`, that is, the
  coefficients of the product of the two polynomials whose coefficients are the inputs. The result
  has length $`a.size()` + `b.size()` - 1$, or is empty if either input is empty, with each entry
  rounded to the nearest integer. Small inputs use direct multiplication to avoid FFT overhead.

Time Complexity:
- O(n log n) per call to `fft()`, where $n$ is the length of the vector.
- O(|a||b|) for the small-input branch of `convolve()`, otherwise O(n log n), where $n$ is the
  padded transform length.

Space Complexity:
- O(1) auxiliary for `fft()`, transforming in place.
- O(n) auxiliary heap space for `convolve()`.

*/

#include <algorithm>
#include <cmath>
#include <complex>
#include <cstdint>
#include <vector>

typedef std::complex<double> cd;
const double PI = acos(-1.0);
const int NAIVE_CUTOFF = 150;

void fft(std::vector<cd> &a, bool invert) {
  int n = static_cast<int>(a.size());
  for (int i = 1, j = 0; i < n; i++) {
    int bit = n >> 1;
    for (; j & bit; bit >>= 1) {
      j ^= bit;
    }
    j ^= bit;
    if (i < j) {
      std::swap(a[i], a[j]);
    }
  }
  for (int len = 2; len <= n; len <<= 1) {
    double angle = 2 * PI / len * (invert ? -1 : 1);
    cd root(cos(angle), sin(angle));
    for (int i = 0; i < n; i += len) {
      cd w(1);
      for (int k = 0; k < len / 2; k++) {
        cd u = a[i + k], v = a[i + k + len / 2] * w;
        a[i + k] = u + v;
        a[i + k + len / 2] = u - v;
        w *= root;
      }
    }
  }
  if (invert) {
    for (cd &x : a) {
      x /= n;
    }
  }
}

std::vector<int64_t> convolve(const std::vector<int64_t> &a, const std::vector<int64_t> &b) {
  if (a.empty() || b.empty()) {
    return {};
  }
  int result_size = a.size() + b.size() - 1;
  if (std::min(a.size(), b.size()) < NAIVE_CUTOFF) {
    std::vector<int64_t> res(result_size);
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      for (int j = 0; j < static_cast<int>(b.size()); j++) {
        res[i + j] += a[i] * b[j];
      }
    }
    return res;
  }
  int n = 1;
  while (n < result_size) {
    n <<= 1;
  }
  std::vector<cd> fa(a.begin(), a.end()), fb(b.begin(), b.end());
  fa.resize(n);
  fb.resize(n);
  fft(fa, false);
  fft(fb, false);
  for (int i = 0; i < n; i++) {
    fa[i] *= fb[i];
  }
  fft(fa, true);
  std::vector<int64_t> res(result_size);
  for (int i = 0; i < result_size; i++) {
    res[i] = llround(fa[i].real());
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // (1 + 2x + 3x^2)(4 + 5x + 6x^2) = 4 + 13x + 28x^2 + 27x^3 + 18x^4.
  vector<int64_t> a{1, 2, 3}, b{4, 5, 6};
  vector<int64_t> c = convolve(a, b);
  assert((c == vector<int64_t>{4, 13, 28, 27, 18}));
  vector<int64_t> ones(160, 1);
  vector<int64_t> d = convolve(ones, ones);
  assert(d[0] == 1 && d[159] == 160 && d[318] == 1);
  return 0;
}
