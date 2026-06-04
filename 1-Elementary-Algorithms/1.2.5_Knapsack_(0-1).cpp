/*

Solves the 0/1 knapsack problem: given items with nonnegative integer weights and values, choose
each item at most once to maximize total value without exceeding a capacity limit.

The one-dimensional dynamic programming array `dp[w]` stores the best value that can be achieved
with total weight at most `w` after processing some prefix of the items. Iterating weights in
decreasing order is what enforces the 0/1 constraint; it prevents the same item from being reused
during the current item update.

- `knapsack_01(weight, value, capacity)` returns the maximum value achievable with total weight at
  most `capacity`.
- `weight[i]` and `value[i]` are the weight and value of item `i`.
- All weights must be nonnegative integers. Items with weight `0` are allowed and can each be taken
  once.

Time Complexity:
- O(nW), where $n$ is the number of items and $W$ is `capacity`.

Space Complexity:
- O(W) auxiliary heap space.

*/

#include <algorithm>
#include <cassert>
#include <vector>

long long knapsack_01(
    const std::vector<int> &weight, const std::vector<long long> &value, int capacity
) {
  int n = weight.size();
  std::vector<long long> dp(capacity + 1, 0);
  for (int i = 0; i < n; i++) {
    for (int w = capacity; w >= weight[i]; w--) {
      dp[w] = std::max(dp[w], dp[w - weight[i]] + value[i]);
    }
  }
  return dp[capacity];
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> weight{3, 4, 5, 9};
  vector<long long> value{4, 5, 7, 10};

  assert(knapsack_01(weight, value, 8) == 11);   // Items of weight 3 and 5.
  assert(knapsack_01(weight, value, 10) == 12);  // Items of weight 4 and 5.
  return 0;
}
