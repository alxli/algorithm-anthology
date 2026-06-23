/*

An XOR basis (also called a linear basis over the field GF(2)) is the analog of a vector space basis
for integers under the bitwise XOR operation. Each integer is treated as a vector of bits, where XOR
plays the role of vector addition. Inserting a set of integers one at a time and keeping only those
that are linearly independent yields a basis whose XOR-combinations (subset XORs) span exactly the
same set of values as the original integers. From a basis of size $r$, exactly $2^r$ distinct values
are reachable, and queries such as the maximum attainable XOR become a simple greedy scan.

The basis is kept in reduced form: the $i$-th basis is either $0$ or a value whose highest set bit
is bit $i$, and no two basis elements share the same highest bit. Reducing a value means XORing it
against the basis elements from the highest bit down, eliminating each set bit that a basis element
covers.

- `insert(x)` adds `x` to the basis, returning `true` if `x` was linearly independent of the current
  basis (increasing its size), or `false` if `x` was already representable as a subset XOR.
- `contains(x)` returns whether `x` is representable as the XOR of some subset of the inserted
  values, i.e. whether it reduces to 0 against the basis.
- `max_xor(base)` returns the maximum value of `base` XORed with some subset XOR of the basis. With
  the default `base` of 0, this is the maximum subset XOR.
- `size()` returns the number of elements in the basis (its rank).

Time Complexity:
- O(b) per call to `insert()`, `contains()`, and `max_xor()`, where $b$ is the number of bits.
- O(1) per call to `size()`.

Space Complexity:
- O(b) for storage of the basis.
- O(1) auxiliary for all operations.

*/

#include <array>
#include <climits>
#include <cstdint>

class XorBasis {
  static const int BITS = sizeof(uint64_t) * CHAR_BIT;

  std::array<uint64_t, BITS> basis{};  // basis[i] != 0 has its highest set bit at bit i.
  int sz = 0;

 public:
  bool insert(uint64_t x) {
    for (int i = BITS - 1; i >= 0; i--) {
      if (!((x >> i) & 1)) {
        continue;
      }
      if (basis[i] == 0) {
        basis[i] = x;
        sz++;
        return true;
      }
      x ^= basis[i];
    }
    return false;
  }

  bool contains(uint64_t x) const {
    for (int i = BITS - 1; i >= 0; i--) {
      if (((x >> i) & 1) && basis[i] != 0) {
        x ^= basis[i];
      }
    }
    return x == 0;
  }

  uint64_t max_xor(uint64_t base = 0) const {
    for (int i = BITS - 1; i >= 0; i--) {
      if ((base ^ basis[i]) > base) {
        base ^= basis[i];
      }
    }
    return base;
  }

  int size() const { return sz; }
};

/*** Example Usage ***/

#include <cassert>

int main() {
  XorBasis b;
  assert(b.insert(1));   // 001
  assert(b.insert(2));   // 010
  assert(!b.insert(3));  // 011 = 1 ^ 2, already representable.
  assert(b.size() == 2);

  assert(b.contains(3));
  assert(b.contains(0));
  assert(!b.contains(4));

  assert(b.max_xor() == 3);   // Best subset XOR over {0, 1, 2, 3}.
  assert(b.max_xor(4) == 7);  // 4 ^ 3.
  return 0;
}
