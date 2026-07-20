/*

Computes one layer of a dynamic program of the form `dp_cur[i] = min(dp_prev[k] + cost(k, i))`,
where $0 \leq `k` \leq `i`$. Let `opt[i]` be the smallest index `k` attaining this minimum.
Divide-and-conquer optimization applies when `opt[i]` $\leq$ `opt[i + 1]`, so the best transition
index moves only to the right as `i` increases.

To compute `dp_cur[l..r]`, evaluate its midpoint and record its best transition as `best_k`.
Monotonicity restricts every optimum in the left half to indices at most `best_k`, and every optimum
in the right half to indices at least `best_k`. Recursively applying these bounds avoids checking
every transition for every state. The caller must verify the required monotonicity property.

- `compute_dp_layer(dp_prev, dp_cur, l, r, opt_l, opt_r, cost)` fills `dp_cur[l..r]` using candidate
  transition indices in [`opt_l`, `opt_r`]. The template parameter `cost` must be callable such that
  `cost(k, i)` returns the transition cost from previous state `k` to current state `i`.

Time Complexity:
- O(n log n) calls to `cost(k, i)` per layer when each state has O(n) possible transitions.

Space Complexity:
- O(log n) auxiliary stack space.

*/

#include <algorithm>
#include <cstdint>
#include <vector>

const int64_t INF = (1LL << 62);

template<typename Cost>
void compute_dp_layer(
    const std::vector<int64_t> &dp_prev, std::vector<int64_t> &dp_cur, int l, int r, int opt_l,
    int opt_r, Cost cost
) {
  if (l > r) {
    return;
  }
  int mid = l + (r - l) / 2;
  int64_t best = INF;
  int best_k = opt_l;
  int upper = std::min(mid, opt_r);
  for (int k = opt_l; k <= upper; k++) {
    int64_t candidate = dp_prev[k] + cost(k, mid);  // Overflow warning!
    if (candidate < best) {
      best = candidate;
      best_k = k;
    }
  }
  dp_cur[mid] = best;
  compute_dp_layer(dp_prev, dp_cur, l, mid - 1, opt_l, best_k, cost);
  compute_dp_layer(dp_prev, dp_cur, mid + 1, r, best_k, opt_r, cost);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

struct SquareSegmentCost {
  vector<int64_t> prefix;

  explicit SquareSegmentCost(const vector<int> &a) : prefix(a.size() + 1) {
    for (int i = 0; i < static_cast<int>(a.size()); i++) {
      prefix[i + 1] = prefix[i] + a[i];
    }
  }

  int64_t operator()(int k, int i) const {
    // Cost of placing the half-open segment a[k..i) in one group.
    int64_t sum = prefix[i] - prefix[k];
    return sum * sum;
  }
};

int main() {
  vector<int> a{1, 2, 3, 4};
  int n = static_cast<int>(a.size());
  SquareSegmentCost cost(a);

  // Base layer for zero groups: only the empty prefix is reachable.
  vector<int64_t> dp_prev(n + 1, INF), dp_cur(n + 1, INF);
  dp_prev[0] = 0;

  // The first layer places each prefix a[0..j) into one group.
  compute_dp_layer(dp_prev, dp_cur, 1, n, 0, n - 1, cost);
  assert(dp_cur[4] == 100);  // One group: (1 + 2 + 3 + 4)^2.

  // The second layer chooses the best split between two groups.
  vector<int64_t> dp_two(n + 1, INF);
  compute_dp_layer(dp_cur, dp_two, 1, n, 0, n - 1, cost);
  assert(dp_two[4] == 52);  // (1 + 2 + 3)^2 + 4^2.
  return 0;
}
