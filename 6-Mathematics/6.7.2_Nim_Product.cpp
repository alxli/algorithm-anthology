/*

Nimbers are the values of impartial games, added by XOR (see the Nim section). Under the additional
operation of nim multiplication they form a field: the nonnegative integers below $2^{2^k}$ are
closed under both operations, with XOR as addition and nim product as multiplication. Writing
$\otimes$ for the nim product and $\oplus$ for XOR, nim multiplication is commutative, associative,
and distributive over $\oplus$, and is defined recursively by $a \otimes b =$
$\operatorname{mex}\{(a' \otimes b) \oplus (a \otimes b') \oplus (a' \otimes b') : 0 \le a' < a, \, 0 \le b' < b\}$.
Its main use is multiplying the Grundy values of independent "coin-turning" games whose move sets
combine as a product, such as two-dimensional turning games built from one-dimensional ones.

Rather than evaluate the recursive definition directly, this implementation precomputes the nim
product of every pair of single bits $2^i \otimes 2^j$ for $i, j < 64$. Each such product follows
from the Fermat-like rule $2^{2^k} \otimes 2^{2^k} = 2^{2^k} \oplus 2^{2^k - 1}$ and the fact that
distinct Fermat powers multiply like ordinary powers of two. A full product then XORs together the
bit-pair products selected by the set bits of the operands. Because every 64-bit value lies in the
field of size $2^{64}$, the operation is total on `uint64_t` and the nonzero values have
multiplicative inverses.

- `nim_product(x, y)` returns the nim product of `x` and `y`.
- `nim_pow(b, e)` returns `b` raised to the `e`-th power under nim multiplication.
- `nim_inverse(b)` returns the multiplicative inverse of a nonzero `b`, so that
  `nim_product(b, nim_inverse(b)) == 1`.

Time Complexity:
- O(1) table construction on first use (a fixed 64 by 64 table).
- O(64^2) per call to `nim_product()`; O(64^2 log e) per call to `nim_pow()` and `nim_inverse()`.

Space Complexity:
- O(1): a fixed 64 by 64 table of 64-bit products.

*/

#include <cstdint>

namespace {

uint64_t bit_prod[64][64];

// Build the single-bit products 2^i (x) 2^j. Each entry depends only on entries with a smaller
// first index, so a single ascending pass fills the table.
const bool nim_product_init = [] {
  for (int i = 0; i < 64; i++) {
    for (int j = 0; j < 64; j++) {
      if ((i & j) == 0) {
        bit_prod[i][j] = uint64_t(1) << (i | j);  // Distinct Fermat powers multiply like 2^(i|j).
      } else {
        int a = (i & j) & -(i & j);  // Lowest Fermat power shared by both exponents.
        bit_prod[i][j] = bit_prod[i ^ a][j] ^ bit_prod[(i ^ a) | (a - 1)][(j ^ a) | (i & (a - 1))];
      }
    }
  }
  return true;
}();

}  // namespace

uint64_t nim_product(uint64_t x, uint64_t y) {
  uint64_t res = 0;
  for (int i = 0; i < 64 && (x >> i) != 0; i++) {
    if ((x >> i) & 1) {
      for (int j = 0; j < 64 && (y >> j) != 0; j++) {
        if ((y >> j) & 1) {
          res ^= bit_prod[i][j];
        }
      }
    }
  }
  return res;
}

uint64_t nim_pow(uint64_t b, uint64_t e) {
  uint64_t res = 1;
  for (; e > 0; e >>= 1) {
    if (e & 1) {
      res = nim_product(res, b);
    }
    b = nim_product(b, b);
  }
  return res;
}

uint64_t nim_inverse(uint64_t b) {
  // The multiplicative group of the size-2^64 field has order 2^64 - 1, so b^(2^64 - 2) = b^{-1}.
  return nim_pow(b, static_cast<uint64_t>(-2));
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Smallest nontrivial products: {0,1,2,3} form the field GF(4) under XOR and nim product.
  assert(nim_product(2, 2) == 3);
  assert(nim_product(2, 3) == 1);
  assert(nim_product(3, 3) == 2);
  assert(nim_product(1, 12345) == 12345);  // 1 is the multiplicative identity.
  assert(nim_product(0, 12345) == 0);

  // Field axioms on a range of values: commutativity, distributivity over XOR, and inverses.
  for (uint64_t a = 0; a < 64; a++) {
    for (uint64_t b = 0; b < 64; b++) {
      assert(nim_product(a, b) == nim_product(b, a));
      for (uint64_t c = 0; c < 8; c++) {
        assert(nim_product(a, b ^ c) == (nim_product(a, b) ^ nim_product(a, c)));
      }
    }
    if (a != 0) {
      assert(nim_product(a, nim_inverse(a)) == 1);
    }
  }
  assert(nim_product(0x1234567890abcdefULL, nim_inverse(0x1234567890abcdefULL)) == 1);
  return 0;
}
