/*

Given a binary matrix, the exact cover problem asks for a set of rows in which every column contains
exactly one $1$. Many search problems reduce to it: a sudoku grid becomes a matrix whose columns
demand that each cell hold one digit and that each digit appear once per row, column, and box, while
a tiling becomes a matrix whose columns demand that each cell be covered by exactly one piece
placement.

Knuth's Algorithm X solves it by repeatedly choosing an uncovered column, trying each row that
covers it, and deleting from the matrix every column that row satisfies along with every other row
conflicting with it. Dancing links is the representation that makes the deletions cheap. Each $1$ of
the matrix is a node in two circular doubly linked lists, one along its row and one along its
column, and every column has a header node holding the count of its remaining nodes. Removing a node
`x` from a list is `left[right[x]] = left[x]` and `right[left[x]] = right[x]`, which leaves `x`
itself still pointing at its old neighbors; restoring it on backtrack is therefore just
`left[right[x]] = x` and `right[left[x]] = x`, with no allocation and no search. Undoing the
covering operations in the exact reverse of the order that performed them restores the matrix
exactly, which is what makes the whole search run in place on one static set of nodes.

The column to branch on is the one with the fewest remaining rows. This is the minimum-remaining-
values heuristic, and it fails as early as possible: a column with no remaining rows ends the branch
immediately, and a column with one forces the choice.

Secondary columns spare a problem with optional requirements from padding its matrix with one slack
row per optional column. In the N-queens reduction below, every board row and column must hold
exactly one queen and so is primary, while a diagonal may hold none and so is secondary. Asking for
two solutions is the usual test of uniqueness, as a sudoku generator does when validating a puzzle.

- `ExactCover(primary, secondary = 0)` constructs an empty matrix with `primary + secondary`
  columns, both counts nonnegative. Columns in $[0, `primary`)$ must be covered exactly once, while
  the optional columns in $[`primary`, `primary` + `secondary`)$ may be covered at most once.
- `add_row(row)` appends a row containing a $1$ in each column of `row`, which must all be in
  $[0, `primary` + `secondary`)$ and must be distinct. The IDs of the first $r$ rows added are
  $[0, r)$, in the order they were added.
- `solve(limit = 1)` returns up to `limit` solutions, where `limit` is positive. Each solution is
  the sorted list of the IDs of its rows, and an empty result means no exact cover exists. The
  matrix is left unchanged, so it may be queried again.

Solutions come in the order the search finds them, which follows the branching heuristic rather than
row IDs.

Time Complexity:
- O(c) per call to the constructor, where $c$ is the number of columns.
- O(k) expected per call to `add_row(row)`, where $k$ is the size of `row`.
- Exponential per call to `solve()`. Exact cover is NP-complete, and dancing links prunes the search
  rather than avoiding it: the cost is proportional to the number of nodes the search unlinks and
  relinks, which the branching heuristic reduces but does not bound polynomially.

Space Complexity:
- O(n + c) for storage, where $n$ is the number of $1$s in the matrix and $c$ is the number of
  columns.
- O(d) auxiliary stack space per call to `solve()`, where $d$ is the number of rows in the deepest
  partial solution reached, plus O(s*d) for the returned solutions, where $s$ is the number found.

*/

#include <algorithm>
#include <cassert>
#include <climits>
#include <unordered_set>
#include <vector>

class ExactCover {
  int cols, rows;
  std::vector<int> left, right, up, down, col_of, row_of, col_size;
  std::vector<int> partial;

  int add_node(int c, int r) {
    left.push_back(0);
    right.push_back(0);
    up.push_back(0);
    down.push_back(0);
    col_of.push_back(c);
    row_of.push_back(r);
    return static_cast<int>(col_of.size()) - 1;
  }

  // Unlinks column c and every row meeting it, so both vanish from all remaining traversals.
  void cover(int c) {
    right[left[c]] = right[c];
    left[right[c]] = left[c];
    for (int i = down[c]; i != c; i = down[i]) {
      for (int j = right[i]; j != i; j = right[j]) {
        up[down[j]] = up[j];
        down[up[j]] = down[j];
        col_size[col_of[j]]--;
      }
    }
  }

  // Relinks in the mirror order of cover(), which is what restores the matrix exactly.
  void uncover(int c) {
    for (int i = up[c]; i != c; i = up[i]) {
      for (int j = left[i]; j != i; j = left[j]) {
        col_size[col_of[j]]++;
        up[down[j]] = j;
        down[up[j]] = j;
      }
    }
    right[left[c]] = c;
    left[right[c]] = c;
  }

