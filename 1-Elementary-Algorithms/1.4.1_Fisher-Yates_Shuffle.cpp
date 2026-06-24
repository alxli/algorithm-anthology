/*

Randomly permutes a range in-place using the Fisher-Yates shuffle. Each possible permutation is
produced with equal probability, assuming the random number source is uniform.

The algorithm scans from right to left. At each position $i$, it chooses a random position $j$ from
$[0, i]$ and swaps the two elements. This fixes one uniformly random remaining element at a time.

- `fisher_yates_shuffle(lo, hi)` randomly shuffles the range $[`lo`, `hi`)$.

Time Complexity:
- O(n) per call, where $n$ is the distance between `lo` and `hi`.

Space Complexity:
- O(1) auxiliary.

*/

#include <algorithm>
#include <random>

template<typename It>
void fisher_yates_shuffle(It lo, It hi) {
  static std::mt19937 rng(std::random_device{}());
  int n = static_cast<int>(hi - lo);
  for (int i = n - 1; i > 0; i--) {
    std::uniform_int_distribution<int> dist(0, i);
    int j = dist(rng);
    std::iter_swap(lo + i, lo + j);
  }
}

/*** Example Usage ***/

#include <algorithm>
#include <cassert>
#include <vector>
using namespace std;

int main() {
  vector<int> a{1, 2, 3, 4, 5}, original(a);
  fisher_yates_shuffle(a.begin(), a.end());
  sort(a.begin(), a.end());
  assert(a == original);
  return 0;
}
