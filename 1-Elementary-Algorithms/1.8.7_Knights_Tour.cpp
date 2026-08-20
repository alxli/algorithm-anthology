/*

A knight's tour visits every square of an $n$-by-$n$ board exactly once, moving as a chess knight.
Plain backtracking over the eight moves is hopeless past a small board, since the search tree has
branching factor up to eight and depth $n^2$.

Warnsdorff's rule fixes the move order: from each square, try the onward squares in increasing order
of how many unvisited moves they themselves have. The intuition is that squares with few remaining
exits, such as corners, become unreachable if they are not used early, so the rule visits the most
constrained squares first and leaves the flexible ones for later. This is the same
minimum-remaining-values idea that drives the exact cover search of section 1.8.8.

Warnsdorff's rule alone is a heuristic and does fail on some boards, so the search below keeps
backtracking as a fallback and merely orders the moves by the rule. That ordering is strong enough
that a tour is normally found along the first path tried, with backtracking almost never invoked.

- `knight_tour(n, sr = 0, sc = 0)` returns an `n`-by-`n` grid whose entry is the step at which the
  knight visits that square, numbered $[0, `n`^2)$, starting from square (`sr`, `sc`). It returns an
  empty grid when no tour exists, which is the case for $`n` \in \{2, 3, 4\}$. The board size must
  be positive and the starting square must lie on the board.

A tour is closed when the knight can leap from its final square back to its start, which is possible
only for even $n$ at least $6$. Requiring one means rejecting a completed tour whose last square is
not a knight's move from the first, so the same search finds it by adding that test at the end.

Time Complexity:
- O(n^2) per call in practice, since the move ordering finds a tour without backtracking on the
  boards where one exists. The worst case remains exponential.

Space Complexity:
- O(n^2) auxiliary stack space and O(n^2) for the returned grid.

*/

#include <algorithm>
#include <array>
#include <cassert>
#include <utility>
#include <vector>

const std::array<std::pair<int, int>, 8> KNIGHT_MOVES{
    {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}}
};

std::vector<std::vector<int>> knight_tour(int n, int sr = 0, int sc = 0) {
  assert(n > 0 && sr >= 0 && sr < n && sc >= 0 && sc < n);
  std::vector<std::vector<int>> visit(n, std::vector<int>(n, -1));
  auto onward_moves = [&](int r, int c) {
    int count = 0;
    for (auto [dr, dc] : KNIGHT_MOVES) {
      int r2 = r + dr, c2 = c + dc;
      if (r2 >= 0 && r2 < n && c2 >= 0 && c2 < n && visit[r2][c2] == -1) {
        count++;
      }
    }
    return count;
  };
  auto rec = [&](auto &&rec, int r, int c, int step) -> bool {
    visit[r][c] = step;
    if (step == n * n - 1) {
      return true;
    }
    // Order the onward squares by how few exits they have left, which is Warnsdorff's rule.
    std::vector<std::pair<int, std::pair<int, int>>> candidates;
    for (auto [dr, dc] : KNIGHT_MOVES) {
      int r2 = r + dr, c2 = c + dc;
      if (r2 >= 0 && r2 < n && c2 >= 0 && c2 < n && visit[r2][c2] == -1) {
        candidates.emplace_back(onward_moves(r2, c2), std::make_pair(r2, c2));
      }
    }
    std::sort(candidates.begin(), candidates.end());
    for (auto [degree, square] : candidates) {
      if (rec(rec, square.first, square.second, step + 1)) {
        return true;
      }
    }
    visit[r][c] = -1;
    return false;
  };
  return rec(rec, sr, sc, 0) ? visit : std::vector<std::vector<int>>{};
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(knight_tour(1) == vector<vector<int>>{{0}});
  assert(knight_tour(3).empty());  // No tour exists on boards of size 2, 3, or 4.
  assert(knight_tour(4).empty());

  for (int n : {5, 6, 8}) {
    auto visit = knight_tour(n, n / 2, n / 2);
    assert(!visit.empty());
    vector<pair<int, int>> at(n * n);
    for (int r = 0; r < n; r++) {
      for (int c = 0; c < n; c++) {
        assert(visit[r][c] >= 0 && visit[r][c] < n * n);
        at[visit[r][c]] = {r, c};  // Every step number is used exactly once.
      }
    }
    assert(at[0] == make_pair(n / 2, n / 2));
    for (int step = 1; step < n * n; step++) {
      int dr = abs(at[step].first - at[step - 1].first);
      int dc = abs(at[step].second - at[step - 1].second);
      assert(dr * dc == 2);  // Each consecutive pair differs by a knight's move.
    }
  }
  return 0;
}
