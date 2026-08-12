/*

The N-queens problem asks for a placement of $n$ queens on an $n$-by-$n$ chessboard such that no two
share a row, column, or diagonal. Backtracking places one queen in each successive row and tracks
which columns and diagonals are occupied. A conflicting placement is rejected immediately; after a
failed recursive branch, its column and diagonals are released before trying the next column.

- `n_queens(n)` returns a placement as a vector `cols` of length `n`, where `cols[row]` is the
  queen's column in that row. It returns an empty vector if no placement exists.

Time Complexity:
- O(n!) per call.

Space Complexity:
- O(n) auxiliary and O(n) for the returned placement.

*/

#include <cassert>
#include <vector>

std::vector<int> n_queens(int n) {
  assert(n > 0);
  std::vector<int> cols(n);
  std::vector<char> used_col(n), used_diag1(2 * n - 1), used_diag2(2 * n - 1);
  auto rec = [&](auto &&rec, int row) -> bool {
    if (row == n) {
      return true;
    }
    for (int col = 0; col < n; col++) {
      int diag1 = row - col + n - 1, diag2 = row + col;
      if (used_col[col] || used_diag1[diag1] || used_diag2[diag2]) {
        continue;
      }
      cols[row] = col;
      used_col[col] = used_diag1[diag1] = used_diag2[diag2] = true;
      if (rec(rec, row + 1)) {
        return true;
      }
      used_col[col] = used_diag1[diag1] = used_diag2[diag2] = false;
      cols[row] = -1;
    }
    return false;
  };
  return rec(rec, 0) ? cols : std::vector<int>{};
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  assert(n_queens(4) == vector<int>({1, 3, 0, 2}));
  assert(n_queens(2).empty());
  assert(n_queens(1) == vector<int>({0}));
  return 0;
}
