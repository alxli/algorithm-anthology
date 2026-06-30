/*

Counts integers up to a decimal bound using digit dynamic programming. A digit DP scans the bound
from most significant digit to least significant digit while tracking whether the prefix is still
"tight" to the bound. If it is tight, the next digit cannot exceed the corresponding bound digit;
otherwise, any digit from $0$ to $9$ is allowed.

The `started` flag separates leading zeroes from digits of the represented number, so properties
can choose whether the number $0$ should count. The example helper below counts numbers whose digit
sum is exactly `target_sum`, treating the number $0$ as having digit sum $0$.

- `count_with_digit_sum(bound, target_sum)` returns the number of integers $x \in [0, `bound`]$ such
  that the decimal digit sum of $x$ is exactly `target_sum`.

Time Complexity:
- O(d*s*10) per call, where $d$ is the number of decimal digits in `bound` and $s$ is `target_sum`.

Space Complexity:
- O(s) auxiliary per call.

*/

#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

int64_t count_with_digit_sum(int64_t bound, int target_sum) {
  assert(bound >= 0 && target_sum >= 0);
  std::string digits = std::to_string(bound);
  std::vector<std::vector<std::vector<int64_t>>> dp(
      target_sum + 1, std::vector<std::vector<int64_t>>(2, std::vector<int64_t>(2, 0))
  );
  dp[0][1][0] = 1;
  for (char ch : digits) {
    std::vector<std::vector<std::vector<int64_t>>> next(
        target_sum + 1, std::vector<std::vector<int64_t>>(2, std::vector<int64_t>(2, 0))
    );
    for (int sum = 0; sum <= target_sum; sum++) {
      for (int tight = 0; tight <= 1; tight++) {
        for (int started = 0; started <= 1; started++) {
          int64_t ways = dp[sum][tight][started];
          if (ways == 0) {
            continue;
          }
          int limit = tight ? ch - '0' : 9;
          for (int digit = 0; digit <= limit; digit++) {
            int next_started = started || digit != 0;
            int next_sum = sum + (next_started ? digit : 0);
            if (next_sum <= target_sum) {
              next[next_sum][tight && digit == limit][next_started] += ways;
            }
          }
        }
      }
    }
    dp.swap(next);
  }
  return dp[target_sum][0][0] + dp[target_sum][0][1] + dp[target_sum][1][0] + dp[target_sum][1][1];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(count_with_digit_sum(20, 2) == 3);   // 2, 11, 20.
  assert(count_with_digit_sum(99, 9) == 10);  // 9, 18, ..., 90.
  assert(count_with_digit_sum(0, 0) == 1);
  assert(count_with_digit_sum(0, 1) == 0);
  return 0;
}
