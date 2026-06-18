/*

Finds candidate frequent elements in a stream using the Misra-Gries algorithm. With parameter $k$,
the algorithm keeps at most $k - 1$ counters and guarantees that every value occurring more than
$\lfloor n/k \rfloor$ times appears among the returned candidates. Each element increments its
counter if already tracked, claims a counter if one of the $k - 1$ slots is free, and otherwise
decrements every counter, discarding those that reach zero. Since a decrement step cancels $k$
distinct occurrences at once, it can happen at most $\lfloor n/k \rfloor$ times, so any value
occurring more often must survive.

The candidates are not automatically verified, since the algorithm intentionally uses sublinear
memory and does not retain the stream. If exact frequencies are needed, make a second pass over the
input and count only the returned candidates.

- `misra_gries(lo, hi, k)` returns a hash table of candidate values to their residual counters. Use
  `k = 2` for the Boyer-Moore majority-candidate special case.

Time Complexity:
- O(n) amortized per call on average: each element either hits an existing counter, taking O(1), or
  triggers a full-table decrement, taking O(k), but the table-full event can occur at most $n/(k-1)$
  times total, so the amortized cost per element is O(1 + k/(k-1)) = O(1). In the worst case, a
  single call is O(n*k), but that cannot be sustained across a long stream.

Space Complexity:
- O(k) auxiliary heap space.

*/

#include <unordered_map>
#include <vector>

template<typename T, typename It>
std::unordered_map<T, int> misra_gries(It lo, It hi, int k) {
  std::unordered_map<T, int> count;
  for (It it = lo; it != hi; ++it) {
    if (auto found = count.find(*it); found != count.end()) {
      found->second++;
    } else if (static_cast<int>(count.size()) < k - 1) {
      count[*it] = 1;
    } else {
      std::vector<T> erase;
      for (auto &[key, val] : count) {
        if (--val == 0) {
          erase.push_back(key);
        }
      }
      for (const auto &key : erase) {
        count.erase(key);
      }
    }
  }
  return count;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  vector<int> a{1, 2, 1, 3, 1, 2, 1, 4, 2, 2, 2};
  unordered_map<int, int> candidates = misra_gries<int>(a.begin(), a.end(), 3);
  assert(candidates.count(1));
  assert(candidates.count(2));
  assert(!candidates.count(3));
  return 0;
}
