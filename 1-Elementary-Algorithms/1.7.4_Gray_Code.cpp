/*

The binary reflected Gray code is an ordering of all $2^n$ bitmasks in which consecutive masks
differ in exactly one bit. It is useful whenever the cost of moving between adjacent states depends
on a single bit flip: enumerating subsets while incrementally maintaining a value, hardware where
only one bit may toggle at a time, and constructions like Hamiltonian paths on the hypercube.

The Gray code at 0-based rank `k` is obtained from the ordinary binary value `k` by `k ^ (k >> 1)`.
Incrementing `k` flips one binary bit and every lower bit, but XORing each bit with its higher
neighbor cancels all but the highest of those changes, so consecutive Gray codes differ in exactly
one bit. The map is a bijection: each binary bit is the prefix XOR of the Gray bits at and above it.
The inverse loop computes those prefix XORs in doubling steps to recover the original rank.

- `gray_code(k)` returns the mask at 0-based rank `k` in Gray code order.
- `inverse_gray_code(g)` returns the 0-based rank `k` such that `gray_code(k)` equals `g`.
- `gray_sequence(n)` returns all $2^n$ masks in Gray code order; consecutive entries (and the last
  with the first when `n > 0`) differ in exactly one bit, where $0 \leq `n` < `MASK_BITS`$.

Time Complexity:
- O(1) per call to `gray_code()`.
- O(log b) per call to `inverse_gray_code()`, where $b$ is `MASK_BITS`.
- O(2^n) per call to `gray_sequence(n)`.

Space Complexity:
- O(1) auxiliary for `gray_code()` and `inverse_gray_code()`.
- O(1) auxiliary and O(2^n) for the returned sequence from `gray_sequence(n)`.

*/

#include <cassert>
#include <climits>
#include <cstdint>
#include <vector>

using Mask = uint32_t;
const int MASK_BITS = sizeof(Mask) * CHAR_BIT;

Mask gray_code(Mask k) {
  return k ^ (k >> 1);
}

Mask inverse_gray_code(Mask g) {
  for (int shift = 1; shift < MASK_BITS && (g >> shift) != 0; shift <<= 1) {
    g ^= g >> shift;
  }
  return g;
}

std::vector<Mask> gray_sequence(int n) {
  assert(0 <= n && n < MASK_BITS);
  std::vector<Mask> res(Mask{1} << n);
  for (Mask k = 0; k < static_cast<Mask>(res.size()); k++) {
    res[k] = gray_code(k);
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(gray_code(0) == 0b000U);
  assert(gray_code(1) == 0b001U);
  assert(gray_code(2) == 0b011U);
  assert(gray_code(3) == 0b010U);

  for (Mask k = 0; k < 256; k++) {
    assert(inverse_gray_code(gray_code(k)) == k);
  }
  assert(inverse_gray_code(gray_code(0xdeadbeefU)) == 0xdeadbeefU);

  vector<Mask> seq = gray_sequence(3);
  assert((seq == vector<Mask>{0b000, 0b001, 0b011, 0b010, 0b110, 0b111, 0b101, 0b100}));
  for (int i = 0; i < static_cast<int>(seq.size()); i++) {
    Mask diff = seq[i] ^ seq[(i + 1) % seq.size()];
    assert((diff & (diff - 1)) == 0);  // Cyclically, one bit changes per step.
  }
  return 0;
}
