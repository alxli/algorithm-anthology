/*

Maintain frequencies over a 1-based integer domain and find order statistics by binary lifting on a
Fenwick tree. The tree stores prefix counts; `kth(k)` greedily walks the implicit binary lifting
structure to find the first index whose prefix count reaches `k`.

This is useful for dynamic multisets of bounded integer values, coordinate-compressed kth-element
queries, and online rank queries.

- `initialize(n)` initializes array indices `1` to `n` to 0.
- `add(i, delta)` adds `delta` to the frequency of value/index `i`.
- `sum(i)` returns the total frequency over indices `[1, i]`.
- `kth(k)` returns the smallest index `i` such that `sum(i) >= k`. The argument `k` is 1-based and
  must satisfy `1 <= k <= sum(n)`.

Time Complexity:
- O(n) per call to `initialize()`.
- O(log n) per call to `add(i, delta)`, `sum(i)`, and `kth(k)`.

Space Complexity:
- O(n) for the Fenwick tree.

*/

#include <algorithm>
#include <vector>

std::vector<int> tree;

void initialize(int n) {
  tree.assign(n + 1, 0);
}

void add(int i, int delta) {
  for (; i < tree.size(); i += i & -i) {
    tree[i] += delta;
  }
}

int sum(int i) {
  int res = 0;
  for (; i > 0; i -= i & -i) {
    res += tree[i];
  }
  return res;
}

int kth(int k) {
  int idx = 0, bits = 1;
  while (bits * 2 < tree.size()) {
    bits *= 2;
  }
  for (; bits > 0; bits >>= 1) {
    int next = idx + bits;
    if (next < tree.size() && tree[next] < k) {
      idx = next;
      k -= tree[next];
    }
  }
  return idx + 1;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  initialize(100);
  add(2, 1);
  add(4, 3);
  add(7, 1);
  assert(sum(4) == 4);
  assert(kth(1) == 2);
  assert(kth(2) == 4);
  assert(kth(4) == 4);
  assert(kth(5) == 7);
  return 0;
}
