/*

Given a collection of integers and a target $v$, the subset sum problem asks for the maximum sum of
any subset that is at most $v$. Two algorithms below solve this problem in complementary regimes,
and which one wins depends on whether the bottleneck is the number of items or the size of the
weights.

The meet-in-the-middle method splits the collection in half, enumerates every subset sum for each
half, sorts one side, and binary searches for the best compatible partner. It makes no assumption
about the sign or size of the values, so it handles negative inputs, and it is the method of choice
when the number of items $n$ is small (up to roughly 40).

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

- `max_subset_sum_at_most(lo, hi, v)` returns a pair (`sum`, `items`) containing that maximum sum
  and the selected item indices relative to `lo`. Values may be negative. The range is supplied as
  random-access iterators, and 64-bit integers are used in calculations to avoid overflow.
- `max_subset_sum_bounded(lo, hi, target)` returns the maximum sum of any subset of the range
  [`lo`, `hi`) that does not exceed `target`. All weights and `target` must be nonnegative integers.
  This is faster than meet-in-the-middle when the largest weight is small relative to $2^{n/2}$.

Time Complexity:
- O(2^{n/2}) per call to `max_subset_sum_at_most()`, where $n$ is the distance between `lo`
  and `hi`.
- O(n*W) per call to `max_subset_sum_bounded()`, where $n$ is the number of items and $W$ is the
  largest weight.

Space Complexity:
- O(2^{n/2}) auxiliary heap space for `max_subset_sum_at_most()`.
- O(W) auxiliary heap space for `max_subset_sum_bounded()`.

*/

#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

template<typename It>
std::pair<int64_t, std::vector<int>> max_subset_sum_at_most(It lo, It hi, int64_t v) {
  int n = static_cast<int>(hi - lo);
  int64_t llen = 1LL << (n / 2), hlen = 1LL << (n - n / 2);
  std::vector<int64_t> lsum(llen);
  std::vector<std::pair<int64_t, int64_t>> hsum(hlen);
  for (int64_t mask = 1; mask < llen; mask++) {
    int bit = __builtin_ctzll(mask);
    lsum[mask] = lsum[mask ^ (1LL << bit)] + *(lo + bit);
  }
  for (int64_t mask = 1; mask < hlen; mask++) {
    int bit = __builtin_ctzll(mask);
    hsum[mask].first = hsum[mask ^ (1LL << bit)].first + *(lo + n / 2 + bit);
    hsum[mask].second = mask;
  }
  std::sort(hsum.begin(), hsum.end());
  int64_t best = INT64_MIN, lmask = 0, hmask = 0;
  for (int64_t mask = 0; mask < llen; mask++) {
    auto it = std::upper_bound(hsum.begin(), hsum.end(), std::make_pair(v - lsum[mask], INT64_MAX));
    if (it != hsum.begin()) {
      --it;
      int64_t candidate = lsum[mask] + it->first;
      if (best < candidate) {
        best = candidate;
        lmask = mask;
        hmask = it->second;
      }
    }
  }
  std::vector<int> items;
  for (int i = 0; i < n / 2; i++) {
    if ((lmask >> i) & 1) {
      items.push_back(i);
    }
  }
  for (int i = 0; i < n - n / 2; i++) {
    if ((hmask >> i) & 1) {
      items.push_back(n / 2 + i);
    }
  }
  return {best, items};
}

template<typename It>
int64_t max_subset_sum_bounded(It lo, It hi, int target) {
  int n = static_cast<int>(hi - lo);
  // Greedily take a prefix while it still fits; the optimum then lies within one weight of target.
  int sum = 0, b = 0;
  while (b < n && sum + static_cast<int>(*(lo + b)) <= target) {
    sum += static_cast<int>(*(lo + b++));
  }
  if (b == n) {
    return sum;  // Every item fits, so the whole collection is the best subset.
  }
  int m = 0;
  for (It it = lo; it != hi; ++it) {
    m = std::max(m, static_cast<int>(*it));
  }
  // reach[s - target + m] = largest prefix boundary that can realize a balanced subset of sum s.
  // Only sums within m of target are tracked, which is where the optimum must lie.
  std::vector<int> reach(2 * m, -1);
  reach[sum - target + m] = b;
  for (int i = b; i < n; i++) {
    std::vector<int> prev = reach;
    int wi = static_cast<int>(*(lo + i));
    for (int x = 0; x < m; x++) {  // Add item i to sums currently below target.
      reach[x + wi] = std::max(reach[x + wi], prev[x]);
    }
    for (int x = 2 * m - 1; x > m; x--) {  // Drop an earlier item from sums above target.
      for (int j = std::max(0, prev[x]); j < reach[x]; j++) {
        int wj = static_cast<int>(*(lo + j));
        reach[x - wj] = std::max(reach[x - wj], j);
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

int main() {
  std::vector<int> a{9, 1, 5, 0, 1, 11, 5};
  auto [sum, items] = max_subset_sum_at_most(a.begin(), a.end(), 8);
  int64_t selected_sum = 0;
  for (int i : items) {
    selected_sum += a[i];
  }
  assert(sum == 7 && selected_sum == sum);
  std::vector<int> b{-7, -3, -2, 5, 8};
  assert(max_subset_sum_at_most(b.begin(), b.end(), 0).first == 0);

  // The bounded-weight method agrees with meet-in-the-middle on nonnegative inputs.
  assert(max_subset_sum_bounded(a.begin(), a.end(), 8) == 7);
  std::vector<int> c{3, 34, 4, 12, 5, 2};
  assert(max_subset_sum_bounded(c.begin(), c.end(), 9) == 9);     // 4 + 5.
  assert(max_subset_sum_bounded(c.begin(), c.end(), 10) == 10);   // 3 + 5 + 2.
  assert(max_subset_sum_bounded(c.begin(), c.end(), 100) == 60);  // All items fit.
  return 0;
}
