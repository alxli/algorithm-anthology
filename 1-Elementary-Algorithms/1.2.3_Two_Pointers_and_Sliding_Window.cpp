/*

Uses two moving indices to avoid trying all pairs of positions. This technique has two common forms:
opposite-end pointers over sorted arrays, and a sliding window over contiguous subarrays. For sorted
2-sum, if the current sum is too small, pairing the left value with any smaller right value also
fails, so the left pointer can safely advance; if the sum is too large, the symmetric argument lets
the right pointer retreat. Each step therefore discards an entire row or column of candidate pairs.

A sliding window applies the same idea when extending and shrinking a range changes its state
monotonically. With nonnegative values, extending a window cannot decrease its sum and removing its
leftmost value cannot increase it, so no shorter feasible window is skipped. For a bound such as at
most $k$ distinct values, the left pointer advances only until the window becomes valid again. Since
neither endpoint moves backward, each element enters and leaves the window at most once.

For fixed-length windows, a monotonic deque of indices can report every minimum or maximum in linear
total time. It stores only candidates that could still become the answer, ordered from the current
extreme at the front to the least useful value at the back. A new element removes every older value
that it dominates, while expired indices are removed from the front. Each index is again pushed and
popped at most once.

The examples below cover sorted 2-sum/3-sum variants, finding the shortest subarray with sum at
least a target when all values are nonnegative, and finding the longest subarray with at most $k$
distinct values. They also cover the minimum or maximum of every fixed-length window and an online
dynamic programming recurrence.

- `two_sum_sorted(a, target)` returns two indices whose values sum to `target`, or $(-1, -1)$ if no
  such pair exists. The input array must be sorted.
- `three_sum(a, target)` returns a tuple of three original indices whose values sum to `target`, or
  $(-1, -1, -1)$ if none exist.
- `min_subarray_at_least(a, target)` returns a tuple (`length`, `lo`, `hi`), the minimum length and
  inclusive endpoints of a contiguous subarray with sum at least `target`, or length $-1$ if none
  exists. Values in `a` must be nonnegative. If `target` is nonpositive, it returns the empty
  subarray.
- `max_subarray_at_most_k_distinct(a, k)` returns a tuple (`length`, `lo`, `hi`), the maximum length
  and inclusive endpoints of a contiguous subarray containing at most `k` distinct values. If `k` is
  nonpositive, it returns the empty subarray.
- `sliding_window_extrema(a, k, comp = std::less<>)` returns the extreme value in each window of
  length `k`. With the default `less<>` comparator it returns minimums; passing `greater<>` returns
  maximums.

Time Complexity:
- O(n) per call to `two_sum_sorted()` and `min_subarray_at_least()`, where $n$ is the array size.
- O(n^2) per call to `three_sum()`.
- O(n) expected per call to `max_subarray_at_most_k_distinct()`.
- O(n) per call to `sliding_window_extrema()`.

Space Complexity:
- O(1) auxiliary for `two_sum_sorted(a, target)`.
- O(n) auxiliary for `three_sum(a, target)`.
- O(1) auxiliary for `min_subarray_at_least(a, target)`.
- O(k) auxiliary for `max_subarray_at_most_k_distinct(a, k)`.
- O(k) auxiliary and O(n) for the result of `sliding_window_extrema()`.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <deque>
#include <functional>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

std::pair<int, int> two_sum_sorted(const std::vector<int> &a, int64_t target) {
  for (int lo = 0, hi = static_cast<int>(a.size()) - 1; lo < hi;) {
    int64_t sum = static_cast<int64_t>(a[lo]) + a[hi];
    if (sum == target) {
      return {lo, hi};
    } else if (sum < target) {
      lo++;
    } else {
      hi--;
    }
  }
  return {-1, -1};
}

std::tuple<int, int, int> three_sum(const std::vector<int> &a, int64_t target) {
  int n = static_cast<int>(a.size());
  std::vector<std::pair<int, int>> sorted;
  sorted.reserve(a.size());
  for (int i = 0; i < n; i++) {
    sorted.push_back({a[i], i});
  }
  std::sort(sorted.begin(), sorted.end());
  for (int i = 0; i < n; i++) {
    int lo = i + 1, hi = n - 1;
    while (lo < hi) {
      int64_t sum = static_cast<int64_t>(sorted[i].first) + sorted[lo].first + sorted[hi].first;
      if (sum == target) {
        return {sorted[i].second, sorted[lo].second, sorted[hi].second};
      } else if (sum < target) {
        lo++;
      } else {
        hi--;
      }
    }
  }
  return {-1, -1, -1};
}

std::tuple<int, int, int> min_subarray_at_least(const std::vector<int> &a, int64_t target) {
  if (target <= 0) {
    return {0, 0, -1};
  }
  int best = static_cast<int>(a.size()) + 1, best_lo = 0, best_hi = -1;
  int64_t sum = 0;
  for (int lo = 0, hi = 0; hi < static_cast<int>(a.size()); hi++) {
    sum += a[hi];  // Overflow warning.
    while (sum >= target) {
      if (hi - lo + 1 < best) {
        best = hi - lo + 1;
        best_lo = lo;
        best_hi = hi;
      }
      sum -= a[lo++];
    }
  }
  if (best == static_cast<int>(a.size()) + 1) {
    return {-1, 0, -1};
  }
  return {best, best_lo, best_hi};
}

std::tuple<int, int, int> max_subarray_at_most_k_distinct(const std::vector<int> &a, int k) {
  if (k <= 0) {
    return {0, 0, -1};
  }
  std::unordered_map<int, int> count;
  int best = 0, best_lo = 0, best_hi = -1;
  for (int lo = 0, hi = 0; hi < static_cast<int>(a.size()); hi++) {
    count[a[hi]]++;
    while (static_cast<int>(count.size()) > k) {
      if (--count[a[lo]] == 0) {
        count.erase(a[lo]);
      }
      lo++;
    }
    if (best < hi - lo + 1) {
      best = hi - lo + 1;
      best_lo = lo;
      best_hi = hi;
    }
  }
  return {best, best_lo, best_hi};
}

template<typename T, typename Compare = std::less<>>
std::vector<T> sliding_window_extrema(const std::vector<T> &a, int k, Compare comp = Compare{}) {
  int n = static_cast<int>(a.size());
  assert(1 <= k && k <= n);
  std::deque<int> window;
  std::vector<T> res;
  res.reserve(n - k + 1);
  for (int i = 0; i < n; i++) {
    while (!window.empty() && !comp(a[window.back()], a[i])) {
      window.pop_back();
    }
    window.push_back(i);
    if (window.front() <= i - k) {
      window.pop_front();
    }
    if (i >= k - 1) {
      res.push_back(a[window.front()]);
    }
  }
  return res;
}

/*

A monotone queue can be used to maintain the minimum or maximum value in an online sliding window.
This is useful for dynamic programming recurrences where each transition may only come from one of
the last $w$ states, such as $dp(i) = a_i + \min_{j \in [i - w, i - 1]} dp(j)$.

The queue stores candidate (`index`, `value`) pairs in monotone order. Expired indices are removed
from the front, and dominated values are removed from the back before inserting a new candidate. An
index is an ordered `int` timestamp used only for expiration: indices need not be contiguous or
nonnegative, but they must be pushed in strictly increasing order, and expiration thresholds must
not decrease.

- `MonotoneQueue<T>()` constructs an empty queue for minimum queries. Instantiate
  `MonotoneQueue<T, std::greater<T>>` for maximum queries.
- `push(index, value)` inserts candidate `value` with timestamp `index`, removing dominated
  candidates from the back.
- `expire(first_valid)` removes candidates whose indices are less than `first_valid`.
- `top()` returns the best active (`index`, `value`) pair. The queue must be nonempty.
- `empty()` returns whether the queue has no active candidates.

Time Complexity:
- O(n) for any sequence of $n$ calls to `push(index, value)` and `expire(first_valid)`.
- O(1) amortized per call to `push(index, value)` and `expire(first_valid)`.
- O(1) worst-case per call to `top()` and `empty()`.

Space Complexity:
- O(w) for storage of a sliding window of width $w$.

*/

