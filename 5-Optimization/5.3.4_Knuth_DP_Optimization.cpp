/*

Computes interval dynamic programming recurrences whose optimal split points are monotone. Knuth
optimization applies to recurrences of the form `dp[l][r] = min(dp[l][k] + dp[k][r]) + cost(l, r)`,
where the optimal split `opt[l][r]` satisfies `opt[l][r - 1] <= opt[l][r] <= opt[l + 1][r]`.

Common applications include optimal binary search trees and some range merging problems. The caller
is responsible for verifying the quadrangle inequality and monotonicity assumptions for the chosen
`cost(l, r)`.

- `knuth_interval_dp(n, cost, &opt)` computes minimum costs for all half-open intervals [`l`, `r`)
  over `n` items. The template parameter `cost` must be callable such that `cost(l, r)` returns the
  interval cost added after choosing the best split. If the optional pointer `opt` is supplied, it
  is filled with the chosen split points.

Time Complexity:
- O(n^2) calls to `cost(l, r)` and O(n^2) candidate split checks.

Space Complexity:
- O(n^2) for the `dp` and `opt` tables.

*/

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

const int64_t INF = (1LL << 62);

template<class Cost>
std::vector<std::vector<int64_t>> knuth_interval_dp(
    int n, Cost cost, std::vector<std::vector<int>> *out = nullptr
) {
  std::vector<std::vector<int64_t>> dp(n + 1, std::vector<int64_t>(n + 1, 0));
  std::vector<std::vector<int>> opt(n + 1, std::vector<int>(n + 1, 0));
  for (int i = 0; i <= n; i++) {
    opt[i][i] = i;
    if (i < n) {
      opt[i][i + 1] = i + 1;
    }
  }
  for (int len = 2; len <= n; len++) {
    for (int l = 0; l + len <= n; l++) {
      int r = l + len;
      dp[l][r] = INF;
      int start = std::max(opt[l][r - 1], l + 1);
      int finish = std::min(opt[l + 1][r], r - 1);
      for (int k = start; k <= finish; k++) {
        int64_t candidate = dp[l][k] + dp[k][r] + cost(l, r);
        if (candidate < dp[l][r]) {
          dp[l][r] = candidate;
          opt[l][r] = k;
        }
      }
    }
  }
  if (out != nullptr) {
    *out = opt;
  }
  return dp;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct MergeCost {
  vector<int64_t> prefix;

  explicit MergeCost(const vector<int> &a) : prefix(a.size() + 1) {
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      prefix[i + 1] = prefix[i] + a[i];
    }
  }

  int64_t operator()(int l, int r) const { return prefix[r] - prefix[l]; }
};

int main() {
  vector<int> a{1, 2, 3, 4};
  vector<vector<int64_t>> dp = knuth_interval_dp(a.size(), MergeCost(a));
  assert(dp[0][4] == 19);
  return 0;
}
