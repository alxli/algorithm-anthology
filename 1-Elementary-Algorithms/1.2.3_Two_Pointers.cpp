/*

Uses two moving indices to process subarrays in linear time. This technique is most useful when
expanding the right endpoint of a window monotonically and shrinking the left endpoint monotonically
preserves enough state to test a condition or update an answer.

The examples below cover two common patterns: finding the shortest subarray with sum at least a
target when all values are nonnegative, and finding the longest subarray with at most `k` distinct
values.

- `min_length_at_least(a, target)` returns the minimum length of a contiguous subarray with sum at
  least `target`, or $-1$ if none exists. Values in `a` must be nonnegative.
- `longest_at_most_k_distinct(a, k)` returns the maximum length of a contiguous subarray containing
  at most `k` distinct values.

Time Complexity:
- O(n) expected per call, where $n$ is the array size.

Space Complexity:
- O(1) auxiliary for `min_length_at_least(a, target)`.
- O(k) auxiliary heap space for `longest_at_most_k_distinct(a, k)`.

*/

#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include <vector>

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
  vector<int> a{2, 3, 1, 2, 4, 3};
  assert(min_length_at_least(a, 7) == 2);  // [4, 3].

  vector<int> b{1, 2, 1, 3, 4, 3, 5};
  assert(longest_at_most_k_distinct(b, 2) == 3);  // [1, 2, 1].
  return 0;
}
