/*

Integers are all fixed-size sets of bits, where the $i$-th bit (counting from the least significant
bit where $i = 0$) is "present" when set to $1$. Treating an `int` as a bitmask makes set
membership, insertion, deletion, and iteration into single machine instructions, which is why
bitmasks are the backbone of subset dynamic programming and many low-level tricks. The helpers below
operate on `Mask`, which is `uint32_t` by default; change its alias to `uint64_t` to use 64-bit
masks. The loop implementations below exist for education only; production code should prefer the
matching GCC built-ins when available, or on C++20 systems the generic `constexpr` equivalents from
the `<bit>` header, which are additionally well-defined at $0$.

- `test_bit(x, i)` returns whether the `i`-th bit of `x` is set, where $0 \leq `i` < `MASK_BITS`$.
- `set_bit(x, i)`, `clear_bit(x, i)`, and `toggle_bit(x, i)` return `x` with the `i`-th respectively
  forced to $1$, forced to $0$, or flipped, where $0 \leq `i` < `MASK_BITS`$.
- `lowest_set_bit(x)` returns the value of the lowest set bit of `x` (a power of two), or $0$ if `x`
  is $0$. `clear_lowest_set_bit(x)` returns `x` with its lowest set bit removed.
- `popcount(x)` returns the number of set bits, analogous to C++20's `std::popcount()`.
- `parity(x)` returns $1$ if the number of set bits is odd and $0$ otherwise.
- `ctz(x)` returns the number of trailing 0-bits for `x > 0`, analogous to C++20's
  `std::countr_zero()`.
- `ffs(x)` returns one plus the position of the lowest 1-bit of `x`, or $0$ if `x` is $0$.
- `clz(x)` returns the number of leading 0-bits for `x > 0`, analogous to C++20's
  `std::countl_zero()`.
- `is_pow2(x)` returns whether `x` has exactly one set bit, analogous to C++20's
  `std::has_single_bit()`.
- `floor_pow2(x)` returns the largest power of two that is $\leq$ `x` (for `x` $> 0$), analogous to
  C++20's `std::bit_floor()`.
- `ceil_pow2(x)` returns the smallest power of two that is $\geq$ `x`, for
  $0 < `x` \leq 2^{b - 1}$ where $b$ is `MASK_BITS`, analogous to C++20's `std::bit_ceil()`.
- `for_each_set_bit(x, f)` calls `f(i)` once for each set bit position `i` of `x`, in increasing
  order.

Time Complexity:
- O(b) worst case per call to `popcount()`, `parity()`, `ctz()`, `ffs()`, `clz()`, and
  `for_each_set_bit()`, where $b$ is `MASK_BITS`.
- O(1) per call to every other function.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cassert>
#include <climits>
#include <cstdint>

using Mask = uint32_t;
const int MASK_BITS = sizeof(Mask) * CHAR_BIT;

// clang-format off
bool test_bit(Mask x, int i) { return (x >> i) & 1; }
Mask set_bit(Mask x, int i) { return x | (Mask{1} << i); }
Mask clear_bit(Mask x, int i) { return x & ~(Mask{1} << i); }
Mask toggle_bit(Mask x, int i) { return x ^ (Mask{1} << i); }
Mask lowest_set_bit(Mask x) { return x & -x; }
Mask clear_lowest_set_bit(Mask x) { return x & (x - 1); }
// clang-format on

// popcount(), parity(), ctz(), ffs(), and clz() are spelled out here for educational purposes.

int popcount(Mask x) {  // std::popcount(x) in C++20.
  int count = 0;
  for (; x != 0; x = clear_lowest_set_bit(x)) {
    count++;
  }
  return count;
}

int parity(Mask x) {
  return popcount(x) & 1;
}

int ctz(Mask x) {  // std::countr_zero(x) in C++20.
  assert(x != 0);
  int count = 0;
  for (; (x & 1) == 0; x >>= 1) {
    count++;
  }
  return count;
}

int ffs(Mask x) {
  return x == 0 ? 0 : ctz(x) + 1;
}

int clz(Mask x) {  // std::countl_zero(x) in C++20.
  assert(x != 0);
  int count = 0;
  for (Mask mask = Mask{1} << (MASK_BITS - 1); (x & mask) == 0; mask >>= 1) {
    count++;
  }
  return count;
}

bool is_pow2(Mask x) {  // std::has_single_bit(x) in C++20.
  return x != 0 && (x & (x - 1)) == 0;
}

Mask floor_pow2(Mask x) {  // std::bit_floor(x) in C++20.
  assert(x != 0);
  return Mask{1} << (MASK_BITS - 1 - clz(x));
}

Mask ceil_pow2(Mask x) {  // std::bit_ceil(x) in C++20.
  assert(0 < x && x <= (Mask{1} << (MASK_BITS - 1)));
  return is_pow2(x) ? x : Mask{1} << (MASK_BITS - clz(x));
}

template<typename Fn>
void for_each_set_bit(Mask x, Fn f) {
  while (x != 0) {
    f(ctz(x));
    x = clear_lowest_set_bit(x);
  }
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  Mask x = 0b101100;
  assert(test_bit(x, 2) == true);
  assert(test_bit(x, 0) == false);
  assert(set_bit(x, 0) == 0b101101u);
  assert(clear_bit(x, 2) == 0b101000u);
  assert(toggle_bit(x, 3) == 0b100100u);

  assert(lowest_set_bit(x) == 0b100u);
  assert(clear_lowest_set_bit(x) == 0b101000u);
  assert(lowest_set_bit(0) == 0u);
  assert(clear_lowest_set_bit(0) == 0u);
  assert(popcount(x) == 3);
  assert(popcount(0) == 0);
  assert(parity(x) == 1);
  assert(parity(0b101101u) == 0);
  assert(parity(0) == 0);
  assert(ctz(x) == 2);
  assert(ffs(x) == 3);
  assert(ffs(0) == 0);
  assert(clz(x) == MASK_BITS - 6);

  assert(is_pow2(16) == true);
  assert(is_pow2(24) == false);
  assert(floor_pow2(20) == 16u);
  assert(ceil_pow2(20) == 32u);
  assert(ceil_pow2(16) == 16u);
  assert(ceil_pow2((Mask{1} << (MASK_BITS - 1)) - 1) == (Mask{1} << (MASK_BITS - 1)));

  vector<int> bits;
  for_each_set_bit(x, [&](int b) { bits.push_back(b); });
  assert((bits == vector<int>{2, 3, 5}));
  return 0;
}