template<typename T, typename Compare = std::less<T>>
class MonotoneQueue {
  std::deque<std::pair<int, T>> q;
  Compare better;

 public:
  bool empty() const { return q.empty(); }

  void push(int index, const T &value) {
    while (!q.empty() && !better(q.back().second, value)) {
      q.pop_back();
    }
    q.emplace_back(index, value);
  }

  void expire(int first_valid) {
    while (!q.empty() && q.front().first < first_valid) {
      q.pop_front();
    }
  }

  std::pair<int, T> top() const {
    assert(!q.empty());
    return q.front();
  }
};

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int> c{-3, -1, 2, 4, 8, 11};
  assert(two_sum_sorted(c, 10) == make_pair(1, 5));  // -1 + 11.
  assert(two_sum_sorted(c, 7) == make_pair(1, 4));   // -1 + 8.
  assert(two_sum_sorted(c, 100) == make_pair(-1, -1));

  vector<int> d{4, -1, 8, 2, -3, 11};
  auto [i, j, k] = three_sum(d, 9);
  assert(d[i] + d[j] + d[k] == 9);
  assert((three_sum(d, 30) == make_tuple(-1, -1, -1)));

  vector<int> a{2, 3, 1, 2, 4, 3};
  auto [length, lo, hi] = min_subarray_at_least(a, 7);
  assert(length == 2);
  assert(lo == 4 && hi == 5);  // [4, 3].
  auto [empty_length, empty_lo, empty_hi] = min_subarray_at_least(a, 0);
  assert(empty_length == 0);
  assert(empty_lo == 0 && empty_hi == -1);

  vector<int> b{1, 2, 1, 3, 4, 3, 5};
  auto [longest, longest_lo, longest_hi] = max_subarray_at_most_k_distinct(b, 2);
  assert(longest == 3);
  assert(longest_lo == 0 && longest_hi == 2);  // [1, 2, 1].

  vector<int> e{1, 3, -1, -3, 5, 3, 6, 7};
  assert((sliding_window_extrema(e, 3) == vector<int>{-1, -3, -3, -3, 3, 3}));
  assert((sliding_window_extrema(e, 3, greater<>()) == vector<int>{3, 3, 5, 5, 6, 7}));

  // Toy DP: dp[i] = f[i] + min(dp[j]) for j in [i - w, i - 1].
  // Scanning the window costs O(w) per state; the monotone queue makes it O(1) amortized.
  vector<int> f{4, 2, 7, 1, 3, 6};
  int w = 3;
  vector<int> dp(f.size());
  MonotoneQueue<int> mq;
  dp[0] = f[0];
  mq.push(0, dp[0]);
  for (int i = 1; i < static_cast<int>(f.size()); i++) {
    mq.expire(i - w);
    dp[i] = f[i] + mq.top().second;
    mq.push(i, dp[i]);
  }
  assert((dp == vector<int>{4, 6, 11, 5, 8, 11}));
  return 0;
}
