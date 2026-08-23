/*

Given a collection of integers and a target, the subset sum problem asks for the maximum sum of any
subset that does not exceed the target. Two algorithms below solve this problem in complementary
regimes, and which one wins depends on whether the bottleneck is the number of items or the size of
the weights.

The meet-in-the-middle method splits the collection in half, enumerates every subset sum for each
half, sorts one side, and binary searches for the best compatible partner. It makes no assumption
about the sign or size of the values, so it handles negative inputs, and it is the method of choice
when the number of items $n$ is small (up to roughly $40$).

Pisinger's algorithm targets the opposite regime: nonnegative weights whose maximum value is small,
with possibly many items. Let $W$ be the largest weight. It greedily takes a maximal prefix with sum
$s$ that fits the target; because the next weight is at most $W$ and does not fit, $s$ is greater
than `target` $- W$. The prefix itself is feasible, so the optimum lies between $s$ and `target` and
only that width-$W$ window matters. A balanced sweep explores exchanges that add later items to sums
below the target and remove earlier items from sums above it. For each sum, it retains the farthest
prefix boundary reached, which dominates smaller boundaries because it permits at least the same
future exchanges. This gives a running time linear in the number of items times $W$, independent of
the target beyond that window. This routine is sometimes called "fast knapsack," a slight misnomer:
it maximizes a subset sum rather than packing items with distinct values, so it is really the
bounded-weight case of subset sum rather than the 0-1 value knapsack.

- `max_subset_sum_at_most(values, target)` returns a pair (`sum`, `items`) containing that maximum
  sum and the selected item indices. Values may be negative, and `target` must be nonnegative.
- `max_subset_sum_bounded(weights, target)` returns the maximum sum of any subset of `weights` that
  does not exceed `target`. All weights and `target` must be nonnegative integers. This is faster
  than meet-in-the-middle when the largest weight is small relative to $2^{n/2}$.

Overflow warning: All subset sums and intermediate differences in `max_subset_sum_at_most()` must
fit in `int64_t`.

Time Complexity:
- O(n*2^{n/2}) per call to `max_subset_sum_at_most()`, where $n$ is the number of values.
- O(n*W) per call to `max_subset_sum_bounded()`, where $n$ is the number of items and $W$ is the
  largest weight.

Space Complexity:
- O(2^{n/2}) auxiliary for `max_subset_sum_at_most()`.
- O(W) auxiliary for `max_subset_sum_bounded()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

std::pair<int64_t, std::vector<int>> max_subset_sum_at_most(
    const std::vector<int> &values, int64_t target
) {
  assert(target >= 0);
  int n = static_cast<int>(values.size());
  assert(n / 2 < 63 && n - n / 2 < 63);
  int64_t llen = 1LL << (n / 2), rlen = 1LL << (n - n / 2);
  std::vector<int64_t> lsum(llen);
  std::vector<std::pair<int64_t, int64_t>> rsum(rlen);
  for (int64_t mask = 1; mask < llen; mask++) {
    int bit = __builtin_ctzll(mask);
    lsum[mask] = lsum[mask ^ (1LL << bit)] + values[bit];  // Overflow warning.
  }
  for (int64_t mask = 1; mask < rlen; mask++) {
    int bit = __builtin_ctzll(mask);
    rsum[mask].first = rsum[mask ^ (1LL << bit)].first + values[n / 2 + bit];
    rsum[mask].second = mask;
  }
  std::sort(rsum.begin(), rsum.end());
  int64_t best = INT64_MIN, lmask = 0, rmask = 0;
  for (int64_t mask = 0; mask < llen; mask++) {
    int64_t limit = target - lsum[mask];  // Overflow warning.
    auto it = std::upper_bound(rsum.begin(), rsum.end(), std::make_pair(limit, INT64_MAX));
    if (it != rsum.begin()) {
      --it;
      int64_t candidate = lsum[mask] + it->first;
      if (best < candidate) {
        best = candidate;
        lmask = mask;
        rmask = it->second;
      }
    }
  }
  // Optional: reconstruct one optimal subset.
  std::vector<int> items;
  for (int i = 0; i < n / 2; i++) {
    if ((lmask >> i) & 1) {
      items.push_back(i);
    }
  }
  for (int i = 0; i < n - n / 2; i++) {
    if ((rmask >> i) & 1) {
      items.push_back(n / 2 + i);
    }
  }
  return {best, items};
}

int max_subset_sum_bounded(const std::vector<int> &weights, int target) {
  assert(target >= 0);
  assert(std::all_of(weights.begin(), weights.end(), [](int w) { return w >= 0; }));
  int n = static_cast<int>(weights.size());
  // Greedily take a prefix while it still fits; the optimum then lies within one weight of target.
  int sum = 0, b = 0;
  while (b < n && weights[b] <= target - sum) {
    sum += weights[b++];
  }
  if (b == n) {
    return sum;  // Every item fits, so the whole collection is the best subset.
  }
  int m = 0;
  for (int w : weights) {
    m = std::max(m, w);
  }
  // reach[s - target + m] = largest prefix boundary that can realize a balanced subset of sum s.
  // Only sums within m of target are tracked, which is where the optimum must lie.
  std::vector<int> reach(2 * m, -1);
  reach[sum - target + m] = b;
  for (int i = b; i < n; i++) {
    std::vector<int> prev = reach;
    for (int x = 0; x < m; x++) {  // Add item i to sums currently below target.
      reach[x + weights[i]] = std::max(reach[x + weights[i]], prev[x]);
    }
    for (int x = 2 * m - 1; x > m; x--) {  // Drop an earlier item from sums above target.
      for (int j = std::max(0, prev[x]); j < reach[x]; j++) {
        reach[x - weights[j]] = std::max(reach[x - weights[j]], j);
      }
    }
  }
  int s = target;
  while (reach[s - target + m] < 0) {
    s--;
  }
  return s;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{9, 1, 5, 0, 1, 11, 5};
  auto [sum, items] = max_subset_sum_at_most(a, 8);
  int64_t selected_sum = 0;
  for (int i : items) {
    selected_sum += a[i];
  }
  assert(sum == 7 && selected_sum == sum);
  vector<int> b{-7, -3, -2, 5, 8};
  assert(max_subset_sum_at_most(b, 0).first == 0);

  // The bounded-weight method agrees with meet-in-the-middle on nonnegative inputs.
  assert(max_subset_sum_bounded(a, 8) == 7);
  vector<int> c{3, 34, 4, 12, 5, 2};
  assert(max_subset_sum_bounded(c, 9) == 9);     // 4 + 5.
  assert(max_subset_sum_bounded(c, 10) == 10);   // 3 + 5 + 2.
  assert(max_subset_sum_bounded(c, 100) == 60);  // All items fit.
  return 0;
}
