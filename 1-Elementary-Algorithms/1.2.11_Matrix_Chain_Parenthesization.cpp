/*

Given a fixed-order chain of compatible matrices, finds a parenthesization that minimizes the number
of scalar multiplications needed to compute their product. The algorithm examines only the matrix
dimensions; it does not multiply matrix entries. Matrix multiplication is associative, but
multiplying an $a$-by-$b$ matrix with a $b$-by-$c$ matrix costs $abc$ scalar multiplications, so
different parenthesizations can have very different costs. This is a canonical example of interval
dynamic programming.

Write $d_i$ for `dimensions[i]`, so matrix $i$ has $d_i$ rows and $d_{i+1}$ columns. For a chain of
matrices, $dp(l, r)$ is the minimum cost of multiplying matrices $l$ through $r$. Trying every final
split $k$ gives the recurrence
$dp(l, r) = \min_{l \le k < r}(dp(l, k) + dp(k + 1, r) + d_l d_{k+1} d_{r+1})$. The final product
multiplies a $d_l$-by-$d_{k+1}$ matrix with a $d_{k+1}$-by-$d_{r+1}$ matrix. Processing intervals in
increasing order of length ensures that both subintervals have already been solved. The same pattern
applies whenever a contiguous interval is formed by combining two smaller intervals.

- `matrix_chain_order(dimensions)` returns a pair (`operations`, `parenthesization`) containing the
  minimum number of scalar multiplications and one optimal parenthesization. Matrix `i` has
  dimensions `dimensions[i]` by `dimensions[i + 1]`; all dimensions must be positive.

Overflow warning: The minimum cost and all intermediate products must fit in `int64_t`.

Time Complexity:
- O(n^3) per call, where $n$ is the number of matrices. The dimension values affect the returned
  cost but not the running time.

Space Complexity:
- O(n^2) auxiliary for the dynamic programming and split tables.

*/

#include <cassert>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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
  auto rec = [&](auto &&rec, int lo, int hi) -> std::string {
    if (lo == hi) {
      return "A" + std::to_string(lo);
    }
    int k = split[lo][hi];
    return "(" + rec(rec, lo, k) + " * " + rec(rec, k + 1, hi) + ")";
  };
  return {dp[0][n - 1], rec(rec, 0, n - 1)};
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
