/*

Searches a finite two-player, zero-sum, perfect-information game tree using minimax and alpha-beta
pruning. Minimax assumes both players play optimally: the maximizing player chooses the child with
largest value, and the minimizing player chooses the child with smallest value.

Alpha-beta pruning computes the same value as minimax, but avoids exploring branches that cannot
affect the final answer. It is most effective when good moves are searched first.

The example game is "take $1$ or $2$ stones": starting with `stones` stones, players alternate
taking $1$ or $2$ stones, and the player who takes the last stone wins. The evaluation returns $1$
for a win for the player to move at the root and $-1$ for a loss.

- `minimax(stones, maximizing)` returns the exact game-tree value.
- `alpha_beta(stones, maximizing, alpha, beta)` returns the same value, with pruning.
- `best_take(stones)` returns an optimal first move, either 1 or 2.

Time Complexity:
- O(b^d) in the worst case, where $b$ is branching factor and $d$ is search depth. Alpha-beta can
  reduce this substantially with good move ordering.

Space Complexity:
- O(d) auxiliary recursion stack space.

*/

#include <algorithm>

int minimax(int stones, bool maximizing) {
  if (stones == 0) {
    return maximizing ? -1 : 1;
  }
  if (maximizing) {
    int best = -2;
    for (int take = 1; take <= 2 && take <= stones; take++) {
      best = std::max(best, minimax(stones - take, false));
    }
    return best;
  } else {
    int best = 2;
    for (int take = 1; take <= 2 && take <= stones; take++) {
      best = std::min(best, minimax(stones - take, true));
    }
    return best;
  }
}

int alpha_beta(int stones, bool maximizing, int alpha = -2, int beta = 2) {
  if (stones == 0) {
    return maximizing ? -1 : 1;
  }
  if (maximizing) {
    int value = -2;
    for (int take = 1; take <= 2 && take <= stones; take++) {
      value = std::max(value, alpha_beta(stones - take, false, alpha, beta));
      alpha = std::max(alpha, value);
      if (alpha >= beta) {
        break;
      }
    }
    return value;
  } else {
    int value = 2;
    for (int take = 1; take <= 2 && take <= stones; take++) {
      value = std::min(value, alpha_beta(stones - take, true, alpha, beta));
      beta = std::min(beta, value);
      if (alpha >= beta) {
        break;
      }
    }
    return value;
  }
}

int best_take(int stones) {
  int best_move = 1, best_value = -2;
  for (int take = 1; take <= 2 && take <= stones; take++) {
    int value = alpha_beta(stones - take, false);
    if (value > best_value) {
      best_value = value;
      best_move = take;
    }
  }
  return best_move;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  assert(minimax(1, true) == 1);
  assert(minimax(3, true) == -1);
  for (int stones = 1; stones <= 10; stones++) {
    assert(minimax(stones, true) == alpha_beta(stones, true));
  }
  assert(best_take(4) == 1);  // Move to losing state with 3 stones.
  assert(best_take(5) == 2);  // Move to losing state with 3 stones.
  return 0;
}
