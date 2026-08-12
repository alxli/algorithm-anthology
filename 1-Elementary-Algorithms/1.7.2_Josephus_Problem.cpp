/*

The Josephus problem arranges $n$ people, numbered $[0, n)$, in a circle and repeatedly removes
every $k$-th remaining person until one survives. Counting starts at person $0$, which counts as the
first person. After the first removal, relabeling the smaller circle from the next person gives the
same problem on $n - 1$ people. Mapping its survivor back to the original labels yields the
recurrence $J(1, k) = 0$ and $J(n, k) = (J(n - 1, k) + k) \bmod n$.

When $k$ is small, several removals can be processed at once. One complete pass removes
$\lfloor n/k \rfloor$ people; recursively solving the compressed circle and undoing the resulting
index shift reduces the running time to O(k log n). To recover the full elimination order, a Fenwick
tree stores which labels remain and selects each next victim by rank.

- `josephus(n, k)` returns the 0-based label of the survivor, where `n` and `k` must be positive.
- `josephus_small_k(n, k)` returns the same survivor using the batched recurrence. It is preferable
  when $k$ is small relative to $n$.
- `josephus_order(n, k)` returns all labels in elimination order, ending with the survivor.

Time Complexity:
- O(n) per call to `josephus()`.
- O(min(n, k log n)) per call to `josephus_small_k()`.
- O(n log n) per call to `josephus_order()`.

Space Complexity:
- O(1) auxiliary for `josephus()`.
- O(min(n, k log n)) call stack space for `josephus_small_k()`.
- O(n) auxiliary and O(n) for the returned elimination order from `josephus_order()`.

*/

#include <cassert>
#include <cstdint>
#include <vector>

int josephus(int n, int k) {
  assert(n > 0 && k > 0);
  int survivor = 0;
  for (int size = 2; size <= n; size++) {
    survivor = static_cast<int>((static_cast<int64_t>(survivor) + k) % size);
  }
  return survivor;
}

int josephus_small_k(int n, int k) {
  assert(n > 0 && k > 0);
  if (n == 1) {
    return 0;
  }
  if (k == 1) {
    return n - 1;
  }
  if (k > n) {
    return josephus(n, k);
  }
  int removed = n / k;
  int survivor = josephus_small_k(n - removed, k);
  survivor -= n % k;
  if (survivor < 0) {
    survivor += n;
  } else {
    survivor += survivor / (k - 1);
  }
  return survivor;
}

std::vector<int> josephus_order(int n, int k) {
  assert(n > 0 && k > 0);
  std::vector<int> bit(n + 1), order;
  order.reserve(n);
  for (int i = 1; i <= n; i++) {
    bit[i] = i & -i;  // Initially every label is alive.
  }
  int max_step = 1;
  while (max_step <= n / 2) {
    max_step *= 2;
  }
  auto find_by_rank = [&](int rank) {
    int pos = 0;
    for (int step = max_step; step > 0; step /= 2) {
      int next = pos + step;
      if (next <= n && bit[next] <= rank) {
        pos = next;
        rank -= bit[next];
      }
    }
    return pos;
  };
  int rank = 0;
  for (int remaining = n; remaining > 0; remaining--) {
    rank = static_cast<int>((static_cast<int64_t>(rank) + k - 1) % remaining);
    int removed = find_by_rank(rank);
    order.push_back(removed);
    for (int i = removed + 1; i <= n; i += i & -i) {
      bit[i]--;
    }
    if (remaining > 1) {
      rank %= remaining - 1;
    }
  }
  return order;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  assert(josephus(1, 5) == 0);
  assert(josephus(7, 3) == 3);
  assert(josephus(10, 1) == 9);
  for (int n = 1; n <= 30; n++) {
    for (int k = 1; k <= 30; k++) {
      assert(josephus_small_k(n, k) == josephus(n, k));
      assert(josephus_order(n, k).back() == josephus(n, k));
    }
  }
  assert(josephus_order(7, 3) == vector<int>({2, 5, 1, 6, 4, 0, 3}));
  return 0;
}
