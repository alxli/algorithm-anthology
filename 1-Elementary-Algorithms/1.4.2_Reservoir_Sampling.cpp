/*

Selects uniformly random samples from a stream whose length may be unknown in advance. Reservoir
sampling keeps only the chosen sample or samples in memory while processing each element once.

- `reservoir_sample_one(lo, hi)` returns one uniformly random element from `[lo, hi)`. The range
  must be nonempty.
- `reservoir_sample_k(lo, hi, k)` returns `k` uniformly random elements without replacement from
  `[lo, hi)`, or the whole range if it has fewer than `k` elements.

Time Complexity:
- O(n) per call, where $n$ is the number of stream elements.

Space Complexity:
- O(1) auxiliary for `reservoir_sample_one(lo, hi)`.
- O(k) auxiliary heap space for `reservoir_sample_k(lo, hi, k)`.

*/

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iterator>
#include <vector>

template<class It>
typename std::iterator_traits<It>::value_type reservoir_sample_one(It lo, It hi) {
  assert(lo != hi);
  auto sample = *lo;
  int seen = 0;
  for (It it = lo; it != hi; ++it) {
    seen++;
    if (rand() % seen == 0) {
      sample = *it;
    }
  }
  return sample;
}

template<class It>
auto reservoir_sample_k(It lo, It hi, int k)
    -> std::vector<typename std::iterator_traits<It>::value_type> {
  using T = typename std::iterator_traits<It>::value_type;
  std::vector<T> res;
  int seen = 0;
  for (It it = lo; it != hi; ++it) {
    seen++;
    if (static_cast<int>(res.size()) < k) {
      res.push_back(*it);
    } else {
      int j = rand() % seen;
      if (j < k) {
        res[j] = *it;
      }
    }
  }
  return res;
}

/*** Example Usage ***/

#include <cassert>
using namespace std;

int main() {
  srand(1);
  int raw[] = {10, 20, 30, 40, 50};
  vector<int> a(raw, raw + 5);

  int one = reservoir_sample_one(a.begin(), a.end());
  assert(one == 10 || one == 20 || one == 30 || one == 40 || one == 50);

  vector<int> sample = reservoir_sample_k(a.begin(), a.end(), 3);
  assert(sample.size() == 3);
  for (int s : sample) {
    bool found = false;
    for (int x : a) {
      found = found || s == x;
    }
    assert(found);
  }
  return 0;
}
