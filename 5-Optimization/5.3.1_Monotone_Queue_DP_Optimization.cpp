/*

Maintains the minimum or maximum value in a sliding window using a monotone queue. This is useful
for dynamic programming recurrences where each transition may only come from one of the last $w$
states, such as `dp[i] = a[i] + min(dp[j])` for `j` $\in$ `[i - w, i - 1)`.

The queue stores candidate `(index, value)` pairs in monotone order. Expired indices are removed
from the front, and dominated values are removed from the back before inserting a new candidate.

- `MonotoneQueue<Compare>()` constructs an empty queue. Use `std::less<T>` for minimum queries and
  `std::greater<T>` for maximum queries.
- `push(index, value)` inserts candidate `value` at position `index`, removing dominated candidates.
- `expire(first_valid)` removes candidates with index less than `first_valid`.
- `top()` returns the best active `(index, value)` pair.
- `empty()` returns whether the queue has no active candidates.

Time Complexity:
- O(n) for any sequence of $n$ calls to `push(index, value)` and `expire(first_valid)`.
- O(1) amortized per call to `push(index, value)` and `expire(first_valid)`.
- O(1) worst-case per call to `top()` and `empty()`.

Space Complexity:
- O(w) for a sliding window of width $w$.

*/

#include <deque>
#include <functional>
#include <utility>

template<class T, class Compare>
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

  std::pair<int, T> top() const { return q.front(); }
};

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<int> a{4, 2, 7, 1, 3, 6};

  MonotoneQueue<int, less<int>> minq;
  vector<int> window_min;
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    minq.push(i, a[i]);
    minq.expire(i - 2);
    if (i >= 2) {
      window_min.push_back(minq.top().second);
    }
  }
  assert(window_min[0] == 2);
  assert(window_min[1] == 1);
  assert(window_min[2] == 1);
  assert(window_min[3] == 1);

  // dp[i] = a[i] + min(dp[j]) over max(0, i - 2) <= j < i.
  vector<int> dp(a.size());
  MonotoneQueue<int, less<int>> best;
  dp[0] = a[0];
  best.push(0, dp[0]);
  for (int i = 1; i < static_cast<int>(a.size()); i++) {
    best.expire(i - 2);
    dp[i] = a[i] + best.top().second;
    best.push(i, dp[i]);
  }
  assert(dp[5] == 13);
  return 0;
}
