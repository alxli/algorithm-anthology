/*

Solves normal-play Nim and related impartial games where each position is the disjoint sum of
independent piles. In normal play, the player who makes the last move wins. A Nim position is losing
exactly when the xor of all pile sizes is 0.

This is the base case of the Sprague-Grundy theorem: every finite impartial game position is
equivalent to a Nim pile of size equal to its Grundy number, and sums of games combine by xor.

- `nim_sum(piles)` returns the xor of all pile sizes.
- `first_player_wins(piles)` returns whether the player to move has a winning strategy.
- `winning_move(piles)` returns a move `(pile, new_size)` that moves to xor 0, or `(-1, -1)` if the
  position is losing.

Time Complexity:
- O(n) per call, where $n$ is the number of piles.

Space Complexity:
- O(1) auxiliary.

*/

#include <utility>
#include <vector>

int nim_sum(const std::vector<int> &piles) {
  int x = 0;
  for (int i = 0; i < (int)piles.size(); i++) {
    x ^= piles[i];
  }
  return x;
}

bool first_player_wins(const std::vector<int> &piles) {
  return nim_sum(piles) != 0;
}

std::pair<int, int> winning_move(const std::vector<int> &piles) {
  int x = nim_sum(piles);
  if (x == 0) {
    return std::make_pair(-1, -1);
  }
  for (int i = 0; i < (int)piles.size(); i++) {
    int target = piles[i] ^ x;
    if (target < piles[i]) {
      return std::make_pair(i, target);
    }
  }
  return std::make_pair(-1, -1);
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  int raw[] = {3, 4, 5};
  vector<int> piles(raw, raw + 3);
  assert(nim_sum(piles) == 2);
  assert(first_player_wins(piles));
  pair<int, int> move = winning_move(piles);
  piles[move.first] = move.second;
  assert(nim_sum(piles) == 0);

  int losing_raw[] = {1, 2, 3};
  vector<int> losing(losing_raw, losing_raw + 3);
  assert(!first_player_wins(losing));
  assert(winning_move(losing).first == -1);
  return 0;
}
