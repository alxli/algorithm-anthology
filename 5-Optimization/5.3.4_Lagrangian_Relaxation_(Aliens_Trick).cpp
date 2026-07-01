/*

Given an optimization problem with a constraint on the number of chosen objects, Lagrangian
relaxation moves that count into the objective by charging a penalty per object. If the relaxed
problem can be solved quickly and the number chosen is monotone as the penalty changes, binary
search recovers the best value for the desired count.

For maximization, solve the relaxed problem as `score - penalty*count`, breaking ties toward larger
`count`. As `penalty` increases, the optimal count never increases. The largest penalty whose
relaxed optimum still chooses at least `target_count` objects gives the exact constrained score
after adding back `penalty*target_count`.

- `lagrangian_maximize(target_count, lo, hi, solve)` returns the maximum original score using
  exactly `target_count` objects. The callable `solve(penalty)` must return `(relaxed_score, count)`
  and must tie-break toward larger `count`.

This technique is often called the Aliens trick. The bounds `lo` and `hi` must contain a valid
integer penalty where the relaxed count crosses `target_count`; widen them when in doubt.

Time Complexity:
- O(log n) calls to `solve()` per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

template<typename Solve>
int64_t lagrangian_maximize(int target_count, int64_t lo, int64_t hi, Solve solve) {
  while (lo < hi) {
    int64_t mid = lo + (hi - lo + 1) / 2;
    if (solve(mid).second >= target_count) {
      lo = mid;
    } else {
      hi = mid - 1;
    }
  }
  auto [relaxed_score, count] = solve(lo);
  assert(count >= target_count);
  return relaxed_score + lo * target_count;
}

/*** Example Usage ***/

int main() {
  std::vector<int64_t> value{10, 7, 3, 2};
  auto choose_profitable = [&](int64_t penalty) {
    int64_t score = 0;
    int count = 0;
    for (int64_t x : value) {
      int64_t gain = x - penalty;
      if (gain >= 0) {
        score += gain;
        count++;
      }
    }
    return std::pair<int64_t, int>{score, count};
  };

  assert(lagrangian_maximize(2, 0, 20, choose_profitable) == 17);
  assert(lagrangian_maximize(3, 0, 20, choose_profitable) == 20);
  return 0;
}
