/*

Given a matrix whose every row is sorted left to right and every column sorted top to bottom, find a
target value. Such a matrix is not sorted overall, since a cell says nothing about the cells
diagonally adjacent to it, so a single binary search over the flattened grid does not apply.

Starting at the top-right corner makes each comparison decisive. That cell is the largest in its row
and the smallest in its column, so if it exceeds the target then no cell in its column can match and
the whole column is discarded, while if it is smaller then no cell in its row can match and the row
is discarded. Either way one row or one column disappears per comparison, so the walk visits at most
$R + C$ cells while descending a staircase-shaped boundary. The bottom-left corner works the same
way with the directions swapped; the other two corners are useless, since they are extreme in both
directions at once and a comparison there rules out nothing.

- `search_sorted_matrix(a, key)` returns the position (`row`, `col`) of an occurrence of `key` in
  the row-wise and column-wise sorted matrix `a`, or $(-1, -1)$ if `key` does not occur.
- `count_at_most(a, key)` returns how many entries of `a` are less than or equal to `key`, by
  measuring the same staircase boundary rather than stopping at a match. Combined with the binary
  search of section 5.1.1 over the value range, it yields the 1-based $k$-th smallest entry in
  O((R + C) log(max - min)) time without materializing the sorted order.

A matrix sorted in reading order, every row beginning after the previous ends, is the easier problem
of binary searching one sorted sequence of length $R \cdot C$.

Time Complexity:
- O(R + C) per call, where $R$ and $C$ are the row and column counts.

Space Complexity:
- O(1) auxiliary.

*/

#include <cstdint>
#include <utility>
#include <vector>

template<typename T>
std::pair<int, int> search_sorted_matrix(const std::vector<std::vector<T>> &a, const T &key) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  for (int r = 0, c = cols - 1; r < rows && c >= 0;) {
    if (a[r][c] < key) {
      r++;  // Every cell left in this row is even smaller.
    } else if (key < a[r][c]) {
      c--;  // Every cell below in this column is even larger.
    } else {
      return {r, c};
    }
  }
  return {-1, -1};
}

template<typename T>
int64_t count_at_most(const std::vector<std::vector<T>> &a, const T &key) {
  int rows = static_cast<int>(a.size());
  int cols = a.empty() ? 0 : static_cast<int>(a[0].size());
  int64_t res = 0;
  for (int r = 0, c = cols - 1; r < rows;) {
    if (c >= 0 && key < a[r][c]) {
      c--;
    } else {
      res += c + 1;  // The row's first c + 1 entries are all at most key.
      r++;
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  //   1   4   7  11
  //   2   5   8  12
  //   3   6   9  16
  //  10  13  14  17
  vector<vector<int>> a{{1, 4, 7, 11}, {2, 5, 8, 12}, {3, 6, 9, 16}, {10, 13, 14, 17}};
  assert(search_sorted_matrix(a, 5) == make_pair(1, 1));
  assert(search_sorted_matrix(a, 1) == make_pair(0, 0));   // Top-left corner.
  assert(search_sorted_matrix(a, 17) == make_pair(3, 3));  // Bottom-right corner.
  assert(search_sorted_matrix(a, 11) == make_pair(0, 3));  // The starting cell itself.
  assert(search_sorted_matrix(a, 15) == make_pair(-1, -1));

  assert(count_at_most(a, 0) == 0);
  assert(count_at_most(a, 5) == 5);  // 1, 2, 3, 4, and 5.
  assert(count_at_most(a, 100) == 16);

  vector<vector<int>> single{{42}};
  assert(search_sorted_matrix(single, 42) == make_pair(0, 0));
  assert(search_sorted_matrix(single, 7) == make_pair(-1, -1));
  return 0;
}
