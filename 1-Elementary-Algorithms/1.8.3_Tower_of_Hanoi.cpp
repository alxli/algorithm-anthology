/*

Move a stack of $n$ disks of distinct sizes from one peg to another using a third as scratch space,
moving one disk at a time and never placing a larger disk on a smaller one. The recursive solution
is forced by the largest disk: it can only move once the other $n - 1$ disks are stacked on the
spare peg, so the puzzle is solved by moving $n - 1$ disks aside, moving the largest disk across,
and moving the $n - 1$ disks back on top of it. That gives $T(n) = 2T(n - 1) + 1$ with $T(0) = 0$,
so exactly $2^n - 1$ moves are needed, and no shorter solution exists because the largest disk
cannot move any earlier.

The sequence also has a closed form, for when one move is wanted rather than all of them: numbering
moves from $1$, move $k$ transfers the disk given by the number of trailing zero bits of $k$, and
that disk always steps in one rotational direction fixed by the parity of $n$. The smallest disk
therefore moves every other turn, which is the rule an iterative solver follows.

- `hanoi_moves(n, from = 0, to = 2)` returns the moves solving the puzzle for `n` disks, as pairs
  (`from_peg`, `to_peg`). Pegs are numbered $[0, 3)$ and the spare peg is whichever of the three is
  neither `from` nor `to`. The number of disks must be nonnegative.
- `hanoi_move_count(n)` returns $2^n - 1$, the number of moves, without generating them. It requires
  $n \leq 63$, the largest count that fits in `int64_t`.

With four or more pegs the problem becomes Frame-Stewart: move a prefix aside using every peg,
transfer the rest with one peg fewer, bring the prefix back, and minimize over the split. That this
family of strategies is optimal was only proved in 2014.

Time Complexity:
- O(2^n) per call to `hanoi_moves()`, which is optimal since that is the size of the output.
- O(1) per call to `hanoi_move_count()`.

Space Complexity:
- O(n) auxiliary stack space and O(2^n) for the returned moves from `hanoi_moves()`.
- O(1) auxiliary for `hanoi_move_count()`.

*/

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

std::vector<std::pair<int, int>> hanoi_moves(int n, int from = 0, int to = 2) {
  assert(n >= 0 && from >= 0 && from < 3 && to >= 0 && to < 3 && from != to);
  std::vector<std::pair<int, int>> moves;
  auto rec = [&](auto &&rec, int disks, int src, int dest) -> void {
    if (disks == 0) {
      return;
    }
    int spare = 3 - src - dest;
    rec(rec, disks - 1, src, spare);
    moves.emplace_back(src, dest);
    rec(rec, disks - 1, spare, dest);
  };
  rec(rec, n, from, to);
  return moves;
}

int64_t hanoi_move_count(int n) {
  assert(n >= 0 && n <= 63);
  return static_cast<int64_t>((uint64_t{1} << n) - 1);  // Unsigned, so n = 63 does not overflow.
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  assert(hanoi_moves(0).empty());
  assert((hanoi_moves(1) == vector<pair<int, int>>{{0, 2}}));
  assert((hanoi_moves(2) == vector<pair<int, int>>{{0, 1}, {0, 2}, {1, 2}}));
  assert(hanoi_move_count(10) == 1023);

  // Replay the moves of a larger puzzle to confirm no disk ever lands on a smaller one.
  const int n = 12;
  vector<vector<int>> pegs(3);
  for (int disk = n; disk >= 1; disk--) {
    pegs[0].push_back(disk);
  }
  auto moves = hanoi_moves(n);
  assert(static_cast<int64_t>(moves.size()) == hanoi_move_count(n));
  for (auto [src, dest] : moves) {
    assert(!pegs[src].empty());
    assert(pegs[dest].empty() || pegs[dest].back() > pegs[src].back());
    pegs[dest].push_back(pegs[src].back());
    pegs[src].pop_back();
  }
  assert(pegs[0].empty() && pegs[1].empty() && static_cast<int>(pegs[2].size()) == n);
  return 0;
}
