/*

`powmod()` and `mulmod()` compute modular powers and products using binary exponentiation, also
known as exponentiation by squaring, which decomposes the operation into a logarithmic number of
multiplications while avoiding overflow. To further prevent overflow in the intermediate squaring
computations, multiplication is itself performed using a similar double-and-add principle of
repeated addition.

- `mulmod(x, n, m)` returns `x` multiplied by `n`, modulo `m`.
- `powmod(x, n, m)` returns `x` raised to the power `n`, modulo `m`.

All arguments are unsigned 64-bit integers, but `x` and `n` must not exceed $2^{63} - 1$ (the
maximum value of a signed 64-bit integer) for the result to be computed correctly without overflow.

Time Complexity:
- O(log n) per call to `mulmod()` and `powmod()`, where $n$ is the second argument.

Space Complexity:
- O(1) auxiliary.

*/

using uint64 = unsigned long long;

// Hot path: Can simply `return (unsigned __int128)x * n % m;` for a major constant-factor speedup
// if the GCC/Clang __int128 extension is available. Otherwise, use the portable double-and-add
// version below, which still won't overflow a signed 64-bit `long long`.
uint64 mulmod(uint64 x, uint64 n, uint64 m) {
  uint64 a = 0, b = x % m;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = (a + b) % m;
    }
    b = (b << 1) % m;
  }
  return a % m;
}

uint64 powmod(uint64 x, uint64 n, uint64 m) {
  uint64 a = 1, b = x;
  for (; n > 0; n >>= 1) {
    if (n & 1) {
      a = mulmod(a, b, m);
    }
    b = mulmod(b, b, m);
  }
  return a % m;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(powmod(2, 10, 1000000007) == 1024);
  assert(powmod(2, 62, 1000000) == 387904);
  assert(powmod(10001, 10001, 100000) == 10001);
  return 0;
}
