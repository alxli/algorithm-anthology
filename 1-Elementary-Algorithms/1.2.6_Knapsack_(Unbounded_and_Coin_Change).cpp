/*

Solves two common unbounded knapsack variants, where each item may be used any number of times. In
the maximum-value version, each item has a weight and value, and the goal is to maximize total value
subject to a capacity limit. In the coin change version, each coin denomination may be used any
number of times, and the goal is to count unordered ways to form a target sum.

For unbounded maximum-value knapsack, weights are iterated in increasing order so the current item
may be reused. For unordered coin change, coins are the outer loop so each multiset of coins is
counted once, independent of ordering.

- `unbounded_knapsack(weight, value, capacity)` returns the maximum value achievable with total
  weight at most `capacity`.
- `count_coin_change(coins, target)` returns the number of unordered ways to make sum `target` using
  the given coin denominations.
- All capacities must be nonnegative integers. Weights and coin denominations should be positive to
  avoid infinitely many uses of a zero-weight item.

Time Complexity:
- O(nW) for `unbounded_knapsack(weight, value, capacity)`, where $n$ is the number of items and $W$
  is `capacity`.
- O(ct) for `count_coin_change(coins, target)`, where $c$ is the number of coin denominations and
  $t$ is `target`.

Space Complexity:
- O(W) auxiliary heap space for `unbounded_knapsack(weight, value, capacity)`.
- O(t) auxiliary heap space for `count_coin_change(coins, target)`.

*/

#include <algorithm>
#include <vector>

long long unbounded_knapsack(
    const std::vector<int> &weight, const std::vector<long long> &value, int capacity
) {
  int n = weight.size();
  std::vector<long long> dp(capacity + 1, 0);
  for (int i = 0; i < n; i++) {
    for (int w = weight[i]; w <= capacity; w++) {
      dp[w] = std::max(dp[w], dp[w - weight[i]] + value[i]);
    }
  }
  return dp[capacity];
}

long long count_coin_change(const std::vector<int> &coins, int target) {
  std::vector<long long> dp(target + 1, 0);
  dp[0] = 1;
  for (int i = 0; i < (int)coins.size(); i++) {
    for (int sum = coins[i]; sum <= target; sum++) {
      dp[sum] += dp[sum - coins[i]];
    }
  }
  return dp[target];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int w_raw[] = {3, 4, 5};
  long long v_raw[] = {4, 5, 7};
  vector<int> weight(w_raw, w_raw + 3);
  vector<long long> value(v_raw, v_raw + 3);
  assert(unbounded_knapsack(weight, value, 10) == 14);  // Two weight-5 items.

  int c_raw[] = {1, 2, 5};
  vector<int> coins(c_raw, c_raw + 3);
  assert(count_coin_change(coins, 5) == 4);  // 5, 2+2+1, 2+1+1+1, 1*5.
  return 0;
}
