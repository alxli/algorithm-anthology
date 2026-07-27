/*

Solves the fractional knapsack problem: given items with positive integer weights and nonnegative
integer values, choose any fraction of each item to maximize total value without exceeding a
capacity limit.

The greedy algorithm processes items in decreasing order of value per unit weight, taking each item
in full until only part of the next item fits. This is optimal because replacing any weight taken
from a lower-density item with the same weight from a higher-density item cannot decrease the total
value. Unlike 0-1 knapsack, allowing fractions makes each such exchange feasible.

- `fractional_knapsack(weight, value, capacity)` returns a pair (`best_value`, `fraction`)
  containing the maximum value and the fraction of each input item taken. Item `i` has weight
  `weight[i]` and value `value[i]`, and `fraction[i]` is in $[0, 1]$.

Time Complexity:
- O(n log n) per call due to sorting, where $n$ is the number of items.

Space Complexity:
- O(n) auxiliary and O(n) for the returned fractions.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <numeric>
#include <utility>
#include <vector>

std::pair<double, std::vector<double>> fractional_knapsack(
    const std::vector<int> &weight, const std::vector<int> &value, int capacity
) {
  int n = static_cast<int>(weight.size());
  assert(static_cast<int>(value.size()) == n && capacity >= 0);
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  for (int i = 0; i < n; i++) {
    assert(weight[i] > 0 && value[i] >= 0);
  }
  std::sort(order.begin(), order.end(), [&](int i, int j) {
    int64_t lhs = static_cast<int64_t>(value[i]) * weight[j];
    int64_t rhs = static_cast<int64_t>(value[j]) * weight[i];
    return lhs != rhs ? lhs > rhs : i < j;
  });
  double best_value = 0;
  std::vector<double> fraction(n, 0);
  for (int i : order) {
    int taken = std::min(capacity, weight[i]);
    fraction[i] = static_cast<double>(taken) / weight[i];
    best_value += fraction[i] * value[i];
    capacity -= taken;
    if (capacity == 0) {
      break;
    }
  }
  return {best_value, fraction};
}

/*** Example Usage ***/

#include <cmath>
using namespace std;

int main() {
  vector<int> weight{10, 20, 30};
  vector<int> value{60, 100, 120};
  auto [best_value, fraction] = fractional_knapsack(weight, value, 50);
  assert(fabs(best_value - 240) < 1e-9);
  assert((fraction == vector<double>{1, 1, 2.0 / 3}));
  return 0;
}