  bool search(int limit, std::vector<std::vector<int>> &found) {
    if (right[0] == 0) {  // Every primary column is covered.
      found.push_back(partial);
      std::sort(found.back().begin(), found.back().end());
      return static_cast<int>(found.size()) >= limit;
    }
    int best = right[0];
    for (int c = right[0]; c != 0; c = right[c]) {
      if (col_size[c] < col_size[best]) {
        best = c;
      }
    }
    if (col_size[best] == 0) {
      return false;  // No row can cover this column, so this branch is dead.
    }
    cover(best);
    for (int r = down[best]; r != best; r = down[r]) {
      partial.push_back(row_of[r]);
      for (int j = right[r]; j != r; j = right[j]) {
        cover(col_of[j]);
      }
      bool done = search(limit, found);
      for (int j = left[r]; j != r; j = left[j]) {
        uncover(col_of[j]);
      }
      partial.pop_back();
      if (done) {
        break;
      }
    }
    uncover(best);
    return static_cast<int>(found.size()) >= limit;
  }

 public:
  explicit ExactCover(int primary, int secondary = 0) : cols(primary), rows(0) {
    assert(primary >= 0 && secondary >= 0 && secondary < INT_MAX);
    assert(primary <= INT_MAX - 1 - secondary);
    cols += secondary;
    col_size.assign(cols + 1, 0);
    for (int c = 0; c <= cols; c++) {  // Node 0 is the root, and node c + 1 heads column c.
      int header = add_node(c, -1);
      up[header] = down[header] = header;
      left[header] = right[header] = header;
    }
    int prev = 0;  // Secondary headers stay outside the root ring, so they are never branched on.
    for (int c = 1; c <= primary; c++) {
      right[prev] = c;
      left[c] = prev;
      prev = c;
    }
    right[prev] = 0;
    left[0] = prev;
  }

  void add_row(const std::vector<int> &row) {
    assert(std::all_of(row.begin(), row.end(), [&](int c) { return c >= 0 && c < cols; }));
    std::unordered_set<int> seen;
    seen.reserve(row.size());
    for (int c : row) {
      bool fresh = seen.insert(c).second;
      assert(fresh);
      (void)fresh;
    }
    int r = rows++, first = -1;
    for (int c : row) {
      int header = c + 1, node = add_node(header, r);
      up[node] = up[header];
      down[node] = header;
      down[up[header]] = node;
      up[header] = node;
      col_size[header]++;
      if (first == -1) {
        first = node;
        left[node] = right[node] = node;
      } else {
        left[node] = left[first];
        right[node] = first;
        right[left[first]] = node;
        left[first] = node;
      }
    }
  }

  std::vector<std::vector<int>> solve(int limit = 1) {
    assert(limit > 0);
    std::vector<std::vector<int>> found;
    partial.clear();
    search(limit, found);
    return found;
  }
};

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Knuth's example matrix over 7 columns, whose unique cover is rows 1, 3, and 5.
  //     col: 0123456
  //   row 0: 1001001
  //   row 1: 1001000
  //   row 2: 0001101
  //   row 3: 0010110
  //   row 4: 0110011
  //   row 5: 0100001
  ExactCover matrix(7);
  matrix.add_row(vector<int>{0, 3, 6});
  matrix.add_row(vector<int>{0, 3});
  matrix.add_row(vector<int>{3, 4, 6});
  matrix.add_row(vector<int>{2, 4, 5});
  matrix.add_row(vector<int>{1, 2, 5, 6});
  matrix.add_row(vector<int>{1, 6});
  assert((matrix.solve() == vector<vector<int>>{{1, 3, 5}}));
  assert(matrix.solve(2).size() == 1);  // Asking for a second solution proves uniqueness.

  ExactCover impossible(2);
  impossible.add_row(vector<int>{0});
  assert(impossible.solve().empty());

  ExactCover optional_only(0, 1);
  optional_only.add_row(vector<int>{0});
  assert((optional_only.solve() == vector<vector<int>>{{}}));

  // The 8-queens problem. Each board row and each board column must hold exactly one queen, while
  // each diagonal may hold at most one, so the 2*n board lines are primary and the 2*(2*n - 1)
  // diagonals are secondary. One matrix row per square places a queen there.
  const int n = 8;
  ExactCover queens(2 * n, 2 * (2 * n - 1));
  for (int r = 0; r < n; r++) {
    for (int c = 0; c < n; c++) {
      queens.add_row(vector<int>{r, n + c, 2 * n + r + c, 2 * n + (2 * n - 1) + r - c + n - 1});
    }
  }
  assert(queens.solve().size() == 1);
  assert(queens.solve(1000).size() == 92);
  return 0;
}
