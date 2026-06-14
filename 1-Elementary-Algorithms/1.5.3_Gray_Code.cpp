/*

The binary reflected Gray code is an ordering of all $2^n$ bitmasks in which consecutive masks
differ in exactly one bit. It is useful whenever the cost of moving between adjacent states depends
on a single bit flip: enumerating subsets while incrementally maintaining a value, hardware where
only one bit may toggle at a time, and constructions like Hamiltonian paths on the hypercube.

The `k`-th Gray code is obtained from the ordinary binary value `k` by `k ^ (k >> 1)`, a bijection
on $[0, 2^n)$. Its inverse recovers the rank of a given Gray code, which tells how many flips into
the sequence a mask sits.

- `gray_code(k)` returns the `k`-th mask in Gray code order.
- `inverse_gray_code(g)` returns the rank `k` such that `gray_code(k) == g`.
- `gray_sequence(n)` returns all $2^n$ masks in Gray code order; consecutive entries (and the last
  with the first) differ in exactly one bit.

Time Complexity:
- O(1) per call to `gray_code(k)`.
- O(log g) per call to `inverse_gray_code(g)`.
- O(2^n) per call to `gray_sequence(n)`.

Space Complexity:
- O(1) auxiliary for `gray_code(k)` and `inverse_gray_code(g)`.
- O(2^n) auxiliary for `gray_sequence(n)`.

*/

#include <cstdint>
#include <vector>

uint32_t gray_code(uint32_t k) {
  return k ^ (k >> 1);
}

uint32_t inverse_gray_code(uint32_t g) {
  for (uint32_t shift = 1; (g >> shift) != 0; shift <<= 1) {
    g ^= g >> shift;
  }
  return g;
}

std::vector<uint32_t> gray_sequence(int n) {
  std::vector<uint32_t> res(1u << n);
  for (uint32_t k = 0; k < static_cast<uint32_t>(res.size()); k++) {
    res[k] = gray_code(k);
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(gray_code(0) == 0b000u);
  assert(gray_code(1) == 0b001u);
  assert(gray_code(2) == 0b011u);
  assert(gray_code(3) == 0b010u);

  for (uint32_t k = 0; k < 256; k++) {
    assert(inverse_gray_code(gray_code(k)) == k);
  }

  vector<uint32_t> seq = gray_sequence(3);
  assert(seq.size() == 8);
  for (int i = 0; i < static_cast<int>(seq.size()); i++) {
    uint32_t diff = seq[i] ^ seq[(i + 1) % seq.size()];
    assert(__builtin_popcount(diff) == 1);  // Cyclically, one bit changes per step.
  }
  return 0;
}
