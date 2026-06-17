/*

Recovers the unique polynomial of degree less than $n$ that passes through $n$ given points, working
modulo a prime. The $x$ coordinates must be distinct; the $y$ coordinates may be anything. Two
operations are provided: reconstructing the full coefficient vector, and evaluating the
interpolating polynomial at a single point without ever forming the coefficients.

Coefficient recovery uses Newton's divided differences. It first replaces the sample values by their
successive divided differences in place, then expands the Newton basis $1$, $(x - x_0)$,
$(x - x_0)(x - x_1)$, $\ldots$ into ordinary coefficients by multiplying through one factor at a
time. Point evaluation instead applies the Lagrange formula directly, summing each sample weighted
by the product of $(t - x_j) / (x_k - x_j)$ over the other nodes. Both run in O(n^2) modular
operations.

- `interpolate(x, y)` returns the coefficients $a_0, ..., a_{n-1}$ (constant term first) of the
  polynomial $P$ with $P(`x[i]`) = `y[i]`$, modulo `MOD`. The entries of `x` must be distinct modulo
  `MOD`.
- `interpolate_at(x, y, t)` returns $P(`t`)$ modulo `MOD` for that same polynomial, without building
  its coefficients.

Time Complexity:
- O(n^2) per call to `interpolate()` and `interpolate_at()`, where $n$ is the number of points.

Space Complexity:
- O(n) auxiliary heap space.

*/

#include <cstdint>
#include <vector>

const int64_t MOD = 998244353;

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

int64_t inv(int64_t a) {
  return powmod(((a % MOD) + MOD) % MOD, MOD - 2, MOD);
}

std::vector<int64_t> interpolate(std::vector<int64_t> x, std::vector<int64_t> y) {
  int n = static_cast<int>(x.size());
  for (int i = 0; i < n; i++) {
    x[i] = ((x[i] % MOD) + MOD) % MOD;
    y[i] = ((y[i] % MOD) + MOD) % MOD;
  }
  // Replace y by its divided differences in place.
  for (int k = 0; k < n; k++) {
    for (int i = k + 1; i < n; i++) {
      y[i] = (y[i] - y[k] + MOD) % MOD * inv((x[i] - x[k] + MOD) % MOD) % MOD;
    }
  }
  // Expand the Newton basis into ordinary coefficients.
  std::vector<int64_t> res(n, 0), temp(n, 0);
  temp[0] = 1;
  for (int k = 0; k < n; k++) {
    int64_t last = 0;
    for (int i = 0; i < n; i++) {
      res[i] = (res[i] + y[k] * temp[i]) % MOD;
      int64_t old = temp[i];
      temp[i] = last;
      last = old;
      temp[i] = (temp[i] - last * x[k] % MOD + MOD) % MOD;
    }
  }
  return res;
}

int64_t interpolate_at(const std::vector<int64_t> &x, const std::vector<int64_t> &y, int64_t t) {
  int n = static_cast<int>(x.size());
  t = ((t % MOD) + MOD) % MOD;
  int64_t res = 0;
  for (int k = 0; k < n; k++) {
    int64_t num = 1, den = 1;
    for (int j = 0; j < n; j++) {
      if (j != k) {
        num = num * ((t - x[j]) % MOD + MOD) % MOD;
        den = den * ((x[k] - x[j]) % MOD + MOD) % MOD;
      }
    }
    res = (res + ((y[k] % MOD) + MOD) % MOD * num % MOD * inv(den)) % MOD;
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // P(x) = x^2 + 2x + 3 sampled at x = 0, 1, 2 gives y = 3, 6, 11.
  vector<int64_t> x{0, 1, 2}, y{3, 6, 11};
  vector<int64_t> coef = interpolate(x, y);
  assert((coef == vector<int64_t>{3, 2, 1}));
  assert(interpolate_at(x, y, 5) == 38);  // 25 + 10 + 3.

  // Non-consecutive nodes for P(x) = 2x^3 - x + 4.
  auto poly = [](int64_t t) { return ((2 * t * t * t - t + 4) % MOD + MOD) % MOD; };
  vector<int64_t> xs{2, 5, 7, 10}, ys;
  for (int64_t v : xs) {
    ys.push_back(poly(v));
  }
  vector<int64_t> c = interpolate(xs, ys);
  assert((c == vector<int64_t>{4, MOD - 1, 0, 2}));  // 4 - x + 2x^3.
  assert(interpolate_at(xs, ys, 13) == poly(13));
  return 0;
}
