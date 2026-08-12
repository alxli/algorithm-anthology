/*

Finds frequent elements in a range using cancellation algorithms that retain only a small set of
candidates. Boyer-Moore finds a possible majority element, while Misra-Gries generalizes the same
idea to frequencies above any threshold $n/k$.

The Boyer-Moore voting algorithm maintains one candidate and counter, incrementing on a match and
decrementing on a mismatch. Each decrement cancels two different values. Removing such pairs cannot
eliminate an element occurring more than $\lfloor n/2 \rfloor$ times, so a true majority survives as
the final candidate. A second pass verifies that candidate.

Misra-Gries keeps at most $k - 1$ candidates. A tracked value increments its counter, an untracked
value claims a free counter, and otherwise every counter is decremented. Each full decrement cancels
$k$ distinct occurrences, so any value occurring more than $\lfloor n/k \rfloor$ times must remain a
candidate. These candidates require a second pass for exact verification because the algorithm does
not retain the stream.

- `majority_element(lo, hi)` returns an iterator to the first occurrence of the majority element of
  $[`lo`, `hi`)$, or `hi` if no majority element exists. The range must provide ForwardIterators,
  and its value type must support equality.
- `frequent_candidates(lo, hi, k)` returns a hash table of candidate values to their residual
  counters. `k` must be at least $2$, and the value type must support equality and `std::hash`. Use
  `k = 2` for the unverified Boyer-Moore candidate phase.

Time Complexity:
- O(n) per call to `majority_element(lo, hi)`, where $n$ is the range length.
- O(n) expected per call to `frequent_candidates(lo, hi, k)`: a full-table decrement cancels $k$
  occurrences, so all decrement sweeps take O(n) total.
- O(n*k) per call to `frequent_candidates()` in the collision-heavy worst case.

Space Complexity:
- O(1) auxiliary for `majority_element()`.
- O(k) for the candidates returned by `frequent_candidates()`.

*/

#include <cassert>
#include <iterator>
#include <unordered_map>

template<typename It>
It majority_element(It lo, It hi) {
  int count = 0;
  It candidate = lo;
  for (It it = lo; it != hi; ++it) {
    if (count == 0) {
      candidate = it;
      count = 1;
    } else if (*it == *candidate) {
      count++;
    } else {
      count--;
    }
  }
  // Second pass to verify. If a majority is guaranteed, skip and return candidate directly instead.
  int n = 0;
  count = 0;
  It first = hi;
  for (It it = lo; it != hi; ++it) {
    n++;
    if (*it == *candidate) {
      if (first == hi) {
        first = it;
      }
      count++;
    }
  }
  if (count <= n / 2) {
    return hi;
  }
  return first;
}

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

#include <vector>
using namespace std;

int main() {
  vector<int> a{3, 2, 3, 1, 3};
  assert(majority_element(a.begin(), a.end()) == a.begin());
  vector<int> b{2, 3, 3, 3, 2, 1};
  assert(majority_element(b.begin(), b.end()) == b.end());

  vector<int> c{1, 2, 1, 3, 1, 2, 1, 4, 2, 2, 2};
  auto candidates = frequent_candidates(c.begin(), c.end(), 3);
  assert(candidates.count(1));
  assert(candidates.count(2));
  assert(!candidates.count(3));
  return 0;
}
