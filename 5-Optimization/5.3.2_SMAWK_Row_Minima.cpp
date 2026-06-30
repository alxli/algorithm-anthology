/*

Finds the position of the minimum in every row of a totally monotone matrix in time linear in its
dimensions, without ever materializing the matrix. A matrix is totally monotone (for row minima)
when the column index of each row's leftmost minimum never decreases as the row index increases, and
this holds not just for the whole matrix but for every submatrix. Monge matrices, where
$M_{i,j} + M_{i+1,j+1} \leq M_{i,j+1} + M_{i+1,j}$, are the most common source of such matrices
and arise throughout dynamic programming on intervals and partitions.

The SMAWK algorithm interleaves two steps. REDUCE discards columns that cannot hold the minimum of
any remaining row, leaving at most as many candidate columns as active rows. INTERPOLATE recurses on
the odd-indexed active rows, then recovers each even-indexed active row's minimum by scanning only
the candidate-column range bounded by its already-solved neighbors; total monotonicity guarantees
those scans advance monotonically and cost O(R + C) overall, where $R$ is the number of rows and $C$
is the number of columns in the original matrix. This removes the logarithmic factor of
divide-and-conquer DP optimization, but unlike that technique it is purely offline: every matrix
entry must be available on demand from the oracle, so it cannot be used when an entry depends on a
row minimum computed earlier in the same pass.

- `smawk_row_minima(rows, cols, get)` returns a vector `arg` of length `rows`, where `arg[r]` is the
  column index minimizing `get(r, c)` over $0 \leq `c` < `cols`$ (on ties, the smallest qualifying
  column is chosen). The input matrix is supplied as a totally monotone function `get(r, c)` with
  bounds `rows` $\geq 0$ and `cols` $\geq 1$.

Time Complexity:
- O(r + c) evaluations of `get` per call, where $r$ is `rows` and $c$ is `cols`.

Space Complexity:
- O(r + c) auxiliary.

*/

#include <vector>

template<typename Get>
std::vector<int> smawk_rec(
    const Get &get, const std::vector<int> &active_rows, const std::vector<int> &candidate_cols
) {
  int n = static_cast<int>(active_rows.size());
  std::vector<int> arg(n, -1);
  if (n == 0) {
    return arg;
  }
  // Base case: a single row or column is cheap to scan directly.
  if (n <= 2 || candidate_cols.size() <= 2) {
    for (int i = 0; i < n; i++) {
      for (int c : candidate_cols) {
        if (arg[i] == -1 || get(active_rows[i], c) < get(active_rows[i], arg[i])) {
          arg[i] = c;
        }
      }
    }
    return arg;
  }
  // REDUCE: keep only columns that can be the minimum of some row.
  std::vector<int> kept;
  if (static_cast<int>(candidate_cols.size()) > n) {
    for (int c : candidate_cols) {
      while (!kept.empty()) {
        int r = active_rows[static_cast<int>(kept.size()) - 1];
        if (get(r, kept.back()) <= get(r, c)) {
          break;
        }
        kept.pop_back();
      }
      if (static_cast<int>(kept.size()) < n) {
        kept.push_back(c);
      }
    }
  } else {
    kept = candidate_cols;
  }
  // INTERPOLATE: solve the odd rows recursively, then fill the even rows between known bounds.
  std::vector<int> odd_rows;
  for (int i = 1; i < n; i += 2) {
    odd_rows.push_back(active_rows[i]);
  }
  std::vector<int> odd_arg = smawk_rec(get, odd_rows, kept);
  for (int i = 0; i < static_cast<int>(odd_arg.size()); i++) {
    arg[2 * i + 1] = odd_arg[i];
  }
  int k = 0;
  for (int i = 0; i < n; i += 2) {
    int hi = (i + 1 < n) ? arg[i + 1] : kept.back();
    int r = active_rows[i];
    while (true) {
      int c = kept[k];
      if (arg[i] == -1 || get(r, c) < get(r, arg[i])) {
        arg[i] = c;
      }
      if (c == hi) {
        break;  // Stop at the upper bound; the next even row starts scanning from here.
      }
      k++;
    }
  }
  return arg;
}

template<typename Get>
std::vector<int> smawk_row_minima(int rows, int cols, const Get &get) {
  std::vector<int> row_ids(rows), col_ids(cols);
  for (int i = 0; i < rows; i++) {
    row_ids[i] = i;
  }
  for (int j = 0; j < cols; j++) {
    col_ids[j] = j;
  }
  return smawk_rec(get, row_ids, col_ids);
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  // An explicit totally monotone matrix; row minima sit at non-decreasing columns.
  vector<vector<int>> m{
      {10, 17, 13, 28, 23}, {17, 22, 16, 29, 23}, {24, 28, 22, 34, 24},
      {11, 13, 6, 17, 7},   {45, 44, 32, 37, 23},
  };
  auto get = [&](int r, int c) { return m[r][c]; };
  vector<int> arg = smawk_row_minima(5, 5, get);

  // Cross-check each reported column against a brute-force row scan.
  for (int r = 0; r < 5; r++) {
    int best = 0;
    for (int c = 1; c < 5; c++) {
      if (m[r][c] < m[r][best]) {
        best = c;
      }
    }
    assert(m[r][arg[r]] == m[r][best]);  // Same minimum value (smallest index on ties).
  }
  return 0;
}
