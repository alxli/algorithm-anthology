/*

Given a sorted range of numeric values, find a target by guessing where it should be rather than
always probing the middle. Binary search halves the range because it only asks whether the target is
smaller or larger than the probe; interpolation search additionally uses how much smaller or larger.
Assuming the values rise roughly linearly across the range, the target's position is estimated by
linear interpolation between the two endpoints, the same way a reader opens a dictionary near the
back to look up a word starting with "s".

Each probe reduces the expected distance to the target from $n$ to $\sqrt{n}$ when the values are
drawn from a uniform distribution, which gives O(log log n) probes: for a billion sorted integers
that is about five probes rather than thirty. The bound depends entirely on the distribution.
Clustered values push the estimate to one end repeatedly and degrade the search to O(n) probes, so
this is the rare algorithm whose worst case is worse than the method it replaces. Use it when the
data is known to be near-uniform, such as generated IDs, timestamps at a fixed rate, or hash values,
and use binary search otherwise.

- `interpolation_search(lo, hi, key)` returns an iterator to an occurrence of `key` in the sorted
  random-access range $[`lo`, `hi`)$, or `hi` if `key` does not occur. The value type must support
  comparison and conversion to `long double`.

Time Complexity:
- O(log log n) per call on uniformly distributed values and O(n) in the worst case, where $n$ is the
  range length.

Space Complexity:
- O(1) auxiliary.

*/

template<typename It, typename T>
It interpolation_search(It lo, It hi, const T &key) {
  It last = hi;
  while (lo < hi && key >= *lo && key <= *(hi - 1)) {
    if (*(hi - 1) == *lo) {  // A constant span holds the key only if it equals that value.
      return *lo == key ? lo : last;
    }
    // Estimate the offset by where the key falls between the endpoint values.
    auto len = hi - lo - 1;
    auto loval = static_cast<long double>(*lo), den = static_cast<long double>(*(hi - 1)) - loval;
    // A narrow long double implementation may round distinct large endpoints together.
    It probe = lo + len / 2;
    if (den != 0) {
      probe = lo + static_cast<decltype(len)>((static_cast<long double>(key) - loval) / den * len);
    }
    if (*probe == key) {
      return probe;
    }
    if (*probe < key) {
      lo = probe + 1;
    } else {
      hi = probe;
    }
  }
  return last;
}

/*** Example Usage ***/

#include <cassert>
#include <climits>
#include <cstdint>
#include <vector>
using namespace std;

int main() {
  vector<int> a{2, 4, 6, 8, 10, 12, 14, 16, 18, 20};
  for (int i = 0; i < static_cast<int>(a.size()); i++) {
    assert(interpolation_search(a.begin(), a.end(), a[i]) - a.begin() == i);
  }
  assert(interpolation_search(a.begin(), a.end(), 5) == a.end());
  assert(interpolation_search(a.begin(), a.end(), 1) == a.end());   // Below the range.
  assert(interpolation_search(a.begin(), a.end(), 99) == a.end());  // Above the range.
  assert(interpolation_search(a.begin(), a.begin(), 2) == a.begin());

  // Heavily clustered values still return the right answer, only more slowly.
  vector<int> skewed{1, 1, 1, 1, 1, 1000000};
  assert(*interpolation_search(skewed.begin(), skewed.end(), 1000000) == 1000000);
  assert(*interpolation_search(skewed.begin(), skewed.end(), 1) == 1);
  assert(interpolation_search(skewed.begin(), skewed.end(), 7) == skewed.end());

  vector<int> wide{INT_MIN, 0, INT_MAX};
  assert(*interpolation_search(wide.begin(), wide.end(), 0) == 0);

  vector<int64_t> adjacent{INT64_MAX - 2, INT64_MAX - 1, INT64_MAX};
  assert(*interpolation_search(adjacent.begin(), adjacent.end(), INT64_MAX - 1) == INT64_MAX - 1);
  return 0;
}
