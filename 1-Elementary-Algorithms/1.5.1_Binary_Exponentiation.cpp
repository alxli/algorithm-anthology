/*

Given three unsigned 64-bit integers x, n, and m, powmod() returns x raised to
the power of n (modulo m). mulmod() returns x multiplied by n (modulo m).
Despite the fact that both functions use unsigned 64-bit integers for arguments
and intermediate calculations, arguments x and n must not exceed 2^63 - 1 (the
maximum value of a signed 64-bit integer) for the result to be correctly
computed without overflow.

Binary exponentiation, also known as exponentiation by squaring, decomposes the
exponentiation into a logarithmic number of multiplications while avoiding
overflow. To further prevent overflow in the intermediate squaring computations,
multiplication is performed using a similar principle of repeated addition.

Time Complexity:
- O(log n) per call to mulmod() and powmod(), where n is the second argument.

Space Complexity:
- O(1) auxiliary.

*/

typedef unsigned long long uint64;

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
