/*

Mo's algorithm answers a batch of range queries offline by reordering them so that the current
window `[cur_l, cur_r]` can be transformed into the next query's range with as few single-element
insertions and deletions as possible. It applies whenever there is no fast direct formula, but
extending or shrinking the window by one element updates the answer cheaply, as with the number of
distinct values in a range or the sum of squares of element frequencies.

The trick is the query order. Sort queries by the block of their left endpoint, where each block
spans about $n / \sqrt{q}$ indices, breaking ties by right endpoint. The left pointer then travels
$O(n / \sqrt{q})$ per query, for $O(n\sqrt{q})$ moves in all, while within each block the right
pointer sweeps monotonically across the array, contributing $O(n)$ per block over the $\sqrt{q}$
blocks, again $O(n\sqrt{q})$. So the pointers move $O(n\sqrt{q})$ times in total. Sorting the right
endpoint in alternating directions between consecutive blocks (a "boustrophedon" order) avoids
resetting the right pointer at every block boundary.

- `mos_algorithm(n, queries, add, remove, current)` answers the given inclusive 0-based ranges over
  an array of size `n`. The caller supplies three callbacks that maintain an arbitrary window state:
  `add(i)` and `remove(i)` insert or delete index `i` from the current window, and `current()`
  returns the answer for the window in its present state. The result is a vector of answers in the
  original query order, with element type matching the return type of `current()`.

Time Complexity:
- O(n sqrt(q)) calls to `add()` and `remove()` in total, plus O(q log q) to sort, where $n$ is the
  array size and $q$ is the number of queries.

Space Complexity:
- O(q) auxiliary, excluding whatever window state the callbacks maintain.

*/

#include <algorithm>
#include <cmath>
#include <numeric>
#include <utility>
#include <vector>

template<class AddFn, class RemoveFn, class CurrentFn>
std::vector<decltype(std::declval<CurrentFn>()())> mos_algorithm(
    int n, const std::vector<std::pair<int, int>> &queries, AddFn add, RemoveFn remove,
    CurrentFn current
) {
  int q = queries.size();
  int block = std::max(1, (int)(n / std::max(1.0, std::sqrt((double)q))));
  std::vector<int> order(q);
  std::iota(order.begin(), order.end(), 0);
  std::sort(order.begin(), order.end(), [&](int x, int y) {
    int bx = queries[x].first / block, by = queries[y].first / block;
    if (bx != by) {
      return bx < by;
    }
    return (bx & 1) ? queries[x].second > queries[y].second : queries[x].second < queries[y].second;
  });
  std::vector<decltype(current())> res(q);
  int cur_l = 0, cur_r = -1;  // Window [cur_l, cur_r] is initially empty.
  for (int idx : order) {
    int l = queries[idx].first, r = queries[idx].second;
    while (cur_r < r) add(++cur_r);
    while (cur_l > l) add(--cur_l);
    while (cur_r > r) remove(cur_r--);
    while (cur_l < l) remove(cur_l++);
    res[idx] = current();
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  // Answer "number of distinct values in [l, r]" for several ranges.
  vector<int> a{1, 1, 2, 1, 3, 2, 3};
  vector<int> freq(4, 0);
  int distinct = 0;
  auto add = [&](int i) {
    if (freq[a[i]]++ == 0) distinct++;
  };
  auto remove = [&](int i) {
    if (--freq[a[i]] == 0) distinct--;
  };
  auto current = [&]() { return distinct; };

  vector<pair<int, int>> queries{{0, 6}, {0, 2}, {3, 5}, {1, 1}, {2, 4}};
  vector<int> ans = mos_algorithm(a.size(), queries, add, remove, current);

  assert((ans == vector<int>{3, 2, 3, 1, 3}));
  return 0;
}
