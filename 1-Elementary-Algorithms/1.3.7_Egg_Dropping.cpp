/*

Given $e$ identical eggs and a building of $f$ floors, find the highest floor from which an egg
survives a drop, using as few drops as possible in the worst case. An egg that survives can be
reused; an egg that breaks is gone. With one egg, the only safe strategy is to climb one floor at a
time, needing $f$ drops. With unlimited eggs, binary search needs $\lceil \log_2(f + 1) \rceil$
drops. The problem is what happens in between.

The natural recurrence minimizes over the drop floor and maximizes over the two outcomes, costing
O(e*f^2). Removing that minimization is a technique worth knowing beyond this puzzle: when the
quantity being minimized is small and bounded, promote it to a state dimension and solve for the old
one. So ask how many floors $d$ drops with $e$ eggs can cover, not how many drops $f$ floors need. A
drop splits the searchable floors into those below it, the drop floor, and those above, giving
$dp(d, e) = dp(d-1, e-1) + dp(d-1, e) + 1$ with nothing left to minimize. Each entry is then O(1),
and the answer is the smallest $d$ whose reach is at least $f$.

That table is $dp(d, e) = \sum_{i=1}^{e} \binom{d}{i}$ in the binomial coefficients of section
6.2.1, which is what keeps $d$ small: enough eggs sum to $2^d - 1$, while one egg leaves only
$\binom{d}{1} = d$. Two eggs reach $91$ floors in 13 drops and $105$ in 14, the classic answer. The
same inversion fits any question of how many tests distinguish $n$ cases, such as finding one
defective item with a limited number of destructive tests.

- `min_drops(eggs, floors)` returns the number of drops needed in the worst case, where `eggs` is
  positive and `floors` is nonnegative.
- `drop_floor(eggs, floors)` returns the floor to drop from first under an optimal strategy, which
  is $dp(d-1, e-1) + 1$ floors above the bottom of the current range, or $0$ when no drop is needed.

Time Complexity:
- O(e*d) per call, where $e$ is `eggs` and $d$ is the returned number of drops, which is at most $f$
  and is O(log f) once $e \geq \log_2(f)$.

Space Complexity:
- O(e) auxiliary.

*/

#include <cassert>
#include <cstdint>
#include <vector>

int min_drops(int eggs, int floors) {
  assert(eggs > 0 && floors >= 0);
  // reach[e] is the number of floors distinguishable by the current number of drops with e eggs.
  std::vector<int64_t> reach(eggs + 1);
  int drops = 0;
  while (reach[eggs] < floors) {
    drops++;
    for (int e = eggs; e > 0; e--) {  // Descending, so reach[e - 1] is still the previous row.
      reach[e] += reach[e - 1] + 1;
    }
  }
  return drops;
}

int drop_floor(int eggs, int floors) {
  assert(eggs > 0 && floors >= 0);
  int drops = min_drops(eggs, floors);
  if (drops == 0) {
    return 0;
  }
  std::vector<int64_t> reach(eggs + 1);
  for (int i = 0; i < drops - 1; i++) {  // Rebuild the table one drop short of the answer.
    for (int e = eggs; e > 0; e--) {
      reach[e] += reach[e - 1] + 1;
    }
  }
  return static_cast<int>(reach[eggs - 1]) + 1;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(min_drops(1, 0) == 0);
  assert(min_drops(1, 10) == 10);  // One egg forces a linear scan.
  assert(min_drops(2, 10) == 4);
  assert(min_drops(2, 100) == 14);  // The classic two-egg, hundred-floor puzzle.
  assert(min_drops(3, 100) == 9);
  assert(min_drops(50, 1000) == 10);  // Enough eggs to binary search.

  // With two eggs and a hundred floors, the first drop is from floor 14.
  assert(drop_floor(2, 100) == 14);
  assert(drop_floor(1, 10) == 1);
  assert(drop_floor(2, 0) == 0);
  return 0;
}
