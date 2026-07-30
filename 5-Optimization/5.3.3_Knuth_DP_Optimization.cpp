/*

Computes minimum costs for every half-open interval $[`l`, `r`)$ using the recurrence
`dp[l][r] = min(dp[l][k] + dp[k][r]) + cost(l, r)`. Each state chooses a split `k` strictly inside
the interval and pays `cost(l, r)` after solving the two resulting subintervals.

A direct implementation checks every split and takes O(n^3). Knuth optimization applies when the
smallest optimal splits satisfy `opt[l][r - 1]` $\leq$ `opt[l][r]` $\leq$ `opt[l + 1][r]`. The two
neighboring states therefore bound the splits that must be checked for `dp[l][r]`, reducing the
total number of candidate checks to O(n^2). Common applications include optimal binary search trees
and some range merging problems. The caller must verify the required monotonicity property for the
chosen `cost(l, r)`; the quadrangle inequality and interval monotonicity of the cost are common
sufficient conditions.

- `knuth_interval_dp(n, cost, &opt_out)` computes minimum costs for all half-open intervals
  $[`l`, `r`)$ over `n` items. The template parameter `cost` must be callable such that `cost(l, r)`
  returns the interval cost added after choosing the best split. If the optional pointer `opt_out`
  is supplied, it is filled with the chosen split points.

Time Complexity:
- O(n^2) calls to `cost()` and O(n^2) candidate split checks per call.

Space Complexity:
- O(n^2) for the `dp` and `opt` tables.

*/

#include <algorithm>
#include <cstdint>
#include <vector>

const int64_t INF = INT64_MAX / 4;

template<typename Cost>
std::vector<std::vector<int64_t>> knuth_interval_dp(
    int n, Cost cost, std::vector<std::vector<int>> *opt_out = nullptr
) {
  std::vector<std::vector<int64_t>> dp(n + 1, std::vector<int64_t>(n + 1, 0));
  std::vector<std::vector<int>> opt(n + 1, std::vector<int>(n + 1, 0));
  // Empty and one-item intervals have cost 0.
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
      int split_lo = std::max(opt[l][r - 1], l + 1);
      int split_hi = std::min(opt[l + 1][r], r - 1);
      int64_t interval_cost = cost(l, r);
      for (int k = split_lo; k <= split_hi; k++) {
        int64_t candidate = dp[l][k] + dp[k][r] + interval_cost;  // Overflow warning.
        if (candidate < dp[l][r]) {
          dp[l][r] = candidate;
          opt[l][r] = k;
        }
      }
    }
  }
  if (opt_out != nullptr) {
    *opt_out = opt;
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

  int64_t operator()(int l, int r) const {
    // Cost of merging the half-open interval a[l..r) into one group.
    return prefix[r] - prefix[l];
  }
};

int main() {
  vector<int> a{1, 2, 3, 4};
  vector<vector<int>> opt;
  vector<vector<int64_t>> dp = knuth_interval_dp(a.size(), MergeCost(a), &opt);
  assert(dp[0][4] == 19);  // Merge 1 + 2, then 3 + 3, then 6 + 4.
  assert(opt[0][4] == 3);  // The final merge splits [1, 2, 3] from [4].
  return 0;
}
