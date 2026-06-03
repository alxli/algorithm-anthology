/*

Maintains frequencies over a 1-based integer domain and finds order statistics by binary lifting on
a Fenwick tree. This is useful for dynamic multisets of bounded integer values,
coordinate-compressed kth-element queries, and online rank queries.

- `initialize()` resets all frequencies.
- `add(i, delta)` adds `delta` to the frequency of value/index `i`.
- `sum(i)` returns the total frequency over indices `[1, i]`.
- `kth(k)` returns the smallest index `i` such that `sum(i) >= k`. The argument `k` is 1-based and
  must satisfy `1 <= k <= sum(MAXN)`.

Time Complexity:
- O(n) per call to `initialize()`.
- O(log n) per call to `add(i, delta)`, `sum(i)`, and `kth(k)`.

Space Complexity:
- O(n) for the Fenwick tree.

*/

const int MAXN = 1000;
int tree[MAXN + 1];

void initialize() {
  for (int i = 0; i <= MAXN; i++) {
    tree[i] = 0;
  }
}

void add(int i, int delta) {
  for (; i <= MAXN; i += i & -i) {
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
  int idx = 0;
  int bit = 1;
  while (bit * 2 <= MAXN) {
    bit *= 2;
  }
  for (; bit > 0; bit >>= 1) {
    int next = idx + bit;
    if (next <= MAXN && tree[next] < k) {
      idx = next;
      k -= tree[next];
    }
  }
  return idx + 1;
}

/*** Example Usage ***/

#include <cassert>

int main() {
  initialize();
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
