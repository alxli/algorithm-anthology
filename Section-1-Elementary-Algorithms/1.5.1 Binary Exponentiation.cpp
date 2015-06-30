/*

1.5.1 - Binary Exponentiation

Exponentiation by squaring is a general method for fast
computation of large positive integer powers of a number.
This method is also known as "square-and-multiply." The
following version computes x^n modulo m. Remove the "% m"
if you do not want the answer to be modded - but as this
causes overflow, you should switch to a bigger data type.

Complexity: O(log n) on the exponent of the computation.

*/

#include <stdint.h>

uint64_t mulmod(uint64_t a, uint64_t b, uint64_t m) {
  uint64_t x = 0, y = a % m;
  for (; b > 0; b >>= 1) {
    if (b & 1) x = (x + y) % m;
    y = (y << 1) % m;
  }
  return x % m;
}

uint64_t powmod(uint64_t a, uint64_t b, uint64_t m) {
  uint64_t x = 1, y = a;
  for (; b > 0; b >>= 1) {
    if (b & 1) x = mulmod(x, y, m);
    y = mulmod(y, y, m);
  }
  return x % m;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(powmod(2, 10, 1000000007) == 1024);
  return 0;
}
