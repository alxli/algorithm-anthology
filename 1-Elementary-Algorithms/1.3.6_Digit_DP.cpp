/*

Digit dynamic programming counts integers in a large bounded range whose decimal representations
satisfy a digit-based property. The concrete routine below counts integers with a specified digit
sum and provides the standard template to adapt for other finite-state properties.

To count valid integers in $[0, x]$, process the digits of $x$ from left to right. The state records
the current position, accumulated digit sum, and whether the chosen prefix still equals the prefix
of $x$. Once that "tight" condition is false, the remaining choices depend only on the position and
sum, so those states can be memoized. Subtracting the prefix counts for `hi` and `lo - 1` gives the
answer for the requested range.

Leading zeros do not affect a digit sum, so every integer can be treated as a zero-padded string of
the same length as the bound. For properties where leading zeros matter, add a `started` flag to the
state. More generally, the sum can be replaced by any small state with a transition for appending a
digit.

- `count_digit_sum(lo, hi, target)` returns the number of integers in the inclusive range
  $[`lo`, `hi`]$ whose decimal digits sum to `target`. The bounds and `target` must be nonnegative.

Time Complexity:
- O(d*s) per call, where $d$ is the number of digits in `hi` and $s$ is `target`.

Space Complexity:
- O(d*s) auxiliary heap space and O(d) call stack space.

*/

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

int64_t count_digit_sum(int64_t lo, int64_t hi, int target) {
  assert(0 <= lo && lo <= hi && target >= 0);
  auto count_up_to = [&](int64_t bound) {
    if (bound < 0) {
      return 0LL;
    }
    std::string digits = std::to_string(bound);
    int n = static_cast<int>(digits.size());
    if (target > 9 * n) {
      return 0LL;
    }
    std::vector<std::vector<int64_t>> memo(n, std::vector<int64_t>(target + 1, -1));
    auto rec = [&](auto &&rec, int pos, int sum, bool tight) -> int64_t {
      if (sum > target || sum + 9 * (n - pos) < target) {
        return 0;
      }
      if (pos == n) {
        return sum == target;
      }
      if (!tight && memo[pos][sum] != -1) {
        return memo[pos][sum];
      }
      int bound_digit = digits[pos] - '0';
      int limit = tight ? bound_digit : 9;
      int64_t res = 0;
      for (int digit = 0; digit <= limit; digit++) {
        res += rec(rec, pos + 1, sum + digit, tight && digit == bound_digit);
      }
      if (!tight) {
        memo[pos][sum] = res;
      }
      return res;
    };
    return rec(rec, 0, 0, true);
  };
  return count_up_to(hi) - count_up_to(lo - 1);
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(count_digit_sum(0, 20, 2) == 3);   // 2, 11, and 20.
  assert(count_digit_sum(10, 30, 3) == 3);  // 12, 21, and 30.
  assert(count_digit_sum(0, 99, 9) == 10);
  assert(count_digit_sum(0, 0, 0) == 1);
  assert(count_digit_sum(0, 999, 28) == 0);
  return 0;
}
