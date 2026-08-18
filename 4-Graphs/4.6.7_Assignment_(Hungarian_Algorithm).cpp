/*

Solves the minimum-cost or maximum-value assignment problem for a rectangular matrix. Given $n$
workers and $m$ jobs (with $n \leq m$), choose a distinct job for each worker so that the total cost
is minimized or the total value is maximized.

The classical Hungarian algorithm using potentials transforms a cost matrix using the principle that
adding or subtracting constants from rows or columns does not change the optimal assignment. Workers
are assigned one row at a time: each new row triggers a search for a least-cost augmenting path over
zero-slack edges, with the potentials adjusted along the way to expose new zero-slack edges until
the path reaches a free job. Note that while the input matrix is 0-based here, the internal
calculations are 1-based.

Maximum-value assignment reduces to minimum-cost assignment by replacing each value $x$ with
$M - x$, where $M$ is the largest matrix entry. Every assignment contains exactly $n$ entries, so
minimizing the transformed total $nM - \sum x$ maximizes the original total.

- `min_assignment(cost)` returns a pair (`min_cost`, `assignment`) for a 0-based `cost` matrix,
  where `assignment` contains $n$ values and `assignment[i]` is the chosen job for worker `i`, using
  0-based job numbers.
- `max_assignment(value)` returns the analogous pair (`max_value`, `assignment`) maximizing a
  0-based `value` matrix.

Time Complexity:
- O(n^2*m) per call, where the input matrix has $n$ rows and $m$ columns.

Space Complexity:
- O(n + m) auxiliary, not counting the input matrix and returned assignment.
- O(n*m) auxiliary for `max_assignment()` due to the transformed matrix.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <utility>
#include <vector>

template<typename T>
std::pair<T, std::vector<int>> min_assignment(const std::vector<std::vector<T>> &cost) {
  static const T INF = std::numeric_limits<T>::max() / 4;
  int n = static_cast<int>(cost.size());
  int m = cost.empty() ? 0 : static_cast<int>(cost[0].size());
  assert(n <= m);
  std::vector<T> u(n + 1), v(m + 1), minv(m + 1);
  std::vector<int> p(m + 1), way(m + 1);
  std::vector<char> used(m + 1);
  for (int i = 1; i <= n; i++) {
    minv.assign(m + 1, INF);
    used.assign(m + 1, false);
    p[0] = i;
    int j0 = 0;
    do {
      used[j0] = true;
      int i0 = p[j0], j1 = 0;
      T delta = INF;
      for (int j = 1; j <= m; j++) {
        if (used[j]) {
          continue;
        }
        T cur = cost[i0 - 1][j - 1] - u[i0] - v[j];  // Overflow warning.
        if (cur < minv[j]) {
          minv[j] = cur;
          way[j] = j0;
        }
        if (minv[j] < delta) {
          delta = minv[j];
          j1 = j;
        }
      }
      for (int j = 0; j <= m; j++) {
        if (used[j]) {
          u[p[j]] += delta;
          v[j] -= delta;
        } else {
          minv[j] -= delta;
        }
      }
      j0 = j1;
    } while (p[j0] != 0);
    do {
      int j1 = way[j0];
      p[j0] = p[j1];
      j0 = j1;
    } while (j0 != 0);
  }
  // Optional: reconstruct one optimal assignment.
  std::vector<int> assignment(n, -1);
  for (int j = 1; j <= m; j++) {
    if (p[j] != 0) {
      assignment[p[j] - 1] = j - 1;
    }
  }
  return {-v[0], assignment};  // Overflow warning.
}

template<typename T>
std::pair<T, std::vector<int>> max_assignment(std::vector<std::vector<T>> value) {
  if (value.empty()) {
    return {0, {}};
  }
  T max_value = value[0][0];
  for (const auto &row : value) {
    max_value = std::max(max_value, *std::max_element(row.begin(), row.end()));
  }
  for (auto &row : value) {
    for (T &x : row) {
      x = max_value - x;
    }
  }
  auto [min_cost, assignment] = min_assignment(value);
  return {max_value * static_cast<T>(value.size()) - min_cost, assignment};  // Overflow warning.
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<vector<int64_t>> cost{
      {9, 2, 7, 8},
      {6, 4, 3, 7},
      {5, 8, 1, 8},
  };
  auto [min_cost, assignment] = min_assignment(cost);
  assert(min_cost == 9 && (assignment == vector<int>{1, 0, 2}));

  auto [max_value, max_jobs] = max_assignment(cost);
  assert(max_value == 24 && (max_jobs == vector<int>{0, 3, 1}));
  return 0;
}
