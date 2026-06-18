/*

Uses two moving indices to avoid trying all pairs of positions. This technique has two common
forms: opposite-end pointers over sorted arrays, and a sliding window over contiguous subarrays.
The opposite-end form moves the smaller or larger side according to the current comparison. The
sliding-window form is useful when expanding the right endpoint and shrinking the left endpoint
monotonically preserves enough state to test a condition or update an answer.

The examples below cover sorted 2-sum/3-sum variants, finding the shortest subarray with sum at
least a target when all values are nonnegative, and finding the longest subarray with at most `k`
distinct values.

- `two_sum_sorted(a, target)` returns two indices whose values sum to `target`, or ${-1, -1}$ if no
  such pair exists. The input array must be sorted.
- `three_sum_exists(a, target)` returns whether any three values in `a` sum to `target`. It sorts a
  copy of the input array.
- `min_length_at_least(a, target)` returns the minimum length of a contiguous subarray with sum at
  least `target`, or $-1$ if none exists. Values in `a` must be nonnegative.
- `longest_at_most_k_distinct(a, k)` returns the maximum length of a contiguous subarray containing
  at most `k` distinct values.

Time Complexity:
- O(n) per call to `two_sum_sorted()` and `min_length_at_least()`, where $n$ is the array size.
- O(n^2) per call to `three_sum_exists()`.
- O(n) expected per call to `longest_at_most_k_distinct()`.

Space Complexity:
- O(1) auxiliary for `two_sum_sorted(a, target)`.
- O(n) auxiliary heap space for `three_sum_exists(a, target)`.
- O(1) auxiliary for `min_length_at_least(a, target)`.
- O(k) auxiliary heap space for `longest_at_most_k_distinct(a, k)`.

*/

#include <algorithm>
#include <cstdint>
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

bool three_sum_exists(std::vector<int> a, int64_t target) {
  std::sort(a.begin(), a.end());
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    int l = i + 1, r = static_cast<int>(a.size()) - 1;
    while (l < r) {
      int64_t sum = static_cast<int64_t>(a[i]) + a[l] + a[r];
      if (sum == target) {
        return true;
      } else if (sum < target) {
        l++;
      } else {
        r--;
      }
    }
  }
  return false;
}

int min_length_at_least(const std::vector<int> &a, int64_t target) {
  int best = static_cast<int>(a.size()) + 1;
  int64_t sum = 0;
  for (int l = 0, r = 0; r < static_cast<int>(a.size()); r++) {
    sum += a[r];
    while (sum >= target) {
      best = std::min(best, r - l + 1);
      sum -= a[l++];
    }
  }
  return best == static_cast<int>(a.size()) + 1 ? -1 : best;
}

int longest_at_most_k_distinct(const std::vector<int> &a, int k) {
  if (k <= 0) {
    return 0;
  }
  std::unordered_map<int, int> count;
  int best = 0;
  for (int l = 0, r = 0; r < static_cast<int>(a.size()); r++) {
    count[a[r]]++;
    while (static_cast<int>(count.size()) > k) {
      if (--count[a[l]] == 0) {
        count.erase(a[l]);
      }
      l++;
    }
    best = std::max(best, r - l + 1);
  }
  return best;
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
  assert(three_sum_exists(d, 9));   // -3 + 4 + 8.
  assert(!three_sum_exists(d, 30));

  vector<int> a{2, 3, 1, 2, 4, 3};
  assert(min_length_at_least(a, 7) == 2);  // [4, 3].

  vector<int> b{1, 2, 1, 3, 4, 3, 5};
  assert(longest_at_most_k_distinct(b, 2) == 3);  // [1, 2, 1].
  return 0;
}
