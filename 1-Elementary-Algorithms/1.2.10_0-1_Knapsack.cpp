/*

Solves the 0-1 knapsack problem: given items with nonnegative integer weights and values, choose
each item at most once to maximize total value without exceeding a capacity limit.

The one-dimensional dynamic programming array `dp[w]` stores the best value that can be achieved
with total weight at most `w` after processing some prefix of the items. Iterating weights in
decreasing order is what enforces the 0-1 constraint; it prevents the same item from being reused
during the current item update.

- `knapsack_01(weight, value, capacity)` returns a pair (`value`, `items`) containing the maximum
  value and the selected item indices in increasing order, where item `i` has weight `weight[i]` and
  value `value[i]`. All weights must be nonnegative integers. Items with weight 0 are allowed and
  can each be taken once. The take/skip decision for every item and capacity is stored to
  reconstruct the optimal subset.

Time Complexity:
- O(n*W) per call, where $n$ is the number of items and $W$ is `capacity`.

Space Complexity:
- O(n*W) auxiliary heap space.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> knapsack_01(
    const std::vector<int> &weight, const std::vector<int64_t> &value, int capacity
) {
  int n = static_cast<int>(weight.size());
  assert(static_cast<int>(value.size()) == n && capacity >= 0);
  std::vector<int64_t> dp(capacity + 1, 0);
  std::vector<std::vector<char>> take(n, std::vector<char>(capacity + 1, false));
  for (int i = 0; i < n; i++) {
    assert(weight[i] >= 0);
    for (int w = capacity; w >= weight[i]; w--) {
      int64_t candidate = dp[w - weight[i]] + value[i];
      if (dp[w] < candidate) {
        dp[w] = candidate;
        take[i][w] = true;
      }
    }
  }
  std::vector<int> items;
  for (int i = n - 1, w = capacity; i >= 0; i--) {
    if (take[i][w]) {
      items.push_back(i);
      w -= weight[i];
    }
  }
  std::reverse(items.begin(), items.end());
  return {dp[capacity], items};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> weight{3, 4, 5, 9};
  vector<int64_t> value{4, 5, 7, 10};

  auto [best, items] = knapsack_01(weight, value, 8);
  assert(best == 11);
  assert((items == vector<int>{0, 2}));
  return 0;
}
