/*

The number theoretic transform (NTT) is the discrete Fourier transform carried out in modular
arithmetic instead of over the complex numbers. It multiplies two polynomials (equivalently,
computes the convolution of two sequences) modulo a prime in O(n log n) time, with no floating
point error. The modulus must be of the form $c \cdot 2^k + 1$ so that the ring of integers modulo
it contains a $2^k$-th root of unity, which limits transform sizes to powers of two up to $2^k$. The
code below uses the common prime $998244353 = 119 \cdot 2^{23} + 1$ with primitive root $3$,
supporting transforms of any power-of-two length up to $2^{23}$.

The transform replaces a coefficient vector with its evaluations at the powers of a root of unity;
pointwise multiplying two such evaluation vectors and transforming back gives their convolution.
This is the same divide-and-conquer butterfly as the fast Fourier transform, with the root of unity
and all arithmetic taken modulo the prime.

- `ntt(a, invert)` transforms the vector `a` in place, whose length must be a power of two. The
  forward transform uses `invert = false`; the inverse uses `invert = true`.
- `convolve(a, b)` returns the convolution of `a` and `b` modulo the prime, that is, the
  coefficients of the product of the two polynomials whose coefficients are the inputs. The result
  has length $`a.size()` + `b.size()` - 1$, or is empty if either input is empty. Input values are
  assumed to lie in $[0, `MOD`)$. Small inputs use direct multiplication to avoid NTT overhead.

Time Complexity:
- O(n log n) per call to `ntt()`, where $n$ is the length of the vector.
- O(|a||b|) for the small-input branch of `convolve()`, otherwise O(n log n), where $n$ is the
  padded transform length.

Space Complexity:
- O(1) auxiliary for `ntt()`, transforming in place.
- O(n) auxiliary heap space for `convolve()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

const int64_t MOD = 998244353;
const int64_t ROOT = 3;
const int MAX_POWER_OF_TWO = 23;
const int NAIVE_CUTOFF = 150;

int64_t powmod(int64_t b, int64_t e, int64_t m) {
  int64_t res = 1;
  for (b %= m; e > 0; e >>= 1) {
    if (e & 1) {
      res = res * b % m;
    }
    b = b * b % m;
  }
  return res;
}

void ntt(std::vector<int64_t> &a, bool invert) {
  int n = static_cast<int>(a.size());
  assert((n & (n - 1)) == 0 && n <= (1 << MAX_POWER_OF_TWO));
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
    int64_t root = powmod(ROOT, (MOD - 1) / len, MOD);
    if (invert) {
      root = powmod(root, MOD - 2, MOD);
    }
    for (int i = 0; i < n; i += len) {
      int64_t w = 1;
      for (int k = 0; k < len / 2; k++) {
        int64_t u = a[i + k], v = a[i + k + len / 2] * w % MOD;
        a[i + k] = (u + v) % MOD;
        a[i + k + len / 2] = (u - v + MOD) % MOD;
        w = w * root % MOD;
      }
    }
  }
  if (invert) {
    int64_t n_inv = powmod(n, MOD - 2, MOD);
    for (int64_t &x : a) {
      x = x * n_inv % MOD;
    }
  }
}

std::vector<int64_t> convolve(std::vector<int64_t> a, std::vector<int64_t> b) {
  if (a.empty() || b.empty()) {
    return {};
  }
  int result_size = a.size() + b.size() - 1;
  if (std::min(a.size(), b.size()) < NAIVE_CUTOFF) {
    std::vector<int64_t> res(result_size);
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      for (int j = 0; j < static_cast<int>(b.size()); j++) {
        res[i + j] = (res[i + j] + a[i] * b[j]) % MOD;
      }
    }
    return res;
  }
  int n = 1;
  while (n < result_size) {
    n <<= 1;
  }
  assert(n <= (1 << MAX_POWER_OF_TWO));
  a.resize(n);
  b.resize(n);
  ntt(a, false);
  ntt(b, false);
  for (int i = 0; i < n; i++) {
    a[i] = a[i] * b[i] % MOD;
  }
  ntt(a, true);
  a.resize(result_size);
  return a;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // (1 + 2x + 3x^2)(4 + 5x + 6x^2) = 4 + 13x + 28x^2 + 27x^3 + 18x^4.
  vector<int64_t> a{1, 2, 3}, b{4, 5, 6};
  vector<int64_t> c = convolve(a, b);
  assert((c == vector<int64_t>{4, 13, 28, 27, 18}));
  assert(convolve({}, b).empty());
  assert((convolve(vector<int64_t>{5}, vector<int64_t>{7}) == vector<int64_t>{35}));
  vector<int64_t> ones(160, 1);
  vector<int64_t> d = convolve(ones, ones);
  assert(d[0] == 1 && d[159] == 160 && d[318] == 1);
  return 0;
}
