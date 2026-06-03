/*

Computes one layer of a dynamic programming recurrence whose optimal transition indices are
monotone. This optimization applies to recurrences of the form
`dp_cur[i] = min(dp_prev[k] + cost(k, i))`, where the minimizing index for `i` never decreases as
`i` increases.

The function below computes `dp_cur[l..r]` by evaluating the midpoint first, then recursing on the
left half with a restricted upper bound on the optimal `k`, and on the right half with a restricted
lower bound. The caller is responsible for checking that the recurrence satisfies the required
monotonicity condition.

- `compute_dc_layer(dp_prev, dp_cur, l, r, opt_l, opt_r, cost)` fills `dp_cur[l..r]` using candidate
  transition indices in `[opt_l, opt_r]`.
- `cost(k, i)` must return the transition cost from previous state `k` to current state `i`.

Time Complexity:
- O(n log n) calls to `cost(k, i)` per layer when each state has O(n) possible transitions.

Space Complexity:
- O(log n) auxiliary stack space.

*/

#include <algorithm>
#include <vector>

const long long INF = (1LL << 62);

template<class CostFunction>
void compute_dc_layer(
    const std::vector<long long> &dp_prev, std::vector<long long> &dp_cur, int l, int r, int opt_l,
    int opt_r, CostFunction cost
) {
  if (l > r) {
    return;
  }
  int mid = (l + r) / 2;
  long long best = INF;
  int best_k = opt_l;
  int upper = std::min(mid, opt_r);
  for (int k = opt_l; k <= upper; k++) {
    long long candidate = dp_prev[k] + cost(k, mid);
    if (candidate < best) {
      best = candidate;
      best_k = k;
    }
  }
  dp_cur[mid] = best;
  compute_dc_layer(dp_prev, dp_cur, l, mid - 1, opt_l, best_k, cost);
  compute_dc_layer(dp_prev, dp_cur, mid + 1, r, best_k, opt_r, cost);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct SquareSegmentCost {
  vector<long long> prefix;

  explicit SquareSegmentCost(const vector<int> &a) : prefix(a.size() + 1) {
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      prefix[i + 1] = prefix[i] + a[i];
    }
  }

  long long operator()(int k, int i) const {
    long long sum = prefix[i] - prefix[k];
    return sum * sum;
  }
};

int main() {
  int raw[] = {1, 2, 3, 4};
  vector<int> a(raw, raw + 4);
  int n = a.size();
  SquareSegmentCost cost(a);

  vector<long long> dp_prev(n + 1, INF), dp_cur(n + 1, INF);
  dp_prev[0] = 0;
  compute_dc_layer(dp_prev, dp_cur, 1, n, 0, n - 1, cost);
  assert(dp_cur[4] == 100);  // One group: (1 + 2 + 3 + 4)^2.

  vector<long long> dp_two(n + 1, INF);
  compute_dc_layer(dp_cur, dp_two, 1, n, 0, n - 1, cost);
  assert(dp_two[4] == 52);  // Split as [1, 2] and [3, 4].
  return 0;
}
