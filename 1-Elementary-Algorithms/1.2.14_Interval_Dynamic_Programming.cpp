/*

Solves matrix-chain multiplication as a canonical example of interval dynamic programming. Matrix
multiplication is associative, but multiplying an $a$-by-$b$ matrix with a $b$-by-$c$ matrix costs
$abc$ scalar multiplications, so the choice of parenthesization can greatly affect the total cost.

For a chain of matrices, $dp(l,r)$ is the minimum cost of multiplying matrices $l$ through $r$.
Trying every final split $k$ gives the recurrence
$dp(l,r) = \min_{l \le k < r}(dp(l,k) + dp(k+1,r) + d_l d_{k+1} d_{r+1})$. Processing intervals in
increasing order of length ensures that both subintervals have already been solved. The same pattern
applies whenever a contiguous interval is formed by combining two smaller intervals.

- `matrix_chain_order(dimensions)` returns a pair (`operations`, `parenthesization`) containing the
  minimum number of scalar multiplications and one optimal parenthesization. Matrix `i` has
  dimensions `dimensions[i]` by `dimensions[i + 1]`; all dimensions must be positive.

The minimum cost and all intermediate products must fit in `int64_t`.

Time Complexity:
- O(n^3) per call, where $n$ is the number of matrices.

Space Complexity:
- O(n^2) auxiliary for the dynamic programming and split tables.

*/

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

std::string matrix_chain_parenthesization(
    const std::vector<std::vector<int>> &split, int lo, int hi
) {
  if (lo == hi) {
    return "A" + std::to_string(lo);
  }
  int k = split[lo][hi];
  return "(" + matrix_chain_parenthesization(split, lo, k) + " * " +
         matrix_chain_parenthesization(split, k + 1, hi) + ")";
}

std::pair<int64_t, std::string> matrix_chain_order(const std::vector<int> &dimensions) {
  assert(dimensions.size() >= 2);
  for (int d : dimensions) {
    assert(d > 0);
  }
  int n = static_cast<int>(dimensions.size()) - 1;
  std::vector<std::vector<int64_t>> dp(n, std::vector<int64_t>(n, 0));
  std::vector<std::vector<int>> split(n, std::vector<int>(n, 0));
  for (int length = 2; length <= n; length++) {
    for (int lo = 0; lo + length <= n; lo++) {
      int hi = lo + length - 1;
      for (int k = lo; k < hi; k++) {
        int64_t candidate = dp[lo][k] + dp[k + 1][hi] +
                            static_cast<int64_t>(dimensions[lo]) * dimensions[k + 1] *
                                dimensions[hi + 1];  // Overflow warning.
        if (k == lo || candidate < dp[lo][hi]) {
          dp[lo][hi] = candidate;
          split[lo][hi] = k;
        }
      }
    }
  }
  return {dp[0][n - 1], matrix_chain_parenthesization(split, 0, n - 1)};
}

/*** Example Usage ***/

using namespace std;

int main() {
  auto [operations, parenthesization] = matrix_chain_order({40, 20, 30, 10, 30});
  assert(operations == 26000);
  assert(parenthesization == "((A0 * (A1 * A2)) * A3)");

  auto [single_operations, single_parenthesization] = matrix_chain_order({3, 5});
  assert(single_operations == 0 && single_parenthesization == "A0");
  return 0;
}
