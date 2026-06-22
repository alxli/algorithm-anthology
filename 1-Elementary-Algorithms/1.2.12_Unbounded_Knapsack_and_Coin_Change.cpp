/*

Solves two common unbounded knapsack variants, where each item may be used any number of times. In
the maximum-value version, each item has a weight and value, and the goal is to maximize total value
subject to a capacity limit. In the coin change version, each coin denomination may be used any
number of times, and the goal is to count unordered ways to form a target sum.

For unbounded maximum-value knapsack, capacities are processed in increasing order. Thus, when
computing `dp[w]`, every smaller-capacity state `dp[w - weight[i]]` is already final and may itself
contain item `i`, allowing unlimited reuse. For unordered coin change, coins are the outer loop so
each multiset of coins is counted once, independent of ordering.

- `unbounded_knapsack(weight, value, capacity)` returns a pair (`value`, `count`) containing the
  maximum value achievable with total weight at most `capacity` and `count[i]`, the number of copies
  selected of each item `i`.
- `count_coin_change(coins, target)` returns the number of unordered ways to make sum `target` using
  the given coin denominations.
- All capacities must be nonnegative integers. Weights and coin denominations should be positive to
  avoid infinitely many uses of a zero-weight item.

Time Complexity:
- O(n*W) for `unbounded_knapsack()`, where $n$ is the number of items and $W$ is `capacity`.
- O(c*t) for `count_coin_change(coins, target)`, where $c$ is the number of coin denominations and
  $t$ is `target`.

Space Complexity:
- O(W) auxiliary heap space for `unbounded_knapsack()`.
- O(t) auxiliary heap space for `count_coin_change(coins, target)`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> unbounded_knapsack(
    const std::vector<int> &weight, const std::vector<int64_t> &value, int capacity
) {
  int n = static_cast<int>(weight.size());
  assert(static_cast<int>(value.size()) == n && capacity >= 0);
  for (int w : weight) {
    assert(w > 0);
  }
  std::vector<int64_t> dp(capacity + 1, 0);
  std::vector<int> choice(capacity + 1, -1);
  for (int w = 1; w <= capacity; w++) {
    for (int i = 0; i < n; i++) {
      if (weight[i] <= w && dp[w] < dp[w - weight[i]] + value[i]) {
        dp[w] = dp[w - weight[i]] + value[i];
        choice[w] = i;
      }
    }
  }
  std::vector<int> count(n, 0);
  for (int w = capacity; choice[w] != -1; w -= weight[choice[w]]) {
    count[choice[w]]++;
  }
  return {dp[capacity], count};
}

int64_t count_coin_change(const std::vector<int> &coins, int target) {
  std::vector<int64_t> dp(target + 1, 0);
  dp[0] = 1;
  for (int coin : coins) {
    for (int sum = coin; sum <= target; sum++) {
      dp[sum] += dp[sum - coin];
    }
  }
  return dp[target];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> weight{3, 4, 5};
  vector<int64_t> value{4, 5, 7};
  auto [best, count] = unbounded_knapsack(weight, value, 10);
  assert(best == 14);
  assert((count == vector<int>{0, 0, 2}));

  vector<int> coins{1, 2, 5};
  assert(count_coin_change(coins, 5) == 4);  // 5, 2+2+1, 2+1+1+1, 1*5.
  return 0;
}
