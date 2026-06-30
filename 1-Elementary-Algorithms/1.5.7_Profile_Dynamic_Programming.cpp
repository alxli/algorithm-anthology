/*

Counts grid tilings using profile dynamic programming. A profile is a bitmask describing which cells
in the next row (or next position) are already occupied by pieces placed earlier. Processing the
grid left-to-right and top-to-bottom keeps the state small: at cell $(`r`, `c`)$, bit `c` of the
mask says whether this cell is filled before considering new placements.

For domino tilings, a filled cell is simply skipped. Otherwise, the algorithm may place a horizontal
domino into the next column or a vertical domino into the same column of the next row. The mask is
shifted by the row transitions implicitly because bit `c` represents the current column at every
row.

- `count_domino_tilings(rows, cols)` returns the number of ways to tile a `rows` by `cols` rectangle
  with $1 \times 2$ and $2 \times 1$ dominoes.

Time Complexity:
- O(r*c*2^c) per call, where $r$ and $c$ are the number of rows and columns, respectively.

Space Complexity:
- O(2^c) auxiliary.

*/

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <vector>

int64_t count_domino_tilings(int rows, int cols) {
  assert(rows >= 0 && cols >= 0);
  if (rows < cols) {
    return count_domino_tilings(cols, rows);
  }
  assert(cols < 31);
  int states = 1 << cols;
  std::vector<int64_t> dp(states, 0), next(states, 0);
  dp[0] = 1;
  for (int r = 0; r < rows; r++) {
    for (int c = 0; c < cols; c++) {
      std::fill(next.begin(), next.end(), 0);
      for (int mask = 0; mask < states; mask++) {
        if (dp[mask] == 0) {
          continue;
        }
        if (mask & (1 << c)) {
          next[mask ^ (1 << c)] += dp[mask];
        } else {
          if (c + 1 < cols && (mask & (1 << (c + 1))) == 0) {
            next[mask | (1 << (c + 1))] += dp[mask];
          }
          if (r + 1 < rows) {
            next[mask | (1 << c)] += dp[mask];
          }
        }
      }
      dp.swap(next);
    }
  }
  return dp[0];
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(count_domino_tilings(0, 5) == 1);
  assert(count_domino_tilings(2, 3) == 3);
  assert(count_domino_tilings(3, 3) == 0);
  assert(count_domino_tilings(4, 4) == 36);
  return 0;
}
