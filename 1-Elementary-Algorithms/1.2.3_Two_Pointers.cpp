/*

Uses two moving indices to avoid trying all pairs of positions. This technique has two common forms:
opposite-end pointers over sorted arrays, and a sliding window over contiguous subarrays. For sorted
2-sum, if the current sum is too small, pairing the left value with any smaller right value also
fails, so the left pointer can safely advance; if the sum is too large, the symmetric argument lets
the right pointer retreat. Each step therefore discards an entire row or column of candidate pairs.

A sliding window applies the same idea when extending and shrinking a range changes its state
monotonically. With nonnegative values, extending a window cannot decrease its sum and removing its
leftmost value cannot increase it, so no shorter feasible window is skipped. For a bound such as at
most `k` distinct values, the left pointer advances only until the window becomes valid again. Since
neither endpoint moves backward, each element enters and leaves the window at most once.

The examples below cover sorted 2-sum/3-sum variants, finding the shortest subarray with sum at
least a target when all values are nonnegative, and finding the longest subarray with at most `k`
distinct values.

- `two_sum_sorted(a, target)` returns two indices whose values sum to `target`, or $(-1, -1)$ if no
  such pair exists. The input array must be sorted.
- `three_sum(a, target)` returns three original indices whose values sum to `target`, or
  $(-1, -1, -1)$ if none exist.
- `min_length_at_least(a, target)` returns the minimum length and inclusive endpoints of a
  contiguous subarray with sum at least `target`, or length $-1$ if none exists. Values in `a` must
  be nonnegative. If `target` is nonpositive, it returns the empty subarray with length 0.
- `longest_at_most_k_distinct(a, k)` returns the maximum length and inclusive endpoints of a
  contiguous subarray containing at most `k` distinct values.

The two window functions return `(length, lo, hi)`. An empty or nonexistent result is represented
by endpoints [0, $-1$].

Time Complexity:
- O(n) per call to `two_sum_sorted()` and `min_length_at_least()`, where $n$ is the array size.
- O(n^2) per call to `three_sum()`.
- O(n) expected per call to `longest_at_most_k_distinct()`.

Space Complexity:
- O(1) auxiliary for `two_sum_sorted(a, target)`.
- O(n) auxiliary heap space for `three_sum(a, target)`.
- O(1) auxiliary for `min_length_at_least(a, target)`.
- O(k) auxiliary heap space for `longest_at_most_k_distinct(a, k)`.

*/

#include <algorithm>
#include <array>
#include <cstdint>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

std::pair<int, int> two_sum_sorted(const std::vector<int> &a, int64_t target) {
  for (int l = 0, r = static_cast<int>(a.size()) - 1; l < r;) {
    int64_t sum = static_cast<int64_t>(a[l]) + a[r];
    if (sum == target) {
      return {l, r};
    } else if (sum < target) {
      l++;
    } else {
      r--;
    }
  }
  return {-1, -1};
}

std::array<int, 3> three_sum(const std::vector<int> &a, int64_t target) {
  std::vector<std::pair<int, int>> sorted;
  sorted.reserve(a.size());
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    sorted.push_back({a[i], i});
  }
  std::sort(sorted.begin(), sorted.end());
  for (int i = 0; i < static_cast<int>(sorted.size()); i++) {
    int l = i + 1, r = static_cast<int>(sorted.size()) - 1;
    while (l < r) {
      int64_t sum = static_cast<int64_t>(sorted[i].first) + sorted[l].first + sorted[r].first;
      if (sum == target) {
        return {sorted[i].second, sorted[l].second, sorted[r].second};
      } else if (sum < target) {
        l++;
      } else {
        r--;
      }
    }
  }
  return {-1, -1, -1};
}

std::tuple<int, int, int> min_length_at_least(const std::vector<int> &a, int64_t target) {
  if (target <= 0) {
    return {0, 0, -1};
  }
  int best = static_cast<int>(a.size()) + 1, best_lo = 0, best_hi = -1;
  int64_t sum = 0;
  for (int l = 0, r = 0; r < static_cast<int>(a.size()); r++) {
    sum += a[r];
    while (sum >= target) {
      if (r - l + 1 < best) {
        best = r - l + 1;
        best_lo = l;
        best_hi = r;
      }
      sum -= a[l++];
    }
  }
  if (best == static_cast<int>(a.size()) + 1) {
    return {-1, 0, -1};
  }
  return {best, best_lo, best_hi};
}

std::tuple<int, int, int> longest_at_most_k_distinct(const std::vector<int> &a, int k) {
  if (k <= 0) {
    return {0, 0, -1};
  }
  std::unordered_map<int, int> count;
  int best = 0, best_lo = 0, best_hi = -1;
  for (int l = 0, r = 0; r < static_cast<int>(a.size()); r++) {
    count[a[r]]++;
    while (static_cast<int>(count.size()) > k) {
      if (--count[a[l]] == 0) {
        count.erase(a[l]);
      }
      l++;
    }
    if (best < r - l + 1) {
      best = r - l + 1;
      best_lo = l;
      best_hi = r;
    }
  }
  return {best, best_lo, best_hi};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> c{-3, -1, 2, 4, 8, 11};
  assert(two_sum_sorted(c, 10) == make_pair(1, 5));  // -1 + 11.
  assert(two_sum_sorted(c, 7) == make_pair(1, 4));   // -1 + 8.
  assert(two_sum_sorted(c, 100) == make_pair(-1, -1));

  vector<int> d{4, -1, 8, 2, -3, 11};
  array<int, 3> triple = three_sum(d, 9);
  assert(d[triple[0]] + d[triple[1]] + d[triple[2]] == 9);
  assert((three_sum(d, 30) == array<int, 3>{-1, -1, -1}));

  vector<int> a{2, 3, 1, 2, 4, 3};
  auto [length, lo, hi] = min_length_at_least(a, 7);
  assert(length == 2);
  assert(lo == 4 && hi == 5);  // [4, 3].
  auto [empty_length, empty_lo, empty_hi] = min_length_at_least(a, 0);
  assert(empty_length == 0);
  assert(empty_lo == 0 && empty_hi == -1);

  vector<int> b{1, 2, 1, 3, 4, 3, 5};
  auto [longest, longest_lo, longest_hi] = longest_at_most_k_distinct(b, 2);
  assert(longest == 3);
  assert(longest_lo == 0 && longest_hi == 2);  // [1, 2, 1].
  return 0;
}
