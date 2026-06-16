/*

Computes the $n$-th term of a linear recurrence modulo a prime in logarithmic time. Given a
recurrence of order $L$, $s_i = c_0 s_{i-1} + c_1 s_{i-2} + \ldots + c_{L-1} s_{i-L}$, together with
the first $L$ terms, the naive approach unrolls $n$ steps. Kitamasa's method instead computes $n$
terms ahead in O(L^2 log n) by working with the characteristic polynomial. The coefficient layout
matches the output of Berlekamp-Massey, so the two compose directly: guess the recurrence from
sampled values, then jump to any index.

The key identity is that the $n$-th term is a fixed linear combination of the first $L$ terms, with
weights given by $x^n$ reduced modulo the characteristic polynomial
$f(x) = x^L - c_0 x^{L-1} - ... - c_{L-1}$. The reduction $x^L = c_0 x^{L-1} + ... + c_{L-1}$ lets
any product of two degree-$<L$ polynomials be folded back to degree $<L$, so $x^n \mod f$ is
obtained by binary exponentiation, and the $n$-th term is the dot product of its coefficients with
the initial terms.

- `kth_term(rec, init, n)` returns $s_n$ modulo `MOD`, where `rec` holds the coefficients
  $c_0, ..., c_{L-1}$ and `init` holds at least $L$ initial terms $s_0, ..., s_{L-1}$. Indexing is
  0-based and `n` $\geq 0$. Values are reduced modulo `MOD` internally.

Time Complexity:
- O(L^2 log n) per call to `kth_term()`.

Space Complexity:
- O(L) auxiliary heap space.

*/

#include <cstdint>
#include <vector>

const int64_t MOD = 998244353;

void reduce(std::vector<int64_t> &p, const std::vector<int64_t> &c) {
  int L = static_cast<int>(c.size());
  for (int d = static_cast<int>(p.size()) - 1; d >= L; d--) {
    if (p[d] != 0) {
      for (int j = 0; j < L; j++) {
        p[d - 1 - j] = (p[d - 1 - j] + p[d] * c[j]) % MOD;
      }
      p[d] = 0;
    }
  }
  p.resize(L);
}

std::vector<int64_t> combine(
    const std::vector<int64_t> &a, const std::vector<int64_t> &b, const std::vector<int64_t> &c
) {
  std::vector<int64_t> res(a.size() + b.size() - 1, 0);
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    for (int j = 0; j < static_cast<int>(b.size()); j++) {
      res[i + j] = (res[i + j] + a[i] * b[j]) % MOD;
    }
  }
  reduce(res, c);
  return res;
}

int64_t kth_term(const std::vector<int64_t> &rec, const std::vector<int64_t> &init, int64_t n) {
  int L = static_cast<int>(rec.size());
  if (n < static_cast<int64_t>(init.size())) {
    return ((init[n] % MOD) + MOD) % MOD;
  }
  if (L == 0) {
    return 0;
  }
  std::vector<int64_t> result(L, 0), base(L, 0);
  result[0] = 1;  // The polynomial 1 = x^0.
  if (L == 1) {
    base[0] = rec[0] % MOD;  // x is congruent to c_0 modulo f.
  } else {
    base[1] = 1;  // The polynomial x.
  }
  for (int64_t e = n; e > 0; e >>= 1) {
    if (e & 1) {
      result = combine(result, base, rec);
    }
    base = combine(base, base, rec);
  }
  int64_t ans = 0;
  for (int j = 0; j < L; j++) {
    ans = (ans + result[j] * (((init[j] % MOD) + MOD) % MOD)) % MOD;
  }
  return ans;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Fibonacci s_i = s_{i-1} + s_{i-2} with s_0 = s_1 = 1: 1, 1, 2, 3, 5, 8, ...
  vector<int64_t> fib_rec{1, 1}, fib_init{1, 1};
  assert(kth_term(fib_rec, fib_init, 0) == 1);
  assert(kth_term(fib_rec, fib_init, 9) == 55);
  assert(kth_term(fib_rec, fib_init, 10) == 89);

  // Tribonacci s_i = s_{i-1} + s_{i-2} + s_{i-3} with s_0,s_1,s_2 = 0,1,1.
  vector<int64_t> trib_rec{1, 1, 1}, trib_init{0, 1, 1};
  assert(kth_term(trib_rec, trib_init, 8) == 44);

  // Geometric s_i = 3 s_{i-1}, s_0 = 1: matches 3^n modulo MOD.
  vector<int64_t> geo_rec{3}, geo_init{1};
  int64_t pw = 1;
  for (int i = 0; i < 50; i++) {
    assert(kth_term(geo_rec, geo_init, i) == pw);
    pw = pw * 3 % MOD;
  }
  return 0;
}
