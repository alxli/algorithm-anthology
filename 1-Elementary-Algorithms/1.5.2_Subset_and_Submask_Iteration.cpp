/*

Many bitmask algorithms must visit related masks in a structured order: every submask of a fixed
mask, every mask with a fixed number of set bits, or every mask while accumulating contributions
from its submasks. Each pattern has a short, well-known idiom.

The classic submask loop `for (int s = m; s > 0; s = (s - 1) & m)` walks every nonzero submask of
`m` in decreasing order; subtracting 1 borrows through the zero bits of `m`, and the `& m` masks
them back off. Summed over all masks `m` of `n` bits, the total work is
$\sum_k \binom{n}{k} 2^k = 3^n$, since each of the `n` bit positions is independently absent,
present in `m` only, or present in both `m` and `s`.

Gosper's hack advances to the next mask with the same popcount. It isolates the lowest set bit and
adds it to move the lowest movable 1 one position left, clearing the block of 1-bits beneath it. The
remaining expression counts those cleared bits and packs them into the least-significant positions,
producing the smallest larger integer with the same number of set bits.

- `submasks(m)` returns all submasks of `m`, including `m` itself and 0, in decreasing order.
- `next_popcount(x)` returns the smallest integer greater than `x` with the same number of set bits
  (Gosper's hack), for `x > 0`.
- `masks_with_popcount(n, k)` returns all `k`-bit subsets of an `n`-bit universe as masks, in
  increasing order.
- `subset_sum_transform(f)` overwrites `f` (indexed by mask over `n` bits) so that `f[m]` becomes
  the sum of the original `f[s]` over all submasks `s` of `m`. This "sum over subsets" (SOS) DP is
  the bitmask analog of a prefix sum, accumulating one bit dimension at a time.

Time Complexity:
- O(2^p) per call to `submasks(m)` where $p$ = `popcount(m)`.
- O(1) per call to `next_popcount(x)`.
- O(\binom{n}{k}) per call to `masks_with_popcount(n, k)`.
- O(n*2^n) per call to `subset_sum_transform(f)`, where `f` has $2^n$ entries.

Space Complexity:
- O(1) auxiliary for `next_popcount(x)` and `subset_sum_transform(f)`.
- O(s) auxiliary for `submasks(m)` and `masks_with_popcount(n, k)`, where $s$ is the result size.

*/

#include <cassert>
#include <cstdint>
#include <vector>

std::vector<uint32_t> submasks(uint32_t m) {
  std::vector<uint32_t> res;
  uint32_t s = m;
  while (true) {
    res.push_back(s);
    if (s == 0) {
      break;
    }
    s = (s - 1) & m;
  }
  return res;
}

uint32_t next_popcount(uint32_t x) {
  uint32_t c = x & (0u - x), r = x + c;
  return r | (((x ^ r) >> 2) / c);
}

std::vector<uint32_t> masks_with_popcount(int n, int k) {
  std::vector<uint32_t> res;
  if (k == 0) {
    return {0u};
  }
  uint32_t limit = 1u << n;
  for (uint32_t x = (1u << k) - 1; x < limit; x = next_popcount(x)) {
    res.push_back(x);
  }
  return res;
}

template<typename T>
void subset_sum_transform(std::vector<T> &f) {
  assert(!f.empty() && (f.size() & (f.size() - 1)) == 0);
  int n = __builtin_ctz(f.size());  // f.size() must be a power of two, 2^n.
  for (int b = 0; b < n; b++) {
    for (uint32_t m = 0; m < static_cast<uint32_t>(f.size()); m++) {
      if (m & (1u << b)) {
        f[m] += f[m ^ (1u << b)];
      }
    }
  }
}

/*** Example Usage ***/

using namespace std;

int main() {
  assert((submasks(0b101) == vector<uint32_t>{0b101, 0b100, 0b001, 0b000}));

  assert(next_popcount(0b0110) == 0b1001u);
  assert((
      masks_with_popcount(4, 2) == vector<uint32_t>{0b0011, 0b0101, 0b0110, 0b1001, 0b1010, 0b1100}
  ));

  // f[m] = 1 for every mask; after the transform f[m] counts submasks of m = 2^popcount(m).
  vector<int> f(1 << 3, 1);
  subset_sum_transform(f);
  assert(f[0b000] == 1);
  assert(f[0b101] == 4);
  assert(f[0b111] == 8);
  return 0;
}
