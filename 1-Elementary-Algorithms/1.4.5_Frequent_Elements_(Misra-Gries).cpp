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

- `frequent_candidates(lo, hi, k)` returns a hash table of candidate values to their residual
  counters. `k` must be at least $2$, and the value type must support equality and `std::hash`. Use
  `k = 2` for the Boyer-Moore majority-candidate special case.

Time Complexity:
- O(n) expected per call: a full-table decrement cancels $k$ occurrences and therefore happens at
  most $\lfloor n/k \rfloor$ times, so all decrement sweeps take O(n) total. Hash-table operations
  take O(1) expected time each.
- O(n*k) in the collision-heavy worst case for the hash table.

Space Complexity:
- O(1) auxiliary and O(k) for the returned candidates.

*/

#include <cassert>
#include <iterator>
#include <unordered_map>

template<typename It>
auto frequent_candidates(It lo, It hi, int k) {
  using T = typename std::iterator_traits<It>::value_type;
  assert(k >= 2);
  std::unordered_map<T, int> count;
  for (It it = lo; it != hi; ++it) {
    if (auto found = count.find(*it); found != count.end()) {
      found->second++;
    } else if (static_cast<int>(count.size()) < k - 1) {
      count[*it] = 1;
    } else {
      for (auto jt = count.begin(); jt != count.end();) {
        if (--jt->second == 0) {
          jt = count.erase(jt);
        } else {
          ++jt;
        }
      }
    }
  }
  return count;
}

/*** Example Usage ***/

#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<int> a{1, 2, 1, 3, 1, 2, 1, 4, 2, 2, 2};
  unordered_map<int, int> candidates = frequent_candidates(a.begin(), a.end(), 3);
  assert(candidates.count(1));
  assert(candidates.count(2));
  assert(!candidates.count(3));
  return 0;
}
