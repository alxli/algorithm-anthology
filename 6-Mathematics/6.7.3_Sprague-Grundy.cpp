/*

Computes Grundy numbers for finite impartial games. The Grundy number of a game position is the
minimum excluded nonnegative integer (MEX) of the Grundy numbers reachable in one move. A position
is losing exactly when its Grundy number is 0.

For a sum of independent impartial games, the combined Grundy number is the XOR of the component
Grundy numbers. This is the Sprague-Grundy theorem.

- `mex(values)` returns the minimum excluded nonnegative integer.
- `subtraction_game_grundy(max_stones, moves)` computes Grundy numbers for a game where a move
  subtracts one value in `moves` from the pile.
- `sum_grundy(grundies)` returns the XOR of component Grundy numbers.

Time Complexity:
- O(n*m) for `subtraction_game_grundy(max_stones, moves)`, where $n$ is `max_stones` and $m$ is the
  number of allowed moves.

Space Complexity:
- O(n + m) auxiliary heap space.

*/

#include <vector>

int mex(const std::vector<int> &values) {
  std::vector<char> seen(values.size() + 1, false);
  for (int v : values) {
    if (0 <= v && v < static_cast<int>(seen.size())) {
      seen[v] = true;
    }
  }
  for (int i = 0; i < static_cast<int>(seen.size()); i++) {
    if (!seen[i]) {
      return i;
    }
  }
  return static_cast<int>(seen.size());
}

std::vector<int> subtraction_game_grundy(int max_stones, const std::vector<int> &moves) {
  std::vector<int> grundy(max_stones + 1, 0);
  for (int stones = 1; stones <= max_stones; stones++) {
    std::vector<int> reachable;
    for (int m : moves) {
      if (m <= stones) {
        reachable.push_back(grundy[stones - m]);
      }
    }
    grundy[stones] = mex(reachable);
  }
  return grundy;
}

int sum_grundy(const std::vector<int> &grundies) {
  int x = 0;
  for (int g : grundies) {
    x ^= g;
  }
  return x;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> moves{1, 3, 4};
  auto g = subtraction_game_grundy(10, moves);
  assert(g[0] == 0);
  assert(g[1] == 1);
  assert(g[2] == 0);
  assert(g[4] == 2);
  assert(g[7] == 0);

  vector<int> heaps{g[5], g[7], g[10]};
  assert(sum_grundy(heaps) == (g[5] ^ g[7] ^ g[10]));
  return 0;
}
