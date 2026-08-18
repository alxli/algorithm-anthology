/*

Finds a non-strict local minimum in an array, that is, an index $i$ whose value is no greater than
the values at its existing neighbors. Unlike ternary search, this does not require the array to be
unimodal, but the result is only a local minimum and need not be the global minimum.

Compare the middle element with its right neighbor. If the sequence slopes downward, the right half
contains a local minimum; otherwise, the middle element or the left half contains one. Each
comparison therefore discards half of the remaining indices.

The same argument works in two dimensions, where a local minimum is a cell no greater than its four
orthogonal neighbors. Scan the middle column for its smallest entry; if that entry is no greater
than its left and right neighbors it is a local minimum, and otherwise the smaller of those two
neighbors lies in a half that must contain one. The half discarded cannot be entered again, because
crossing back would require passing through a cell larger than the one just left, so recursing on
the surviving half halves the columns each time. Scanning a column costs O(R), giving O(R log C).

- `local_min_index(a)` returns the index of a non-strict local minimum in the nonempty array `a`.
  Either endpoint may be returned when it is no greater than its sole neighbor.
- `local_min_cell(a)` returns the position (`row`, `col`) of a non-strict local minimum in the
  nonempty grid `a`, whose rows must all have the same nonzero length.

Time Complexity:
- O(log n) comparisons per call to `local_min_index()`, where $n$ is the size of `a`.
- O(R*log(C)) comparisons per call to `local_min_cell()`, where $R$ and $C$ are the grid dimensions.

Space Complexity:
- O(1) auxiliary for both operations.

*/

#include <cassert>
#include <utility>
#include <vector>

template<typename T>
int local_min_index(const std::vector<T> &a) {
  assert(!a.empty());
  int lo = 0, hi = static_cast<int>(a.size()) - 1;
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    if (a[mid] > a[mid + 1]) {
      lo = mid + 1;
    } else {
      hi = mid;
    }
  }
  return lo;
}

template<typename T>
std::pair<int, int> local_min_cell(const std::vector<std::vector<T>> &a) {
  int rows = static_cast<int>(a.size());
  assert(rows > 0 && !a[0].empty());
  int lo = 0, hi = static_cast<int>(a[0].size()) - 1;
  while (true) {
    int mid = lo + (hi - lo) / 2, best = 0;
    for (int r = 1; r < rows; r++) {  // Smallest entry of the middle column.
      if (a[r][mid] < a[best][mid]) {
        best = r;
      }
    }
    bool descends_left = mid > lo && a[best][mid - 1] < a[best][mid];
    bool descends_right = mid < hi && a[best][mid + 1] < a[best][mid];
    if (!descends_left && !descends_right) {
      return {best, mid};  // No neighbor is smaller, so this cell is a local minimum.
    }
    if (descends_left && (!descends_right || a[best][mid - 1] <= a[best][mid + 1])) {
      hi = mid - 1;
    } else {
      lo = mid + 1;
    }
  }
}

/*** Example Usage ***/

using namespace std;

int main() {
  assert(local_min_index(vector<int>{9, 7, 3, 4, 8}) == 2);
  assert(local_min_index(vector<int>{1, 2, 3, 4}) == 0);

  vector<int> duplicates{4, 2, 2, 3};
  int i = local_min_index(duplicates);
  assert((i == 1 || i == 2) && duplicates[i] == 2);

  vector<vector<int>> grid{
      {9, 8, 7},
      {4, 1, 6},
      {5, 2, 3},
  };
  assert(local_min_cell(grid) == make_pair(1, 1));  // The value 1 beats all four neighbors.
  assert(local_min_cell(vector<vector<int>>{{5}}) == make_pair(0, 0));

  // Every cell of a monotone grid slopes toward the corner, which is the only local minimum.
  vector<vector<int>> slope{
      {1, 2, 3},
      {2, 3, 4},
      {3, 4, 5},
  };
  assert(local_min_cell(slope) == make_pair(0, 0));
  return 0;
}
