/*

Integers double as fixed-size sets of bits, where bit `b` (counting from 0 at the least significant
end) is "present" when it is set to 1. Treating an integer as a bitmask makes set membership,
insertion, deletion, and iteration into single machine instructions, which is why bitmasks are the
backbone of subset dynamic programming and many low-level tricks. The helpers below operate on
`unsigned` masks; to use 64-bit masks, replace `unsigned` with `unsigned long long`, the literal
`1u` with `1ull`, and the `1u << 31` inside `clz()` with `1ull << 63`.

- `test_bit(x, b)` returns whether bit `b` of `x` is set.
- `set_bit(x, b)`, `clear_bit(x, b)`, and `toggle_bit(x, b)` return `x` with bit `b` respectively
  forced to 1, forced to 0, or flipped.
- `lowest_set_bit(x)` returns the value of the lowest set bit of `x` (a power of 2), or 0 if `x`
  is 0. `clear_lowest_set_bit(x)` returns `x` with its lowest set bit removed.
- `popcount(x)` returns the number of set bits, analogous to `__builtin_popcount()`.
- `ctz(x)` returns the number of trailing 0-bits (undefined for 0), analogous to `__builtin_ctz()`.
- `clz(x)` returns the number of leading 0-bits (undefined for 0), analgoous to `__builtin_clz()`.
- `is_power_of_two(x)` returns whether `x` has exactly one set bit.
- `floor_pow2(x)` returns the largest power of 2 that is $\leq$ `x` (for `x` $> 0$).
- `ceil_pow2(x)` returns the smallest power of 2 that is $\geq$ `x` (for `x` $> 0$).
- `for_each_set_bit(x, f)` calls `f(b)` once for each set bit position `b` of `x`, in increasing
  order.

Time Complexity:
- O(b) worst case per call to `popcount(x)`, `ctz(x)`, `clz(x)`, and `for_each_set_bit(x, f)`, where
  $b$ is the bit width of the mask (32 here).
- O(1) for all other operations.

Space Complexity:
- O(1) auxiliary for all operations.

*/

// clang-format off
bool test_bit(unsigned x, int b) {  return (x >> b) & 1u; }
unsigned set_bit(unsigned x, int b) { return x | (1u << b); }
unsigned clear_bit(unsigned x, int b) { return x & ~(1u << b); }
unsigned toggle_bit(unsigned x, int b) { return x ^ (1u << b); }
unsigned lowest_set_bit(unsigned x) { return x & (0u - x); }
unsigned clear_lowest_set_bit(unsigned x) { return x & (x - 1); }
// clang-format on

// popcount(), ctz(), and clz() are spelled out here for educational purposes.
// Production code should use the compiler intrinsics named in the comments below.
int popcount(unsigned x) {  // __builtin_popcount(x)
  int count = 0;
  for (; x != 0; x = clear_lowest_set_bit(x)) {
    count++;
  }
  return count;
}

int ctz(unsigned x) {  // __builtin_ctz(x); undefined when x == 0.
  int count = 0;
  for (; (x & 1u) == 0; x >>= 1) {
    count++;
  }
  return count;
}

int clz(unsigned x) {  // __builtin_clz(x); undefined when x == 0.
  int count = 0;
  for (unsigned mask = 1u << 31; (x & mask) == 0; mask >>= 1) {
    count++;
  }
  return count;
}

bool is_power_of_two(unsigned x) {
  return x != 0 && (x & (x - 1)) == 0;
}

unsigned floor_pow2(unsigned x) {
  return 1u << (31 - __builtin_clz(x));
}

unsigned ceil_pow2(unsigned x) {
  return is_power_of_two(x) ? x : 1u << (32 - __builtin_clz(x));
}

template<class Fn>
void for_each_set_bit(unsigned x, Fn f) {
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
  unsigned x = 0b101100;
  assert(test_bit(x, 2) == true);
  assert(test_bit(x, 0) == false);
  assert(set_bit(x, 0) == 0b101101u);
  assert(clear_bit(x, 2) == 0b101000u);
  assert(toggle_bit(x, 3) == 0b100100u);

  assert(lowest_set_bit(x) == 0b100u);
  assert(clear_lowest_set_bit(x) == 0b101000u);
  assert(popcount(x) == 3);
  assert(ctz(x) == 2);

  assert(is_power_of_two(16) == true);
  assert(is_power_of_two(24) == false);
  assert(floor_pow2(20) == 16u);
  assert(ceil_pow2(20) == 32u);
  assert(ceil_pow2(16) == 16u);

  vector<int> bits;
  for_each_set_bit(x, [&](int b) { bits.push_back(b); });
  assert((bits == vector<int>{2, 3, 5}));
  return 0;
}
