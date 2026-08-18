/*

Modular arithmetic replaces every value by its remainder modulo `m`, which keeps intermediate
results bounded and is what makes counting problems with astronomically large answers tractable. The
operations are straightforward except for two hazards, both handled below: a sum or product can
overflow the underlying integer type before the remainder is taken, and a negative operand leaves a
negative remainder under C++ truncating division. Binary exponentiation squares the base while
walking the bits of the exponent, so a power costs O(log n) multiplications rather than $n$; the
same loop computes powers in any monoid, which is how the matrix exponentiation of section 6.5.1 and
the linear recurrence of section 6.6.6 are evaluated.

- `addmod(a, b, m)` and `submod(a, b, m)` respectively return addition and subtraction modulo `m`,
  each result in $[0, `m`)$. Both operands may be negative or unreduced; they are normalized before
  arithmetic to avoid signed overflow. The modulus `m` must be positive.
- `mulmod(a, b, m)` returns `a` multiplied by `b`, modulo `m`. This is done in a way to avoid
  overflow: on compilers with `__uint128_t` it uses one wide product, while the portable fallback
  uses double-and-add multiplication. The fallback is slower by a logarithmic factor, but avoids
  relying on nonstandard 128-bit integers. Unlike `addmod`/`submod`, this takes unsigned operands
  and supports a full 64-bit modulus.
- `powmod(x, n, m)` returns `x` raised to the power `n`, modulo `m`.

Paste these when a solution needs a handful of modular operations; when modular arithmetic pervades
one instead, the `Modular` value type of section 6.3.3 overloads the operators so expressions read
normally. Only `mulmod()` has no counterpart there, since a value type multiplying in `int64_t`
overflows once the modulus passes about three billion, which is why it underlies the Miller-Rabin
test and Pollard's rho of section 6.3.9.

Time Complexity:
- O(1) per call to `addmod()` and `submod()`.
- O(1) per call to `mulmod()` with `__uint128_t`, or O(log b) with the portable fallback, where $b$
  is the second argument.
- O(log n) calls to `mulmod()` per call to `powmod(x, n, m)`.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <cstdint>

int64_t addmod(int64_t a, int64_t b, int64_t m) {
  assert(m > 0);
  if ((a %= m) < 0) a += m;
  if ((b %= m) < 0) b += m;
  return a >= m - b ? a - (m - b) : a + b;
}

int64_t submod(int64_t a, int64_t b, int64_t m) {
  assert(m > 0);
  if ((a %= m) < 0) a += m;
  if ((b %= m) < 0) b += m;
  return a >= b ? a - b : m - (b - a);
}

uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
  assert(m > 0);
#if defined(__SIZEOF_INT128__)
  return static_cast<uint64_t>(static_cast<__uint128_t>(a) * b % m);
#else
  uint64_t res = 0, cur = a % m;
  for (; b > 0; b >>= 1) {
    if (b & 1) {
      res = res >= m - cur ? res - (m - cur) : res + cur;
    }
    cur = cur >= m - cur ? cur - (m - cur) : cur + cur;
  }
  return res;
#endif
}

uint64_t powmod(uint64_t x, uint64_t n, uint64_t m) {
  assert(m > 0);
  uint64_t res = 1 % m;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      res = mulmod(res, x, m);
    }
    x = mulmod(x, x, m);
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(addmod(7, 8, 10) == 5 && submod(2, 5, 10) == 7);
  // Negative and unreduced operands are normalized into [0, m).
  assert(addmod(-3, -4, 10) == 3 && submod(-3, 4, 10) == 3);
  assert(addmod(25, -7, 10) == 8);
  assert(addmod(INT64_MAX - 1, INT64_MAX - 1, INT64_MAX) == INT64_MAX - 2);

  assert(mulmod(INT64_MAX - 1, INT64_MAX - 1, INT64_MAX) == 1);
  assert(powmod(2, 10, 1000000007) == 1024);
  assert(powmod(2, 62, 1000000) == 387904);
  assert(powmod(10001, 10001, 100000) == 10001);
  return 0;
}
