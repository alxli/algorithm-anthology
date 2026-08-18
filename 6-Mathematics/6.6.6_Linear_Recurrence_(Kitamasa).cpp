/*

Computes the $n$-th term of a linear recurrence modulo a prime in logarithmic time. Given a
recurrence of order $L$, $s_i = c_0 s_{i-1} + c_1 s_{i-2} + \ldots + c_{L-1} s_{i-L}$, together with
the first $L$ terms, the naive approach unrolls $n$ steps. Kitamasa's method instead jumps directly
to index $n$ by working with the characteristic polynomial. The coefficient layout matches the
output of Berlekamp-Massey, so the two compose directly: guess the recurrence from sampled values,
then jump to any index.

The key identity is that the $n$-th term is a fixed linear combination of the first $L$ terms, with
weights given by $x^n$ reduced modulo the characteristic polynomial
$f(x) = x^L - c_0 x^{L-1} - ... - c_{L-1}$. The reduction $x^L = c_0 x^{L-1} + ... + c_{L-1}$ lets
any product of two degree-$<L$ polynomials be folded back to degree $<L$, so $x^n \mod f$ is
obtained by binary exponentiation, and the $n$-th term is the dot product of its coefficients with
the initial terms.

Each doubling step is one polynomial multiplication followed by one remainder, both taken from
section 6.6.3, where the multiplication is a number theoretic transform and the remainder is a
Newton-iterated division. Squaring and reducing by hand instead costs O(L^2) per step, which is
simpler to write but asymptotically worse; `MOD` is inherited from that section and must stay
transform-friendly.

- `kth_term(rec, init, n)` returns $s_n$ modulo `MOD`, where `rec` holds the coefficients
  $c_0, ..., c_{L-1}$ and `init` holds at least $L$ initial terms $s_0, ..., s_{L-1}$. Indexing is
  0-based and `n` $\geq 0$. Values are reduced modulo `MOD` internally.

Time Complexity:
- O(L log L log n) per call.

Space Complexity:
- O(L) auxiliary.

*/

#include <cassert>
#include <cstdint>
#include <vector>

#define main polynomial_example
#include "6.6.3_Polynomial_Operations.cpp"
#undef main

int64_t kth_term(const std::vector<int64_t> &rec, const std::vector<int64_t> &init, int64_t n) {
  assert(n >= 0 && init.size() >= rec.size());
  int L = static_cast<int>(rec.size());
  if (n < static_cast<int64_t>(init.size())) {
    return ((init[n] % MOD) + MOD) % MOD;
  }
  if (L == 0) {
    return 0;
  }
  // The characteristic polynomial f(x) = x^L - c_0 x^(L-1) - ... - c_(L-1), indexed by degree.
  Poly f(L + 1);
  f[L] = 1;
  for (int j = 0; j < L; j++) {
    f[L - 1 - j] = ((-rec[j]) % MOD + MOD) % MOD;
  }
  Poly result{1}, base = mod(Poly{0, 1}, f);  // The polynomials x^0 and x, each modulo f.
  for (int64_t e = n; e > 0; e >>= 1) {
    if (e & 1) {
      result = mod(multiply(result, base), f);
    }
    base = mod(multiply(base, base), f);
  }
  int64_t ans = 0;
  for (int j = 0; j < L && j < static_cast<int>(result.size()); j++) {
    ans = (ans + result[j] * (((init[j] % MOD) + MOD) % MOD)) % MOD;
  }
  return ans;
}

/*** Example Usage ***/

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
  assert(kth_term({-1}, {1}, 5) == MOD - 1);
  return 0;
}
