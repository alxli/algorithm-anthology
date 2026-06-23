/*

Integers are all fixed-size sets of bits, where the $i$-th bit (counting from the least significant
bit where $i = 0$) is "present" when set to $1$. Treating an `int` as a bitmask makes set
membership, insertion, deletion, and iteration into single machine instructions, which is why
bitmasks are the backbone of subset dynamic programming and many low-level tricks. The helpers
below operate on `uint32_t` masks; to use 64-bit masks, replace `uint32_t` with `uint64_t`, the
literal `1u` with `1ull`, and the `1u << 31` inside `clz()` with `1ull << 63`. The loop
implementations below exist for education only; production code should prefer the GCC built-ins
named in the inline comments whenever available, or on C++20 systems the `<bit>` header, whose
generic `constexpr` equivalents (also named in the comments) are additionally well-defined at $0$.

- `test_bit(x, i)` returns whether the `i`-th bit of `x` is set.
- `set_bit(x, i)`, `clear_bit(x, i)`, and `toggle_bit(x, i)` return `x` with the `i`-th respectively
  forced to $1$, forced to $0$, or flipped.
- `lowest_set_bit(x)` returns the value of the lowest set bit of `x` (a power of two), or $0$ if `x`
  is $0$. `clear_lowest_set_bit(x)` returns `x` with its lowest set bit removed.
- `popcount(x)` returns the number of set bits, analogous to `__builtin_popcount()` and C++20's
  `std::popcount()`.
- `parity(x)` returns $1$ if the number of set bits is odd and $0$ otherwise, analogous to
  `__builtin_parity()`.
- `ctz(x)` returns the number of trailing 0-bits (undefined for $0$), analogous to `__builtin_ctz()`
  and C++20's `std::countr_zero()`.
- `ffs(x)` returns one plus the position of the lowest 1-bit of `x`, or $0$ if `x` is $0$,
  analogous to `__builtin_ffs()`.
- `clz(x)` returns the number of leading 0-bits (undefined for $0$), analogous to `__builtin_clz()`
  and C++20's `std::countl_zero()`.
- `is_pow2(x)` returns whether `x` has exactly one set bit, analogous to C++20's
  `std::has_single_bit()`.
- `floor_pow2(x)` returns the largest power of two that is $\leq$ `x` (for `x` $> 0$), analogous to
  C++20's `std::bit_floor()`.
- `ceil_pow2(x)` returns the smallest power of two that is $\geq$ `x` (for `x` $> 0$), analogous to
  C++20's `std::bit_ceil()`.
- `for_each_set_bit(x, f)` calls `f(i)` once for each set bit position `i` of `x`, in increasing
  order.

Time Complexity:
- O(b) worst case per call to `popcount(x)`, `parity(x)`, `ctz(x)`, `ffs(x)`, `clz(x)`, and
  `for_each_set_bit(x, f)`, where $b$ is the bit width of the mask (32 here).
- O(1) for all other operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

#include <cstdint>

// clang-format off
bool test_bit(uint32_t x, int i) {  return (x >> i) & 1u; }
uint32_t set_bit(uint32_t x, int i) { return x | (1u << i); }
uint32_t clear_bit(uint32_t x, int i) { return x & ~(1u << i); }
uint32_t toggle_bit(uint32_t x, int i) { return x ^ (1u << i); }
uint32_t lowest_set_bit(uint32_t x) { return x & (0u - x); }
uint32_t clear_lowest_set_bit(uint32_t x) { return x & (x - 1); }
// clang-format on

// popcount(), parity(), ctz(), ffs(), and clz() are spelled out here for educational purposes.
// Production code should use the compiler intrinsics named in the comments below.

int popcount(uint32_t x) {  // __builtin_popcount(x); std::popcount(x) in C++20.
  int count = 0;
  for (; x != 0; x = clear_lowest_set_bit(x)) {
    count++;
  }
  return count;
}

int parity(uint32_t x) {  // __builtin_parity(x)
  return popcount(x) & 1;
}

int ctz(uint32_t x) {  // __builtin_ctz(x); undefined when x == 0. std::countr_zero(x) in C++20.
  int count = 0;
  for (; (x & 1u) == 0; x >>= 1) {
    count++;
  }
  return count;
}

int ffs(uint32_t x) {  // __builtin_ffs(x); returns 0 when x == 0.
  return x == 0 ? 0 : ctz(x) + 1;
}

int clz(uint32_t x) {  // __builtin_clz(x); undefined when x == 0. std::countl_zero(x) in C++20.
  int count = 0;
  for (uint32_t mask = 1u << 31; (x & mask) == 0; mask >>= 1) {
    count++;
  }
  return count;
}

bool is_pow2(uint32_t x) {  // std::has_single_bit(x) in C++20.
  return x != 0 && (x & (x - 1)) == 0;
}

uint32_t floor_pow2(uint32_t x) {  // std::bit_floor(x) in C++20.
  return 1u << (31 - __builtin_clz(x));
}

uint32_t ceil_pow2(uint32_t x) {  // std::bit_ceil(x) in C++20.
  return is_pow2(x) ? x : 1u << (32 - __builtin_clz(x));
}

template<typename Fn>
void for_each_set_bit(uint32_t x, Fn f) {
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
  uint32_t x = 0b101100;
  assert(test_bit(x, 2) == true);
  assert(test_bit(x, 0) == false);
  assert(set_bit(x, 0) == 0b101101u);
  assert(clear_bit(x, 2) == 0b101000u);
  assert(toggle_bit(x, 3) == 0b100100u);

  assert(lowest_set_bit(x) == 0b100u);
  assert(clear_lowest_set_bit(x) == 0b101000u);
  assert(popcount(x) == 3);
  assert(parity(x) == 1);
  assert(parity(0b101101u) == 0);
  assert(ctz(x) == 2);
  assert(ffs(x) == 3);
  assert(ffs(0) == 0);
  assert(clz(x) == 26);

  assert(is_pow2(16) == true);
  assert(is_pow2(24) == false);
  assert(floor_pow2(20) == 16u);
  assert(ceil_pow2(20) == 32u);
  assert(ceil_pow2(16) == 16u);

  vector<int> bits;
  for_each_set_bit(x, [&](int b) { bits.push_back(b); });
  assert((bits == vector<int>{2, 3, 5}));
  return 0;
}
