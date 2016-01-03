/*

1.5.1 - Binary Exponentiation

Given three positive, signed 64-bit integers, powmod() efficiently
computes the power of the first two integers, modulo the third integer.
Binary exponentiation, also known as "exponentiation by squaring,"
decomposes the computation with the observation that the exponent is
reduced by half whenever the base is squared. Odd-numbered exponents
can be dealt with by subtracting one and multiplying the overall
expression by the base of the power. This yields a logarithmic number
of multiplications while avoiding overflow. To further prevent overflow
in intermediate multiplications, multiplication can be done using the
similar principle of multiplication by adding. Despite using unsigned
64-bit integers for intermediate calculations and as parameter types,
each argument to powmod() must not exceed 2^63 - 1, the maximum value
of a signed 64-bit integer.

Time Complexity: O(log n) on the exponent of the power.
Space Complexity: O(1) auxiliary.

*/

typedef unsigned long long int64;

int64 mulmod(int64 a, int64 b, int64 m) {
  int64 x = 0, y = a % m;
  for (; b > 0; b >>= 1) {
    if (b & 1)
      x = (x + y) % m;
    y = (y << 1) % m;
  }
  return x % m;
}

int64 powmod(int64 a, int64 b, int64 m) {
  int64 x = 1, y = a;
  for (; b > 0; b >>= 1) {
    if (b & 1)
      x = mulmod(x, y, m);
    y = mulmod(y, y, m);
  }
  return x % m;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(powmod(2, 10, 1000000007) == 1024);
  assert(powmod(2, 62, 1000000) == 387904);
  assert(powmod(10001, 10001, 100000) == 10001);
  return 0;
}
