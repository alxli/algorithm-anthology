/*

Given an optimization problem with a constraint on the number of chosen objects, Lagrangian
relaxation moves that count into the objective by charging a penalty per object. If the relaxed
problem can be solved quickly and the number chosen is monotone as the penalty changes, binary
search recovers the best value for the desired count. This technique is often called the Aliens
trick. Widen the penalty range when in doubt.

For maximization, solve the relaxed problem as `score - penalty*count`, breaking ties toward larger
`count`. As `penalty` increases, the optimal count never increases. The largest penalty whose
relaxed optimum still chooses at least `target_count` objects gives the exact constrained score
after adding back `penalty*target_count`.

- `lagrangian_maximize(target_count, lo, hi, solve)` returns the maximum original score using
  exactly `target_count` objects. The callable `solve(penalty)` must return `relaxed_score` and
  `count` as a pair and must tie-break toward larger `count`. Penalties are searched over
  the half-open range $[`lo`, `hi`)$; `lo` must choose at least `target_count` objects, while `hi`
  must be greater than the last penalty that does so.

Overflow warning: The difference `hi - lo` must fit in `int64_t`.

Time Complexity:
- O(log n) calls to `solve()` per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

template<typename Solve>
int64_t lagrangian_maximize(int target_count, int64_t lo, int64_t hi, Solve solve) {
  // The chosen count is nonincreasing, so find the last penalty that still chooses enough objects.
  while (lo < hi) {
    int64_t mid = lo + (hi - lo) / 2;
    if (solve(mid).second >= target_count) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  int64_t penalty = lo - 1;
  auto [relaxed_score, count] = solve(penalty);
  assert(count >= target_count);
  return relaxed_score + penalty * target_count;  // Overflow warning.
}

/*** Example Usage ***/

using namespace std;

int main() {
  vector<int64_t> value{10, 7, 3, 2};
  auto choose_profitable = [&](int64_t penalty) {
    int64_t score = 0;
    int count = 0;
    for (int64_t x : value) {
      int64_t gain = x - penalty;
      if (gain >= 0) {  // Include zero gains to break ties toward larger counts.
        score += gain;
        count++;
      }
    }
    return pair<int64_t, int>{score, count};
  };

  assert(lagrangian_maximize(2, 0, 20, choose_profitable) == 17);
  assert(lagrangian_maximize(3, 0, 20, choose_profitable) == 20);
  return 0;
}
