/*

Solves unbounded knapsack and two related coin change problems, where each item or coin may be used
any number of times. In the maximum-value version, each item has a weight and value, and the goal is
to maximize total value subject to a capacity limit. In coin change variants, the goal is either to
count unordered ways to form a target sum or to minimize the number of coins used.

For unbounded maximum-value knapsack, capacities are processed in increasing order. Thus, when
computing `dp[w]`, every smaller-capacity state `dp[w - weight[i]]` is already final and may itself
contain item `i`, allowing unlimited reuse. For unordered coin change, coins are the outer loop so
each multiset of coins is counted once, independent of ordering. For minimum coins, each target sum
chooses the best previous sum `sum - coin`, and the predecessor coin reconstructs one optimal
multiset.

- `unbounded_knapsack(weight, value, capacity)` returns a pair (`best_value`, `count`) containing
  the maximum value achievable with total weight at most `capacity` and `count[i]`, the number of
  copies selected of each item `i`.
- `count_coin_change(coins, target)` returns the number of unordered ways to make sum `target` using
  the given coin denominations.
- `min_coin_change(coins, target)` returns a pair (`count`, `used`), where `count` is the minimum
  number of coins needed to make sum `target`, and `used[i]` is the number of copies of `coins[i]`
  in one optimal multiset. If `target` cannot be formed, `count` is $-1$ and `used` is empty.

`capacity` and `target` must be nonnegative integers. Weights and coin denominations must be
positive. Coin denominations must also be distinct when counting ways; duplicate denominations are
treated as different coin types and overcount identical multisets.

Time Complexity:
- O(n*W) for `unbounded_knapsack()`, where $n$ is the number of items and $W$ is `capacity`.
- O(c*t) for `count_coin_change(coins, target)`, where $c$ is the number of coin denominations and
  $t$ is `target`.
- O(c*t) for `min_coin_change(coins, target)`.

Space Complexity:
- O(W) auxiliary and O(n) for the returned `count` from `unbounded_knapsack()`.
- O(t) auxiliary for `count_coin_change(coins, target)`.
- O(t) auxiliary and O(c) for the returned `used` from `min_coin_change(coins, target)`.

*/

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
      if (weight[i] <= w) {
        int64_t candidate = dp[w - weight[i]] + value[i];  // Overflow warning.
        if (dp[w] < candidate) {
          dp[w] = candidate;
          choice[w] = i;
        }
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
  assert(target >= 0);
  std::vector<int64_t> dp(target + 1, 0);
  dp[0] = 1;
  for (int coin : coins) {
    assert(coin > 0);
    for (int sum = coin; sum <= target; sum++) {
      dp[sum] += dp[sum - coin];  // Overflow warning.
    }
  }
  return dp[target];
}

std::pair<int, std::vector<int>> min_coin_change(const std::vector<int> &coins, int target) {
  assert(target >= 0);
  for (int coin : coins) {
    assert(coin > 0);
  }
  int inf = target + 1;
  std::vector<int> dp(target + 1, inf), choice(target + 1, -1);
  dp[0] = 0;
  for (int sum = 1; sum <= target; sum++) {
    for (int i = 0; i < static_cast<int>(coins.size()); i++) {
      if (coins[i] <= sum && dp[sum] > dp[sum - coins[i]] + 1) {
        dp[sum] = dp[sum - coins[i]] + 1;
        choice[sum] = i;
      }
    }
  }
  if (dp[target] == inf) {
    return {-1, {}};
  }
  std::vector<int> used(coins.size(), 0);
  for (int sum = target; sum > 0; sum -= coins[choice[sum]]) {
    used[choice[sum]]++;
  }
  return {dp[target], used};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> weight{3, 4, 5};
  vector<int64_t> value{4, 5, 7};
  auto [best_value, count] = unbounded_knapsack(weight, value, 10);
  assert(best_value == 14);
  assert((count == vector<int>{0, 0, 2}));

  vector<int> coins{1, 2, 5};
  assert(count_coin_change(coins, 5) == 4);  // 5, 2+2+1, 2+1+1+1, 1*5.

  auto [coin_count, used] = min_coin_change(coins, 11);
  assert(coin_count == 3);
  assert((used == vector<int>{1, 0, 2}));

  auto [impossible, empty_used] = min_coin_change(vector<int>{4, 7}, 6);
  assert(impossible == -1 && empty_used.empty());
  return 0;
}
