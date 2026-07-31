/*

Finds a non-strict local minimum in an array, that is, an index $i$ whose value is no greater than
the values at its existing neighbors. Unlike ternary search, this does not require the array to be
unimodal, but the result is only a local minimum and need not be the global minimum.

Compare the middle element with its right neighbor. If the sequence slopes downward, the right half
contains a local minimum; otherwise, the middle element or the left half contains one. Each
comparison therefore discards half of the remaining indices.

- `local_min_index(a)` returns the index of a non-strict local minimum in the nonempty array `a`.
  Either endpoint may be returned when it is no greater than its sole neighbor.

Time Complexity:
- O(log n) comparisons per call, where $n$ is the size of `a`.

Space Complexity:
- O(1) auxiliary.

*/

#include <cassert>
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

/*** Example Usage ***/

using namespace std;

int main() {
  assert(local_min_index(vector<int>{9, 7, 3, 4, 8}) == 2);
  assert(local_min_index(vector<int>{1, 2, 3, 4}) == 0);

  vector<int> duplicates{4, 2, 2, 3};
  int i = local_min_index(duplicates);
  assert((i == 1 || i == 2) && duplicates[i] == 2);
  return 0;
}
