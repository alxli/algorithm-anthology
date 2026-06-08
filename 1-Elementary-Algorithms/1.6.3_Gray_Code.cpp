/*

The binary reflected Gray code is an ordering of all $2^n$ bitmasks in which consecutive masks
differ in exactly one bit. It is useful whenever the cost of moving between adjacent states depends
on a single bit flip: enumerating subsets while incrementally maintaining a value, hardware where
only one bit may toggle at a time, and constructions like Hamiltonian paths on the hypercube.

The `k`-th Gray code is obtained from the ordinary binary value `k` by `k ^ (k >> 1)`, a bijection
on `[0, 2^n)`. Its inverse recovers the rank of a given Gray code, which tells how many flips into
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

#include <vector>

unsigned gray_code(unsigned k) {
  return k ^ (k >> 1);
}

unsigned inverse_gray_code(unsigned g) {
  for (unsigned shift = 1; (g >> shift) != 0; shift <<= 1) {
    g ^= g >> shift;
  }
  return g;
}

std::vector<unsigned> gray_sequence(int n) {
  std::vector<unsigned> res(1u << n);
  for (unsigned k = 0; k < res.size(); k++) {
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

  for (unsigned k = 0; k < 256; k++) {
    assert(inverse_gray_code(gray_code(k)) == k);
  }

  vector<unsigned> seq = gray_sequence(3);
  assert(seq.size() == 8);
  for (size_t i = 0; i < seq.size(); i++) {
    unsigned diff = seq[i] ^ seq[(i + 1) % seq.size()];
    assert(__builtin_popcount(diff) == 1);  // Cyclically, one bit changes per step.
  }
  return 0;
}
